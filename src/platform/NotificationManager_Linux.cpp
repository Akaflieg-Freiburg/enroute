/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <KNotification>

#include "platform/NotificationManager.h"


QMap<Platform::NotificationManager::NotificationType, QPointer<KNotification>> notifications;


Platform::NotificationManager::NotificationManager(QObject *parent)
    : QObject(parent)
{
    ;
}


Platform::NotificationManager::~NotificationManager()
{

    foreach(auto notification, notifications) {
        if (notification.isNull()) {
            continue;
        }
        notification->close();
        delete notification;
    }

}


void Platform::NotificationManager::hideNotification(Platform::NotificationManager::NotificationType notificationType)
{

    auto notification = notifications.value(notificationType, nullptr);
    if (!notification.isNull()) {
        notification->close();
        delete notification;
    }

    if (notifications.contains(notificationType)) {
        notifications.remove(notificationType);
    }

}


void Platform::NotificationManager::showNotification(NotificationType notificationType, const QString& title, const QString& text, const QString& longText)
{

    // Get notificonst cation, &if it exists; otherwise get nullptr
    auto notification = notifications.value(notificationType, nullptr);

    // Otherwise, generate a new notification
    if (notification.isNull()) {
        switch (notificationType) {
        case DownloadInfo:
            notification = new KNotification(QStringLiteral("downloading"), KNotification::Persistent, this);
            break;
        case TrafficReceiverSelfTestError:
        case TrafficReceiverRuntimeError:
            notification = new KNotification(QStringLiteral("trafficReceiverProblem"), KNotification::Persistent, this);
            break;
        }
        notification->setDefaultAction( tr("Open Application") );
        notification->setPixmap( {":/icons/appIcon.png"} );
    }

    notification->setTitle(title);
    notifications[notificationType] = notification;
    connect(notification, &KNotification::defaultActivated, [this, notificationType]() { emit notificationClicked(notificationType); });
    if (longText.isEmpty()) {
        notification->setText(text);
    } else {
        notification->setText(longText);
    }
    notification->sendEvent();

}
