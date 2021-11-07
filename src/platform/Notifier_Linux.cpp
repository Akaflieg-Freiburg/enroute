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

#warning
//#include <KNotification>

#include "platform/Notifier.h"

#warning
// QMap<Platform::Notifier::Notifications, QPointer<KNotification>> notificationPtrs;


Platform::Notifier::Notifier(QObject *parent)
    : QObject(parent)
{
    ;
}


Platform::Notifier::~Notifier()
{
#warning
/*
    foreach(auto notification, notificationPtrs) {
        if (notification.isNull()) {
            continue;
        }
        notification->close();
        delete notification;
    }
*/
}


void Platform::Notifier::hideNotification(Platform::Notifier::Notifications notificationType)
{
#warning
/*
    auto notification = notificationPtrs.value(notificationType, nullptr);
    if (!notification.isNull()) {
        notification->close();
        delete notification;
    }

    if (notificationPtrs.contains(notificationType)) {
        notificationPtrs.remove(notificationType);
    }
*/
}


void Platform::Notifier::showNotification(Notifications notification, const QString& text, const QString& longText)
{
#warning
/*
    // Get notificonst cation, &if it exists; otherwise get nullptr
    auto notificationPtr = notificationPtrs.value(notification, nullptr);

    // Otherwise, generate a new notification
    if (notificationPtr.isNull()) {
        switch (notification) {
        case DownloadInfo:
            notificationPtr = new KNotification(QStringLiteral("downloading"), KNotification::Persistent, this);
            break;
        case TrafficReceiverSelfTestError:
            notificationPtr = new KNotification(QStringLiteral("trafficReceiverSelfTestError"), KNotification::Persistent, this);
            break;
        case TrafficReceiverRuntimeError:
            notificationPtr = new KNotification(QStringLiteral("trafficReceiverRuntimeError"), KNotification::Persistent, this);
            break;
        }
        notificationPtr->setDefaultAction( tr("Open Application") );
        notificationPtr->setPixmap( {":/icons/appIcon.png"} );
    }

    notificationPtr->setTitle(title(notification));
    notificationPtrs[notification] = notificationPtr;
    connect(notificationPtr, &KNotification::defaultActivated, [this, notification]() { emit notificationClicked(notification); });
    if (longText.isEmpty()) {
        notificationPtr->setText(text);
    } else {
        notificationPtr->setText(longText);
    }
    notificationPtr->sendEvent();
*/
}
