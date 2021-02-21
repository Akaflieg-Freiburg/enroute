/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Vibrator;

import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.os.Bundle;
import android.util.Log;


public class MobileAdaptor extends de.akaflieg_freiburg.enroute.ShareActivity
{
    public static native void onWifiConnected();

    private static MobileAdaptor        m_instance;

    private static NotificationManager  m_notificationManager;
    private static Notification.Builder m_builder;

    private static Vibrator             m_vibrator;

    private static WifiManager          m_wifiManager;
    private static WifiStateChangeReceiver m_wifiStateChangeReceiver;

    /*
      private BroadcastReceiver receiver = new BroadcastReceiver() {
      
      @Override
      public void onReceive(Context context, Intent intent) {
      NetworkInfo info = intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
      Log.i("enroute Wi-Fi network changestate", info.getDetailedState().toString());
      }
      
      };
    */
    
    public MobileAdaptor()
    {
        m_instance = this;
	m_wifiStateChangeReceiver = new WifiStateChangeReceiver();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {

	super.onCreate(savedInstanceState);

	// Look for WiFi changes
	IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
	m_instance.registerReceiver(m_wifiStateChangeReceiver, intentFilter);

    }

    @Override
    public void onDestroy() {
        m_instance.unregisterReceiver(m_wifiStateChangeReceiver);
        super.onDestroy();
    }
    
    /* Vibrate once, very briefly */
    public static void vibrateBrief()
    {
        if (m_vibrator == null) {
            m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
	}
        m_vibrator.vibrate(20);
    }
    
    /* Get the SSID of the current WIFI network, if any.  Returns a string like "<unknown SSID>" otherwise */
    public static String getSSID()
    {
	if (m_wifiManager == null)
	    m_wifiManager = (WifiManager) m_instance.getSystemService(Context.WIFI_SERVICE);
	WifiInfo wifiInfo = m_wifiManager.getConnectionInfo();
	return wifiInfo.getSSID();
    }
    
    /* Show download notification */
    public static void notifyDownload(String text)
    {
        m_notificationManager = (NotificationManager) m_instance.getSystemService(Context.NOTIFICATION_SERVICE);
	
	// Cancel notification
	if ("".equals(text)) {
	    m_notificationManager.cancel(0);
	    return;
	}

	// Build and show notification
	if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
	    NotificationChannel notificationChannel = new NotificationChannel("appStatus", "App Status", NotificationManager.IMPORTANCE_LOW);
	    m_notificationManager.createNotificationChannel(notificationChannel);
	    m_builder = new Notification.Builder(m_instance, notificationChannel.getId());
	} else {
	    m_builder = new Notification.Builder(m_instance);
	}

	Context context = QtNative.activity();
	String packageName = context.getApplicationContext().getPackageName();
        Intent resultIntent = context.getPackageManager().getLaunchIntentForPackage(packageName);
        resultIntent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
        PendingIntent resultPendingIntent = PendingIntent.getActivity(context, 0, resultIntent, PendingIntent.FLAG_UPDATE_CURRENT);
        m_builder.setContentIntent(resultPendingIntent);
	
	m_builder.setSmallIcon(R.drawable.ic_file_download)
	    .setContentTitle(text)
	    .setOngoing(false)
	    .setAutoCancel(false);
	
	m_notificationManager.notify(0, m_builder.build());
    }

    private class WifiStateChangeReceiver extends BroadcastReceiver
    {
        @Override
        public void onReceive(Context context, Intent intent)
        {
	    NetworkInfo info = intent.getParcelableExtra(WifiManager.EXTRA_NETWORK_INFO);
	    if (info.getState() == State.CONNECTED) {
		onWifiConnected();
	    }
	}
    }

}
