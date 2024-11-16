/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

import org.qtproject.qt.android.QtNative;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Configuration;
import android.graphics.Color;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.MulticastLock;
import android.net.wifi.WifiManager.WifiLock;
import android.os.Build;
import android.os.Bundle;
import android.os.Vibrator;
import android.provider.Settings;
import android.provider.Settings.System;
import android.util.Log;
import android.view.*;
import androidx.core.view.WindowCompat;

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
		if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) 
		    {
			getWindow().getAttributes().layoutInDisplayCutoutMode = WindowManager.LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
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

	/* If the system setting for haptic feedback is "on", then vibrate once briefly */
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

	/* If the system setting for haptic feedback is "on", then vibrate once for a longer time */
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

	//
	// Embedded classes
	//
	
	private
	 class LocaleChangedReceiver extends BroadcastReceiver {
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
