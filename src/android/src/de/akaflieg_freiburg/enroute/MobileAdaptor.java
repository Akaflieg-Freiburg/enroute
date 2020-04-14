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

// Qt
import org.qtproject.qt5.android.QtNative;

import android.content.Intent;
import android.app.PendingIntent;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.os.Vibrator;


public class MobileAdaptor extends org.qtproject.qt5.android.bindings.QtActivity
{
    private static MobileAdaptor        m_instance;

    private static NotificationManager  m_notificationManager;
    private static Notification.Builder m_builder;

    private static Vibrator             m_vibrator;

    public MobileAdaptor()
    {
        m_instance = this;
    }

    /* Vibrate once, very briefly */
    public static void vibrateBrief()
    {
        if (m_vibrator == null)
            m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
        m_vibrator.vibrate(20);
    }
    
    /* Show download notification */
    public static void notifyDownload(boolean show)
    {
        m_notificationManager = (NotificationManager)m_instance.getSystemService(Context.NOTIFICATION_SERVICE);

	// Cancel notification
	if (!show) {
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
	    .setContentTitle("Downloading map data…")
	    .setOngoing(true)
	    .setAutoCancel(false);
	
	m_notificationManager.notify(0, m_builder.build());
    }
    
}
