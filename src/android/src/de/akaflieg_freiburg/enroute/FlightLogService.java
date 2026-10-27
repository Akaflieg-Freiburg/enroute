/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
 *   stefan.kebekus@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

package de.akaflieg_freiburg.enroute;

import android.Manifest;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ServiceInfo;
import android.media.RingtoneManager;
import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.Uri;
import android.os.Build;
import android.os.IBinder;
import android.os.PowerManager;
import android.util.Log;

/**
 * Foreground service that keeps the app alive while recording a flight.
 *
 * When the user switches to another app, Android normally suspends our process
 * and GPS updates stop. This foreground service with a persistent notification
 * prevents that, allowing automatic flight detection to continue working.
 */
public class FlightLogService extends Service {

    private static final String TAG = "FlightLogService";

    // True while the service runs as a foreground service. Set once
    // startForeground() succeeded, cleared in onDestroy(). This is the
    // authoritative running state; C++ asks for it through start().
    private static volatile boolean s_running = false;

    // The notification permission is requested at most once per process.
    // Every permission request pauses and resumes the activity, and the
    // resume re-checks the service; an unconditional request would loop.
    private static boolean s_notificationPermissionRequested = false;

    // Identifier for the notification channel, used to group notifications
    // in system settings under "Flight Recording".
    private static final String CHANNEL_ID = "flightlog_channel";

    // Separate channel for takeoff/landing event notifications. Uses
    // IMPORTANCE_HIGH so the notification plays a sound and shows as
    // a heads-up banner — even when the app is in the background.
    private static final String EVENT_CHANNEL_ID = "flightlog_event_channel";

    // Unique ID for the persistent notification. Android requires each
    // foreground service to display a notification with a stable ID.
    private static final int NOTIFICATION_ID = 1001;
    private static final int PERMISSION_REQUEST_CODE = 1001;

    // Public notification IDs — referenced from C++ via JNI to post and
    // cancel individual notifications independently.
    public static final int NOTIFICATION_ID_EVENT   = 1002; // takeoff / landing
    public static final int NOTIFICATION_ID_NO_GPS  = 1004; // no position data warning

    // Used only internally (notifyRestart / start).
    private static final int NOTIFICATION_ID_RESTART = 1003;

    // ConnectivityManager for requesting WiFi networks without internet access.
    // Keeps local hotspots like Stratux connected even when they have no internet.
    private ConnectivityManager m_connectivityManager;
    private ConnectivityManager.NetworkCallback m_networkCallback;

    // Partial wake lock: keeps the CPU awake (screen may still turn off) so
    // Qt's main-thread event loop continues to process incoming GPS callbacks
    // from the OS location service even when the screen is locked.
    private PowerManager.WakeLock m_wakeLock;

    @Override
    public void onCreate() {
        super.onCreate();

        // The notification channel must exist before we can post notifications.
        // Creating it multiple times is safe — the system ignores duplicates.
        createNotificationChannel();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // When Android kills the process and START_STICKY causes the service to
        // restart, it delivers a null intent. In that state the Qt application
        // is NOT running — only this Java service is alive. GPS processing and
        // flight detection are inactive. Alert the user so they know to reopen
        // the app and resume recording.
        //
        // Do not promote the service in that state: there is nothing to keep
        // alive, and on Android 14+ promotion would throw if the user revoked
        // location access (revoking kills the process and triggers exactly
        // this restart). Show the notification and stop. stopSelf() is safe
        // here: a sticky restart is not a startForegroundService() call, so
        // Android does not require startForeground().
        if (intent == null) {
            notifyRestart();
            stopSelf();
            return START_NOT_STICKY;
        }

        // Build an intent that brings the user back to the app when they
        // tap the notification. MobileAdaptor is the app's main Activity.
        // FLAG_ACTIVITY_SINGLE_TOP avoids creating a second instance if
        // the activity is already running — it simply brings it to front.
        Intent notificationIntent = new Intent(this, MobileAdaptor.class);
        notificationIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);

        // Wrap the intent in a PendingIntent so Android can launch it later
        // on behalf of our app.
        // - FLAG_UPDATE_CURRENT: reuse an existing PendingIntent, updating extras
        // - FLAG_IMMUTABLE: required on Android 12+, prevents other apps from
        //   modifying the intent (security requirement)
        PendingIntent pendingIntent = PendingIntent.getActivity(
                this, 0, notificationIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        // Build the persistent notification shown in the status bar.
        // setOngoing(true) prevents the user from swiping it away.
        Notification notification = new Notification.Builder(this, CHANNEL_ID)
                .setContentTitle(getString(R.string.notification_title))
                .setContentText(getString(R.string.notification_text))
                .setSmallIcon(R.drawable.ic_notification)
                .setContentIntent(pendingIntent)
                .setOngoing(true)
                .build();

        // Promote this service to a foreground service. The
        // FOREGROUND_SERVICE_TYPE_LOCATION flag tells Android that this service
        // needs location access. With this flag, the app is treated as
        // "in the foreground" and does NOT need ACCESS_BACKGROUND_LOCATION —
        // ACCESS_FINE_LOCATION is sufficient.
        // The three-argument overload with a service type exists only on
        // Android 10 (API 29) and later. Calling it on Android 9 throws
        // NoSuchMethodError and kills the app. Android 9 has no foreground
        // service types, so the two-argument overload is all it needs.
        //
        // start() checks the location permission before starting the service,
        // so promotion should not fail. Catch anyway (for instance, a
        // permission revoked in the instant between start() and this call):
        // an uncaught exception here crashes the app.
        try {
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.Q) {
                startForeground(NOTIFICATION_ID, notification,
                        ServiceInfo.FOREGROUND_SERVICE_TYPE_LOCATION);
            } else {
                startForeground(NOTIFICATION_ID, notification);
            }
        } catch (RuntimeException e) {
            Log.w(TAG, "Cannot promote the flight log service to a foreground service", e);
            stopSelf();
            return START_NOT_STICKY;
        }
        s_running = true;

        // Keep the CPU awake so Qt's event loop processes GPS callbacks
        // even when the screen is locked.
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        if (pm != null) {
            if (m_wakeLock == null) {
                m_wakeLock = pm.newWakeLock(
                        PowerManager.PARTIAL_WAKE_LOCK,
                        "Enroute:FlightLogService");
            }
            if (!m_wakeLock.isHeld()) {
                m_wakeLock.acquire();
            }
        }

        // Request WiFi networks without internet access (e.g. Stratux hotspots).
        // This tells Android to keep these connections alive. We don't bind the
        // process to a specific network — the kernel routing table handles traffic
        // naturally (local Stratux → WiFi, internet traffic → mobile/other networks).
        m_connectivityManager = (ConnectivityManager) getSystemService(
                Context.CONNECTIVITY_SERVICE);
        if (m_connectivityManager != null && m_networkCallback == null) {
            NetworkRequest networkRequest = new NetworkRequest.Builder()
                    .addTransportType(NetworkCapabilities.TRANSPORT_WIFI)
                    .removeCapability(NetworkCapabilities.NET_CAPABILITY_INTERNET)
                    .build();

            // Minimal callback that just monitors; no binding or special handling.
            m_networkCallback = new ConnectivityManager.NetworkCallback();
            m_connectivityManager.requestNetwork(networkRequest, m_networkCallback);
        }

        // START_STICKY tells Android to restart this service if it gets killed
        // due to memory pressure. The service will be restarted with a null
        // intent, so we don't rely on intent extras.
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        // This service is started (not bound), so we return null.
        return null;
    }

    @Override
    public void onDestroy() {
        s_running = false;

        // Release the wake lock so the CPU can sleep again.
        if (m_wakeLock != null && m_wakeLock.isHeld()) {
            m_wakeLock.release();
            m_wakeLock = null;
        }

        // Unregister the network callback.
        if (m_connectivityManager != null && m_networkCallback != null) {
            m_connectivityManager.unregisterNetworkCallback(m_networkCallback);
            m_networkCallback = null;
        }

        // Remove the persistent notification when the service stops.
        // STOP_FOREGROUND_REMOVE ensures the notification is dismissed
        // rather than left behind as a regular notification.
        stopForeground(STOP_FOREGROUND_REMOVE);
        super.onDestroy();
    }

    /**
     * Create the notification channel for flight recording notifications.
     *
     * IMPORTANCE_DEFAULT means sound enabled — required for reliable
     * notification delivery and heads-up display on most devices.
     */
    private void createNotificationChannel() {
        NotificationManager manager = getSystemService(NotificationManager.class);
        if (manager == null) {
            return;
        }

        // IMPORTANCE_DEFAULT shows the icon in the status bar (top bar).
        // IMPORTANCE_LOW would hide it. We suppress the sound so the
        // notification doesn't make noise every time the service starts.
        NotificationChannel channel = new NotificationChannel(
                CHANNEL_ID,
                "Flight Recording",
                NotificationManager.IMPORTANCE_DEFAULT);
        channel.setDescription("Keeps flight recording active in the background");
        channel.setSound(null, null);
        manager.createNotificationChannel(channel);

        // High-priority channel for takeoff/landing events (plays sound,
        // shows heads-up banner). The user can configure or mute this
        // channel in Android system settings independently.
        NotificationChannel eventChannel = new NotificationChannel(
                EVENT_CHANNEL_ID,
                "Flight Events",
                NotificationManager.IMPORTANCE_HIGH);
        eventChannel.setDescription("Takeoff and landing notifications");
        eventChannel.enableVibration(true);
        // Set the default notification sound — required on some devices
        // for heads-up (floating banner) display to trigger.
        Uri defaultSound = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_NOTIFICATION);
        eventChannel.setSound(defaultSound,
                new android.media.AudioAttributes.Builder()
                        .setUsage(android.media.AudioAttributes.USAGE_NOTIFICATION_EVENT)
                        .build());
        manager.createNotificationChannel(eventChannel);
    }

    /**
     * Check whether Android allows this app to run a location foreground service.
     *
     * Since Android 14 (API 34), startForeground() with the location service
     * type throws SecurityException unless the app holds coarse or fine
     * location permission. Older versions do not have this requirement.
     */
    static boolean canStartLocationService(Context context) {
        if (Build.VERSION.SDK_INT < Build.VERSION_CODES.UPSIDE_DOWN_CAKE) {
            return true;
        }
        return context.checkSelfPermission(Manifest.permission.ACCESS_COARSE_LOCATION)
                == PackageManager.PERMISSION_GRANTED
            || context.checkSelfPermission(Manifest.permission.ACCESS_FINE_LOCATION)
                == PackageManager.PERMISSION_GRANTED;
    }

    /**
     * Start the foreground service, if Android allows it.
     *
     * Called from C++ via JNI (QJniObject::callStaticMethod) whenever automatic
     * flight detection is enabled, and again whenever the app window regains
     * focus. Calling it while the service runs is cheap and does nothing.
     *
     * Once startForegroundService() has been called, the service must reach
     * startForeground() or Android crashes the app. The location permission is
     * therefore checked here, before starting, and not only in the service.
     *
     * On Android 13+ (API 33), also requests the POST_NOTIFICATIONS runtime
     * permission, once per process. Without it, the foreground service
     * notification (and takeoff/landing alerts) would be silently suppressed.
     *
     * @return true if the service runs or is starting; false if Android does
     *         not allow it, typically because location permission is missing.
     */
    public static boolean start(Context context) {
        if (s_running) {
            return true;
        }
        if (!canStartLocationService(context)) {
            return false;
        }

        // startForegroundService() throws ForegroundServiceStartNotAllowedException,
        // an IllegalStateException, if the app is in the background (Android 12+).
        Intent intent = new Intent(context, FlightLogService.class);
        try {
            context.startForegroundService(intent);
        } catch (IllegalStateException | SecurityException e) {
            Log.w(TAG, "Cannot start the flight log service", e);
            return false;
        }

        // Qt is now running again — dismiss the "app was killed" warning if
        // it is still visible.
        cancelNotification(context, NOTIFICATION_ID_RESTART);

        // On Android 13+, POST_NOTIFICATIONS is a runtime permission that
        // defaults to denied. Request it via MobileAdaptor which holds the
        // Activity reference, since the context passed from C++ via JNI is
        // only an application context and cannot call requestPermissions().
        if (!s_notificationPermissionRequested) {
            s_notificationPermissionRequested = true;
            MobileAdaptor.requestNotificationPermission();
        }
        return true;
    }

    /**
     * Stop the foreground service.
     *
     * Called from C++ via JNI when the user disables the "Automatic flight
     * detection" setting. This removes the notification and allows Android
     * to suspend the app normally.
     */
    public static void stop(Context context) {
        Intent intent = new Intent(context, FlightLogService.class);
        context.stopService(intent);
    }

    /**
     * Post a notification on the flight events channel.
     *
     * Use the NOTIFICATION_ID_* constants to address individual notifications
     * independently. Called from C++ via JNI.
     *
     * @param context        Application context
     * @param notificationId One of the NOTIFICATION_ID_* constants
     * @param title          Notification title
     * @param message        Notification body text
     */
    public static void postNotification(Context context, int notificationId, String title, String message) {
        NotificationManager manager = context.getSystemService(NotificationManager.class);
        if (manager == null) {
            return;
        }

        // Tapping the notification brings the user back to the app.
        // Use notificationId as the request code so each notification gets
        // its own distinct PendingIntent.
        Intent notificationIntent = new Intent(context, MobileAdaptor.class);
        notificationIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        PendingIntent pendingIntent = PendingIntent.getActivity(
                context, notificationId, notificationIntent,
                PendingIntent.FLAG_UPDATE_CURRENT | PendingIntent.FLAG_IMMUTABLE);

        // CATEGORY_ALARM ensures Android treats this as time-critical and
        // displays it as a heads-up banner with sound.
        Notification notification = new Notification.Builder(context, EVENT_CHANNEL_ID)
                .setContentTitle(title)
                .setContentText(message)
                .setSmallIcon(R.drawable.ic_notification)
                .setContentIntent(pendingIntent)
                .setAutoCancel(true)
                .setCategory(Notification.CATEGORY_ALARM)
                .build();

        manager.notify(notificationId, notification);
    }

    /**
     * Cancel a notification by ID.
     *
     * Use the NOTIFICATION_ID_* constants. Called from C++ via JNI.
     *
     * @param context        Application context
     * @param notificationId One of the NOTIFICATION_ID_* constants
     */
    public static void cancelNotification(Context context, int notificationId) {
        NotificationManager manager = context.getSystemService(NotificationManager.class);
        if (manager != null) {
            manager.cancel(notificationId);
        }
    }

    /**
     * Post a heads-up notification warning that Android killed the app and
     * flight recording is no longer active. Uses RESTART_NOTIFICATION_ID
     * so it can be dismissed independently of real flight-event notifications.
     *
     * This is called from onStartCommand() when intent == null, which is the
     * signal that Android restarted the service via START_STICKY after killing
     * the process. In that state Qt is not running: there is no GPS processing
     * and no flight detection. The user must reopen the app to resume.
     *
     * Uses the EVENT_CHANNEL_ID (IMPORTANCE_HIGH) so the alert is shown as
     * a heads-up banner when the screen is on.
     */
    private void notifyRestart() {
        postNotification(this, NOTIFICATION_ID_RESTART,
                getString(R.string.notification_restart_title),
                getString(R.string.notification_restart_text));
    }
}
