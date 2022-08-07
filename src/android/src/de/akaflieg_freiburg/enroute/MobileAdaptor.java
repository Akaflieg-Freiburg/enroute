/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

import org.qtproject.qt5.android.QtNative;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
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

public class MobileAdaptor extends de.akaflieg_freiburg.enroute.ShareActivity {
	public static native void onNotificationClicked(int notifyID, int actionID);

	public static native void onWifiConnected();

	private static MobileAdaptor m_instance;

	private static Vibrator m_vibrator;

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

		m_notifyClickReceiver = new NotifyClickReceiver();
		IntentFilter intentFilter = new IntentFilter();
		intentFilter.addAction("de.akaflieg_freiburg.enroute.onNotificationClick");
		m_instance.registerReceiver(m_notifyClickReceiver, intentFilter);
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
	 * Get the SSID of the current WIFI network, if any. Returns a string like
	 * "<unknown SSID>" otherwise
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

	/* If systems setting for haptic feedback is "on", then vibrate once briefly */
	public static void vibrateBrief() {

		// Get system settings for haptic feedback
		int haptic = Settings.System.getInt(QtNative.activity().getContentResolver(),
				Settings.System.HAPTIC_FEEDBACK_ENABLED, 1);

		// If systems settings want vibrate, then do vibrate
		if (haptic != 0) {
			if (m_vibrator == null) {
				m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
			}
			m_vibrator.vibrate(20);
		}

	}

	//
	// Embedded classes
	//

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
