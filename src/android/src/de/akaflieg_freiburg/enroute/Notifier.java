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
import android.graphics.Color;


public class Notifier
{
    public static native void onNotificationClicked(int notifyID);
    
    private static NotificationManager     m_notificationManager;
    private static Notification.Builder    m_builder;
  
    
    //
    // Static Methods
    //
          
    /* Show traffic receiver error notification */
    public static void hideNotification(int id)
    {
	m_notificationManager = (NotificationManager)QtNative.activity().getSystemService(Context.NOTIFICATION_SERVICE);
        m_notificationManager.cancel(id);
    }

    
    /* Show traffic receiver error notification */
    public static void showNotification(int id, String title, String text, String longText)
    {
	// Get notification manager
	m_notificationManager = (NotificationManager) QtNative.activity().getSystemService(Context.NOTIFICATION_SERVICE);
	
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
	    m_builder = new Notification.Builder(QtNative.activity(), notificationChannel.getId());
	} else {
	    m_builder = new Notification.Builder(QtNative.activity());
	}
	
        Intent notificationIntent = new Intent("de.akaflieg_freiburg.enroute.onNotificationClick");
	notificationIntent.putExtra("NotificationID", id);
	PendingIntent pendingIntent = PendingIntent.getBroadcast(QtNative.activity(), 0, notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT) ;
	
	m_builder.setContentIntent(pendingIntent);
	m_builder.setColor(Color.rgb(00,0x80,0x80));
	m_builder.setContentTitle(title);
	m_builder.setContentText(text);
	m_builder.setOngoing(false);
	m_builder.setAutoCancel(false);
	if (!"".equals(longText)) {
	    m_builder.setStyle(new Notification.BigTextStyle().bigText(longText));
	}
	
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
	
	m_notificationManager.notify(id, m_builder.build());
    }
    
}
