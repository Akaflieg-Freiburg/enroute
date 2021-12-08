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

#include <QDBusInterface>
#include <QDebug>

#include "platform/Notifier.h"


QMap<Platform::Notifier::Notifications, uint> notificationIDs;


Platform::Notifier::Notifier(QObject *parent)
    : QObject(parent)
{
    ;
}


Platform::Notifier::~Notifier()
{
    QDBusInterface notify("org.freedesktop.Notifications",
                          "/org/freedesktop/Notifications",
                          "org.freedesktop.Notifications",
                          QDBusConnection::sessionBus());

    foreach(auto notificationID, notificationIDs) {
        if (notificationID == 0) {
            continue;
        }
        auto reply = notify.call("CloseNotification", notificationID); // time: 0 means: never expire.
        if (reply.type() != QDBusMessage::ReplyMessage) {
            qWarning() << reply.errorName() << reply.errorMessage();
        }
    }

}


void Platform::Notifier::hideNotification(Platform::Notifier::Notifications notificationType)
{
qWarning() << "hide" << notificationType;
    if (!notificationIDs.contains(notificationType)) {
        return;
    }

    auto notificationID = notificationIDs.value(notificationType, 0);
    if (notificationID == 0) {
        return;
    }

    QDBusInterface notify("org.freedesktop.Notifications",
                          "/org/freedesktop/Notifications",
                          "org.freedesktop.Notifications",
                          QDBusConnection::sessionBus());
    auto reply = notify.call("CloseNotification", notificationID); // time: 0 means: never expire.
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << reply.errorName() << reply.errorMessage();
    }
    notificationIDs.remove(notificationType);

}


void Platform::Notifier::showNotification(Notifications notification, const QString& text, const QString& longText)
{
    qWarning() << "show" << notification;

    QDBusInterface notify("org.freedesktop.Notifications",
                          "/org/freedesktop/Notifications",
                          "org.freedesktop.Notifications",
                          QDBusConnection::sessionBus());

    auto reply = notify.call("Notify",
                             "Enroute Flight Navigation",
                             notificationIDs.value(notification, 0), // Notification to replace. 0 means: do not replace
                             "de.akaflieg_freiburg.enroute",
                             title(notification),
                             text,
                             QStringList(), // actions_list
                             QMap<QString,QVariant>(), // hint
                             0); // time: 0 means: never expire.
    if (reply.type() == QDBusMessage::ReplyMessage) {
        if (reply.arguments().size() > 0) {
            bool ok = false;
            auto number = reply.arguments().at(0).toUInt(&ok);
            if (ok) {
                notificationIDs.insert(notification, number);
            }
        }
    } else {
        qWarning() << reply.errorName() << reply.errorMessage();
    }

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
