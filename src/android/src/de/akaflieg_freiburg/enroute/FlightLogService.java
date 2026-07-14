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

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ServiceInfo;
import android.media.RingtoneManager;
import android.net.ConnectivityManager;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.Uri;
import android.os.Build;
import android.os.IBinder;

/**
 * Foreground service that keeps the app alive while recording a flight.
 *
 * When the user switches to another app, Android normally suspends our process
 * and GPS updates stop. This foreground service with a persistent notification
 * prevents that, allowing automatic flight detection to continue working.
 */
public class FlightLogService extends Service {

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
        if (intent == null) {
            notifyRestart();
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
        startForeground(NOTIFICATION_ID, notification,
                ServiceInfo.FOREGROUND_SERVICE_TYPE_LOCATION);

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
     * Start the foreground service.
     *
     * Called from C++ via JNI (QJniObject::callStaticMethod) when the user
     * enables the "Automatic flight detection" setting.
     * Uses startForegroundService() which is required on Android 8+ (API 26)
     * for services that will call startForeground().
     *
     * On Android 13+ (API 33), requests the POST_NOTIFICATIONS runtime
     * permission first. Without it, the foreground service notification
     * (and takeoff/landing alerts) would be silently suppressed.
     */
    public static void start(Context context) {
        // On Android 13+, POST_NOTIFICATIONS is a runtime permission that
        // defaults to denied. Request it via MobileAdaptor which holds the
        // Activity reference, since the context passed from C++ via JNI is
        // only an application context and cannot call requestPermissions().
        MobileAdaptor.requestNotificationPermission();

        // Qt is now running again — dismiss the "app was killed" warning if
        // it is still visible.
        cancelNotification(context, NOTIFICATION_ID_RESTART);

        Intent intent = new Intent(context, FlightLogService.class);
        context.startForegroundService(intent);
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
