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

#include <QCoreApplication>
#include <QDir>
#include <QPointer>
#include <QStandardPaths>
#include <QTimer>

#include "Global.h"
#include "MobileAdaptor.h"
#include "geomaps/GeoMapProvider.h"
#include "traffic/TrafficDataProvider.h"


void MobileAdaptor::hideSplashScreen()
{
}


void MobileAdaptor::lockWifi(bool lock)
{
    Q_UNUSED(lock)
}


Q_INVOKABLE auto MobileAdaptor::missingPermissionsExist() -> bool
{
    Q_UNUSED(this);
    return false;
}


void MobileAdaptor::vibrateBrief()
{
}


auto MobileAdaptor::getSSID() -> QString
{
    return "<unknown ssid>";
}


void MobileAdaptor::hideNotification(NotificationType notificationType)
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


void MobileAdaptor::showDownloadNotification(bool show)
{

    if (show) {
        if (downloadNotification.isNull()) {
            downloadNotification = new KNotification(QStringLiteral("downloading"), KNotification::Persistent, this);
            downloadNotification->setDefaultAction( tr("Open Application") );
            downloadNotification->setPixmap( {":/icons/appIcon.png"} );
            downloadNotification->setText(tr("Downloading map data…"));
        }
        downloadNotification->sendEvent();
        connect(downloadNotification, &KNotification::defaultActivated, [this]() { emit notificationClicked(NotificationType::DownloadInfo); });
    } else {
        if (!downloadNotification.isNull()) {
            downloadNotification->close();
        }
    }

}


void MobileAdaptor::showNotification(NotificationType notificationType, QString message)
{
    if (message.isEmpty()) {
        hideNotification(notificationType);
        return;
    }

    // Get notification, if it exists; otherwise get nullptr
    auto notification = notifications.value(notificationType, nullptr);

    // Close and delete notification if message is empty
    if (message.isEmpty()) {
        if (!notification.isNull()) {
            notification->close();
            delete notification;
        }

        if (notifications.contains(notificationType)) {
            notifications.remove(notificationType);
        }
        return;
    }

    // Otherwise, generate a new notification
    if (notification.isNull()) {
        switch (notificationType) {
        case DownloadInfo:
            notification = new KNotification(QStringLiteral("downloading"), KNotification::Persistent, this);
            notification->setDefaultAction( tr("Open Application") );
            notification->setPixmap( {":/icons/appIcon.png"} );
            notification->setText(tr("Downloading map data…"));
            break;
        case TrafficReceiverSelfTestError:
        case TrafficReceiverProblem:
            notification = new KNotification(QStringLiteral("trafficReceiverProblem"), KNotification::Persistent, this);
            notification->setDefaultAction( tr("Open Application") );
            notification->setPixmap( {":/icons/appIcon.png"} );
            notification->setTitle(tr("Traffic Receiver Problem"));
            break;
        }
    }

    notifications[notificationType] = notification;
    connect(notification, &KNotification::defaultActivated, [this, notificationType]() { emit notificationClicked(notificationType); });
    notification->setText(message);
    notification->sendEvent();

}
