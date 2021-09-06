  /***************************************************************************
  *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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
import android.graphics.BitmapFactory;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.net.wifi.WifiManager.WifiLock;
import android.os.Vibrator;

import android.content.BroadcastReceiver;
import android.content.IntentFilter;
import android.graphics.Color;
import android.net.NetworkInfo;
import android.net.NetworkInfo.State;
import android.os.Bundle;
import android.util.Log;


public class MobileAdaptor extends de.akaflieg_freiburg.enroute.ShareActivity
{
    public static native void onNotificationClicked(int notifyID);
    public static native void onWifiConnected();
    
    private static MobileAdaptor           m_instance;
    
    private static NotificationManager     m_notificationManager;
    private static Notification.Builder    m_builder;
    
    private static Vibrator                m_vibrator;
    
    private static WifiLock                m_wifiLock;
    private static WifiManager             m_wifiManager;
    private static BroadcastReceiver       m_wifiStateChangeReceiver;
    private static BroadcastReceiver       m_notifyClickReceiver;
    
    
    public MobileAdaptor()
    {
	m_instance = this;
    }
    
    @Override
    public void onCreate(Bundle savedInstanceState) {  
	super.onCreate(savedInstanceState);
	
	// Get WiFi manager and stateChangeReceiver
	m_wifiManager = (WifiManager) m_instance.getSystemService(Context.WIFI_SERVICE);
	m_wifiStateChangeReceiver = new WifiStateChangeReceiver();
	
	m_notifyClickReceiver = new NotifyClickReceiver();
	IntentFilter intentFilter = new IntentFilter();
	intentFilter.addAction("MyAction");
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
	
	// Unregister the WiFi state change receiver
	if (m_wifiStateChangeReceiver != null) {
	    
	    try  {
		m_instance.unregisterReceiver(m_wifiStateChangeReceiver);
		m_wifiStateChangeReceiver = null;
	    } catch (IllegalArgumentException e) {
		Log.d("enroute flight navigation", "onDestroy: IllegalArgumentException on m_instance.unregisterReceiver(m_wifiStateChangeReceiver)");
	    }
	    
	} else {
	    Log.d("enroute flight navigation", "onDestroy: m_wifiStateChangeReceiver == null");
	}
	
	super.onDestroy();
    }
    
    
    //
    // Static Methods
    //
    
    /* Get the SSID of the current WIFI network, if any.  Returns a string like "<unknown SSID>" otherwise */
    public static String getSSID()
    {
	if (m_wifiManager == null) {
	    m_wifiManager = (WifiManager) m_instance.getSystemService(Context.WIFI_SERVICE);
	}
	WifiInfo wifiInfo = m_wifiManager.getConnectionInfo();
	return wifiInfo.getSSID();
    }
    
    
    /* Acquire or release a WiFi lock */
    public static void lockWiFi(boolean on)
    {
	Log.i("enroute Flight Navigation", "lockWiFi");
	
	// Get WiFi lock w/o reference counting
	if (m_wifiLock == null) {
	    Log.d("Enroute Flight Navigation", "lockWiFi - get locker");
	    m_wifiLock = m_wifiManager.createWifiLock(WifiManager.WIFI_MODE_FULL , "Traffic Receiver Wi-Fi Lock");
	    m_wifiLock.setReferenceCounted(false);
	}
	
	// Paranoid safety checks
	if (m_wifiLock == null) {
	    return;
	}
	
	// Acquire lock
	if (on == true) {
	    m_wifiLock.acquire();
	    return;
	}
	
	// Release lock
	if (m_wifiLock.isHeld()==true) {
	    m_wifiLock.release();
	}
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
        Intent notificationIntent = new Intent("MyAction");
        notificationIntent.putExtra("NotificationID", (int)0);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(context, 0, notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT) ;
        m_builder.setContentIntent(pendingIntent);
	
	m_builder.setSmallIcon(R.drawable.ic_file_download)
	    .setColor(Color.rgb(00,0x80,0x80))
	    .setContentTitle(text)
	    .setOngoing(false)
	    .setAutoCancel(false);
	
	m_notificationManager.notify(0, m_builder.build());
    }
    
    
    /* Show traffic receiver error notification */
    public static void hideNotification(int id)
    {
	m_notificationManager = (NotificationManager) m_instance.getSystemService(Context.NOTIFICATION_SERVICE);
        m_notificationManager.cancel(id);
    }

    
    /* Show traffic receiver error notification */
    public static void showNotification(int id, String title, String subject, String text, String longText)
    {
	// Get notification manager
	m_notificationManager = (NotificationManager) m_instance.getSystemService(Context.NOTIFICATION_SERVICE);
	
	// Build and show notification
	if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
	    
	    NotificationChannel notificationChannel = null;
	    switch(id) {
            case 0:
		notificationChannel = new NotificationChannel("info", "Information", NotificationManager.IMPORTANCE_LOW);
		break;
            case 1:
            case 2:
		notificationChannel = new NotificationChannel("error", "Error Message", NotificationManager.IMPORTANCE_HIGH);
		break;
            }
	    m_notificationManager.createNotificationChannel(notificationChannel);
	    m_builder = new Notification.Builder(m_instance, notificationChannel.getId());
	} else {
	    m_builder = new Notification.Builder(m_instance);
	}
	
	Intent notificationIntent = new Intent("MyAction");
	notificationIntent.putExtra("NotificationID", id);
	PendingIntent pendingIntent = PendingIntent.getBroadcast(QtNative.activity(), 0, notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT) ;
	
	m_builder.setContentIntent(pendingIntent);
	m_builder.setColor(Color.rgb(00,0x80,0x80));
	m_builder.setContentTitle(title);
	m_builder.setContentText(subject);
	m_builder.setOngoing(false);
	m_builder.setAutoCancel(false);
	m_builder.setStyle(new Notification.BigTextStyle().bigText(text));
	
	switch(id) {
        case 0:
	    m_builder.setSmallIcon(R.drawable.ic_info);
	    m_builder.setLargeIcon(BitmapFactory.decodeResource(QtNative.activity().getResources(), R.drawable.ic_file_download));
	    break;
        case 1:
        case 2:
	    m_builder.setSmallIcon(R.drawable.ic_error);
	    m_builder.setLargeIcon(BitmapFactory.decodeResource(QtNative.activity().getResources(), R.drawable.ic_error));
	    break;
        }
	
	if (!"".equals(longText)) {
	    m_builder.setStyle(new Notification.BigTextStyle().bigText(longText));
        }
	
	m_notificationManager.notify(id, m_builder.build());
    }
    
    
    /* Begin to monitor network changes */
    public static void startWiFiMonitor()
    {
	Log.d("enroute flight navigation", "startWiFiMonitor");
	
	// Look for WiFi changes
	IntentFilter intentFilter = new IntentFilter();
	intentFilter.addAction(WifiManager.NETWORK_STATE_CHANGED_ACTION);
	m_instance.registerReceiver(m_wifiStateChangeReceiver, intentFilter);
    }
    
    
    /* Vibrate once, very briefly */
    public static void vibrateBrief()
    {
	if (m_vibrator == null) {
	    m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
	}
	m_vibrator.vibrate(20);
    }
    
    
    //
    // Embedded classes
    //
    
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
    
    
    private class NotifyClickReceiver extends BroadcastReceiver
    {
	@Override
	public void onReceive(Context context, Intent intent) {
	    Log.d("enroute flight navigation", "onReceive " + intent.getIntExtra("NotificationID", -1));

            Intent i = new Intent(context, MobileAdaptor.class);
            i.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TOP);
            context.startActivity(i);

            onNotificationClicked(intent.getIntExtra("NotificationID", -1));
	}
    }
    
}
