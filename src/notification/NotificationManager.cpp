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

#include <QQmlEngine>
#include <QStandardPaths>

#include "GlobalObject.h"
#include "notification/NotificationManager.h"


//
// Constructors and destructors
//

Notification::NotificationManager::NotificationManager(QObject *parent) : GlobalObject(parent)
{
}


void Notification::NotificationManager::deferredInitialization()
{
}


void Notification::NotificationManager::hideNotification(Notification::NotificationManager::NotificationTypes notificationType)
{
    _visible = false;
    emit currentNotificationVisibleChanged();
}

void Notification::NotificationManager::hideAll()
{
    _visible = false;
    emit currentNotificationVisibleChanged();
}

void Notification::NotificationManager::showNotification(Notification::NotificationManager::NotificationTypes notificationType, const QString& text, const QString& longText)
{
    _title = {};
    _text = text;
    _button1Text = {};
    _button2Text = {};
    _visible = false;

    if (notificationType == DownloadInfo)
    {
        _title = tr("Downloading map and data…");
                    _button1Text = tr("Dismiss");
        _button2Text = tr("Update");
                    _visible = true;
    }
    if (notificationType == TrafficReceiverRuntimeError)
    {
                    _title = tr("Traffic data receiver problem");
                    _visible = true;
    }
    if (notificationType == TrafficReceiverSelfTestError)
    {
                    _title = tr("Traffic data receiver self test error");
                    _visible = true;
    }
    if (notificationType == GeoMapUpdatePending)
    {
                    _title = tr("Map and data updates available");
                    _visible = true;
    }

    emit currentNotificationTitleChanged();
    emit currentNotificationTextChanged();
    emit currentNotificationButton1TextChanged();
    emit currentNotificationButton2TextChanged();
    emit currentNotificationVisibleChanged();
}
