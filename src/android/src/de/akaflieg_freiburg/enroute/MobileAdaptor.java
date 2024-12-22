/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus, stefan.kebekus@gmail.com   *
 *   Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org          *
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

import org.qtproject.qt.android.QtNative;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.content.res.Configuration;
import android.graphics.Color;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.net.wifi.WifiManager.WifiLock;
import android.os.Build;
import android.os.Bundle;
import android.os.Parcelable;
import android.os.Vibrator;
import android.provider.Settings;
import android.provider.Settings.System;
import android.util.Log;
import android.view.*;
import androidx.core.app.ShareCompat;
import androidx.core.content.FileProvider;
import androidx.core.view.WindowCompat;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

public class MobileAdaptor extends de.akaflieg_freiburg.enroute.ShareActivity {

	public static native void onLanguageChanged();

	public static native void onNotificationClicked(int notifyID, int actionID);

	public static native void onWifiConnected();

	public static native void onWindowSizeChanged();

	private static MobileAdaptor m_instance;

	private static Vibrator m_vibrator;

	private static LocaleChangedReceiver m_localeChangedReceiver;
	private static WifiLock m_wifiLock;
	private static WifiManager m_wifiManager;
	private static MulticastLock m_multicastLock;
	private static BroadcastReceiver m_wifiStateChangeReceiver;
	private static BroadcastReceiver m_notifyClickReceiver;

	// reference Authority as defined in AndroidManifest.xml
	private static String AUTHORITY = "de.akaflieg_freiburg.enroute";
	private static String TAG = "IntentLauncher";

	public MobileAdaptor() {
		m_instance = this;
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		// Get WiFi manager and stateChangeReceiver
		m_wifiManager = (WifiManager) m_instance.getSystemService(Context.WIFI_SERVICE);
		m_wifiStateChangeReceiver = new WifiStateChangeReceiver();

		// Acquire Multicast lock
		m_multicastLock = m_wifiManager.createMulticastLock("multicastLock");
		m_multicastLock.setReferenceCounted(true);
		m_multicastLock.acquire();

		// Be informed when notifications are clicked
		m_notifyClickReceiver = new NotifyClickReceiver();
		IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction("de.akaflieg_freiburg.enroute.onNotificationClick");
		m_instance.registerReceiver(m_notifyClickReceiver, intentFilter, RECEIVER_EXPORTED);

		// Be informed when locale changes
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.TIRAMISU) {
			m_localeChangedReceiver = new LocaleChangedReceiver();
			IntentFilter filter = new IntentFilter(Intent.ACTION_LOCALE_CHANGED);
			registerReceiver(m_localeChangedReceiver, filter);
		}

		// Be informed when the window size changes, and call the C++ method
		// onWindowSizeChanged() whenever it changes. The window size changes
		// when the user starts/end the split view mode, or when the user drags
		// the slider in order to adjust the relative size of the two windows
		// shown.
		View rootView = getWindow().getDecorView().getRootView();
		rootView.addOnLayoutChangeListener(new View.OnLayoutChangeListener() {
			@Override
			public void onLayoutChange(View view, int left, int top, int right, int bottom,
					int oldLeft, int oldTop, int oldRight, int oldBottom) {
				onWindowSizeChanged();
			}
		});

		// Set fullscreen
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
			getWindow()
					.getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
		}

	}

	@Override
	public void onDestroy() {
		// Release WiFi lock
		if (m_wifiLock != null) {
			if (m_wifiLock.isHeld() == true) {
				m_wifiLock.release();
			}
		}

		// Release multicast lock
		if (m_multicastLock != null) {
			m_multicastLock.release();
			m_multicastLock = null;
		}

		// Unregister the WiFi state change receiver
		if (m_wifiStateChangeReceiver != null) {

			try {
				m_instance.unregisterReceiver(m_wifiStateChangeReceiver);
				m_wifiStateChangeReceiver = null;
			} catch (IllegalArgumentException e) {
				Log.d("enroute flight navigation", "Exception in onDestroy: " + e.toString());
			}

		} else {
			Log.d("enroute flight navigation", "onDestroy: m_wifiStateChangeReceiver == null");
		}

		super.onDestroy();
	}

	//
	// Static Methods
	//

	// Returns the device name
	public static String deviceName() {
		return android.os.Build.MANUFACTURER + " " + android.os.Build.PRODUCT + " (" + android.os.Build.MODEL + ")";
	}

	// Returns the height of the screen, taking the Android split view
	// into account
	public static double windowHeight() {
		return m_instance.getWindow().getDecorView().getRootView().getHeight();
	}

	// Returns the width of the screen, taking the Android split view
	// into account
	public static double windowWidth() {
		return m_instance.getWindow().getDecorView().getRootView().getWidth();
	}

	// Returns the bottom inset required to avoid system bars and display cutouts
	public static double safeInsetBottom() {
		if (Build.VERSION.SDK_INT >= 30) {
			return m_instance.getWindow().getDecorView().getRootWindowInsets()
					.getInsets(WindowInsets.Type.systemBars() | WindowInsets.Type.ime()
							| WindowInsets.Type.displayCutout()).bottom;
		}

		return m_instance.getWindow().getDecorView().getRootWindowInsets().getSystemWindowInsetBottom();
	}

	// Returns the left inset required to avoid system bars and display cutouts
	public static double safeInsetLeft() {
		if (Build.VERSION.SDK_INT >= 30) {
			return m_instance.getWindow().getDecorView().getRootWindowInsets()
					.getInsets(WindowInsets.Type.systemBars() | WindowInsets.Type.ime()
							| WindowInsets.Type.displayCutout()).left;
		}

		return m_instance.getWindow().getDecorView().getRootWindowInsets().getSystemWindowInsetLeft();
	}

	// Returns the right inset required to avoid system bars and display cutouts
	public static double safeInsetRight() {
		if (Build.VERSION.SDK_INT >= 30) {
			return m_instance.getWindow().getDecorView().getRootWindowInsets()
					.getInsets(WindowInsets.Type.systemBars() | WindowInsets.Type.ime()
							| WindowInsets.Type.displayCutout()).right;
		}

		return m_instance.getWindow().getDecorView().getRootWindowInsets().getSystemWindowInsetRight();
	}

	// Returns the top inset required to avoid system bars and display cutouts
	public static double safeInsetTop() {
		if (Build.VERSION.SDK_INT >= 30) {
			return m_instance.getWindow().getDecorView().getRootWindowInsets()
					.getInsets(WindowInsets.Type.systemBars() | WindowInsets.Type.ime()
							| WindowInsets.Type.displayCutout()).top;
		}

		return m_instance.getWindow().getDecorView().getRootWindowInsets().getSystemWindowInsetTop();
	}

	/*
	 * Get the SSID of the current WIFI network, if any. Returns a string like
	 * "<unknown SSID>" otherwise
	 */
	public static String getSSID() {

		if (m_wifiManager == null) {
			m_wifiManager = (WifiManager) m_instance.getSystemService(Context.WIFI_SERVICE);
		}
		WifiInfo wifiInfo = m_wifiManager.getConnectionInfo();
		return wifiInfo.getSSID();

	}

	/*
	 * Get the manufacturer of the current device.
	 */
	public static String manufacturer() {

		return android.os.Build.MANUFACTURER;

	}

	/*
	 * Acquire or release a WiFi lock
	 */
	public static void lockWiFi(boolean on) {

		// Get WiFi lock w/o reference counting
		if (m_wifiLock == null) {
			m_wifiLock = m_wifiManager.createWifiLock(WifiManager.WIFI_MODE_FULL, "Traffic Receiver Wi-Fi Lock");
			m_wifiLock.setReferenceCounted(false);
		}

		// Paranoid safety checks
		if (m_wifiLock == null) {
			return;
		}

		// Acquire lock
		if (on == true) {

			try {
				m_wifiLock.acquire();
			} catch (Exception e) {
				Log.d("enroute flight navigation", "Exception in lockWiFi(): " + e.toString());
			}

			return;
		}

		// Release lock
		if (m_wifiLock.isHeld() == true) {
			m_wifiLock.release();
		}

	}

	/* Begin to monitor network changes */
	public static void startWiFiMonitor() {

		// Look for WiFi changes
		IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
		m_instance.registerReceiver(m_wifiStateChangeReceiver, intentFilter);

	}

	/*
	 * If the system setting for haptic feedback is "on", then vibrate once briefly
	 */
	public static void vibrateBrief() {

		// Get system settings for haptic feedback
		int haptic = Settings.System.getInt(m_instance.getContentResolver(),
				Settings.System.HAPTIC_FEEDBACK_ENABLED, 1);

		// If systems settings want vibrate, then do vibrate
		if (haptic != 0) {
			if (m_vibrator == null) {
				m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
			}
			m_vibrator.vibrate(20);
		}

	}

	/*
	 * If the system setting for haptic feedback is "on", then vibrate once for a
	 * longer time
	 */
	public static void vibrateLong() {

		// Get system settings for haptic feedback
		int haptic = Settings.System.getInt(m_instance.getContentResolver(),
				Settings.System.HAPTIC_FEEDBACK_ENABLED, 1);

		// If systems settings want vibrate, then do vibrate
		if (haptic != 0) {
			if (m_vibrator == null) {
				m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
			}
			m_vibrator.vibrate(500);
		}

	}

	/**
	 * SEND (== "share") a file to another application.
	 *
	 * The receiving app is _not_ supposed to handle the mime type but
	 * it is rather supposed to _transport_ the file.
	 * Examples for receiving apps are email, messenger or file managers.
	 *
	 * @param filePath the path of the file to send.
	 * @param mimeType the mime type of the file to send.
	 *
	 * @return true if the intent was launched otherwise false
	 */
	public static boolean sendFile(String filePath, String mimeType) {
		return sendOrViewFile(filePath, mimeType, Intent.ACTION_SEND);
	}

	/**
	 * VIEW (== "open") a file in another application.
	 *
	 * The receiving app is supposed to handle the mime type.
	 *
	 * @param filePath the path of the file to send.
	 * @param mimeType the mime type of the file to send.
	 *
	 * @return true if the intent was launched otherwise false
	 */
	public static boolean viewFile(String filePath, String mimeType) {
		return sendOrViewFile(filePath, mimeType, Intent.ACTION_VIEW);
	}

	//
	// Private Methods
	//
	/**
	 * common implementation for both the sendFile() and viewFile() methods.
	 *
	 * Creates an appropriate intent and does the necessary settings.
	 * Creates a chooser with the intent and calls startActivity() to fire the
	 * intent.
	 *
	 * This method does _not_ block until the content has been share with another
	 * app but rather returns immediately (if action == 0). Therefore the receiving
	 * apps can handle the file URL while enroute stays active an is not blocked.
	 * Effectively this means that both the sending and the receiving app will run
	 * concurrently.
	 *
	 * @param filePath the path of the file to send.
	 * @param mimeType the mime type of the file to send.
	 * @param action   either Intent.ACTION_SEND or Intent.ACTION_VIEW
	 *
	 * @return true if the intent was launched otherwise false
	 */
	private static boolean sendOrViewFile(String filePath, String mimeType, String action) {

		if (m_instance == null) {
			return false;
		}

		// Intent intent = new Intent();
		// using v4 support library create the Intent from ShareCompat
		Intent intent = ShareCompat.IntentBuilder.from(m_instance).getIntent();
		intent.setAction(action);

		Uri uri = fileToUri(filePath);

		if (uri == null) {
			return false;
		}

		if (mimeType == null || mimeType.isEmpty()) {
			mimeType = m_instance.getContentResolver().getType(uri);
		}

		if (action == Intent.ACTION_SEND) {
			intent.putExtra(Intent.EXTRA_STREAM, uri);
			intent.setType(mimeType);
		} else {
			intent.setDataAndType(uri, mimeType);
		}

		intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);

		return customStartActivity(intent);
	}

	private static boolean openInGoogleEarth(String geoUrl) {
		if (m_instance == null) {
			return false;
		}
		Uri gmmIntentUri = Uri.parse(geoUrl);
		Intent mapIntent = new Intent(Intent.ACTION_VIEW, gmmIntentUri);
		mapIntent.setPackage("com.google.earth");

		try {
    		m_instance.startActivity(mapIntent);
		} catch (Exception e) {
		    // Google Earth is not installed
			return false;
		}
		return true;
	}

	/**
	 * create uri from file path.
	 *
	 * android content provoiders operate with Uri's which represent the
	 * content scheme (e.g. file:// or content://). This methods converts
	 * the absolute file path in the sharing directory into a valid Uri.
	 *
	 * @param filePath the absolut path of the file in the cache directory.
	 *
	 * @return Uri the corresponding uri
	 */

	private static Uri fileToUri(String filePath) {

		File fileToShare = new File(filePath);

		// Using FileProvider you must get the URI from FileProvider using your
		// AUTHORITY
		// Uri uri = Uri.fromFile(imageFileToShare);
		Uri uri;
		try {
			return FileProvider.getUriForFile(m_instance, AUTHORITY, fileToShare);
		} catch (IllegalArgumentException e) {
			Log.d(TAG, "error" + e.getMessage());
			return null;
		}
	}

	/**
	 * create a custom chooser and start activity.
	 *
	 * The custom chooser takes care of not sharing with or sending to the own app.
	 * This is done by first finding matching apps which can handle the intent
	 * and then blacklisting the own app by Intent.EXTRA_EXCLUDE_COMPONENTS.
	 *
	 * The chooser intent is then started with startActivity() which will return
	 * immediately and not wait for the chooser result. This way, both enroute and
	 * the receiving app can run concurrently and enroute is _not_ blocked until
	 * the receiving app terminates.
	 *
	 * @param theIntent intent to be handled
	 *
	 * @return true if the intent was launched otherwise false
	 */
	private static boolean customStartActivity(Intent theIntent) {

		final Context context = m_instance;
		final PackageManager packageManager = context.getPackageManager();

		// MATCH_DEFAULT_ONLY: Resolution and querying flag. if set, only filters
		// that support the CATEGORY_DEFAULT will be considered for matching. Check
		// if there is a default app for this type of content.
		ResolveInfo defaultAppInfo = packageManager.resolveActivity(theIntent, PackageManager.MATCH_DEFAULT_ONLY);
		if (defaultAppInfo == null) {
			Log.d(TAG, "PackageManager cannot resolve Activity");
			return false;
		}

		// Retrieve all apps for our intent. Check if there are any apps returned
		List<ResolveInfo> appInfoList = packageManager.queryIntentActivities(theIntent,
				PackageManager.MATCH_DEFAULT_ONLY);
		if (appInfoList.isEmpty()) {
			Log.d(TAG, "appInfoList.isEmpty");
			return false;
		}
		// Log.d(TAG, "appInfoList: " + appInfoList.size());

		// Sort in alphabetical order
		Collections.sort(appInfoList, new Comparator<ResolveInfo>() {
			@Override
			public int compare(ResolveInfo first, ResolveInfo second) {
				String firstName = first.loadLabel(packageManager).toString();
				String secondName = second.loadLabel(packageManager).toString();
				return firstName.compareToIgnoreCase(secondName);
			}
		});

		// find own package and blacklist it
		//
		List<ComponentName> blacklistedComponents = new ArrayList<ComponentName>();
		for (ResolveInfo info : appInfoList) {

			String pkgName = info.activityInfo.packageName;
			String clsName = info.activityInfo.name;
			// we don't want to share with our own app so blacklist it
			//
			if (pkgName.equals(context.getPackageName())) {
				blacklistedComponents.add(new ComponentName(pkgName, clsName));
			}
		}

		Intent chooserIntent = Intent.createChooser(theIntent, null /* title */);
		chooserIntent.putExtra(Intent.EXTRA_EXCLUDE_COMPONENTS, blacklistedComponents.toArray(new Parcelable[] {}));
		chooserIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);

		// Verify that the intent will resolve to an activity
		// do NOT use startActivityForResult as it will block
		// enroute until the receiving app terminates.
		if (chooserIntent.resolveActivity(m_instance.getPackageManager()) != null) {
			m_instance.startActivity(chooserIntent);
			return true;
		}
		return false;
	}

	//
	// Embedded classes
	//

	private class LocaleChangedReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			onLanguageChanged();
		}
	}

	private class WifiStateChangeReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			NetworkInfo info = intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
			if (info.getState() == State.CONNECTED) {
				onWifiConnected();
			}
		}
	}

	private class NotifyClickReceiver extends BroadcastReceiver {
		@Override
		public void onReceive(Context context, Intent intent) {
			Intent i = new Intent(context, MobileAdaptor.class);
			i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
			context.startActivity(i);

			onNotificationClicked(intent.getIntExtra("NotificationID", -1), intent.getIntExtra("ActionID", 0));
		}
	}

}
