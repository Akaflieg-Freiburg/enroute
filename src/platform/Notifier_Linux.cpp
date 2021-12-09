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

#include <QApplication>
#include <QDBusInterface>
#include <QDBusReply>

#include "platform/Notifier.h"


QMap<Platform::Notifier::NotificationTypes, uint> notificationIDs;
QPointer<QDBusInterface> notficationInterface;

// This implementation of notifications uses the specification found here:
// https://specifications.freedesktop.org/notification-spec/latest/ar01s09.html
//
// Help with DBus programming is found here:
// https://develop.kde.org/docs/d-bus/accessing_dbus_interfaces/



// This method returns a pointer to the QDBusInterface used to access notifications.
// It constructs the object, if necessary.
// We do not implement this in the constructor because the construction
// takes time, which would add to the startup delay.

QDBusInterface* getNotificationInterface()
{
    if (notficationInterface.isNull()) {
        notficationInterface = new QDBusInterface("org.freedesktop.Notifications",
                                                  "/org/freedesktop/Notifications",
                                                  "org.freedesktop.Notifications",
                                                  QDBusConnection::sessionBus());
        notficationInterface->setParent(qApp);
    }
    return notficationInterface;
}


Platform::Notifier::Notifier(QObject *parent)
    : QObject(parent)
{
}


Platform::Notifier::~Notifier()
{
    delete notficationInterface;
}


void Platform::Notifier::hideNotification(Platform::Notifier::NotificationTypes notificationType)
{

    auto notificationID = notificationIDs.value(notificationType, 0);
    if (notificationID == 0) {
        return;
    }

    getNotificationInterface()->call("CloseNotification", notificationID);
    notificationIDs.remove(notificationType);

}


void Platform::Notifier::showNotification(NotificationTypes notificationType, const QString& text, const QString& longText)
{

    QDBusReply<uint> reply = getNotificationInterface()->call("Notify",
                                                              "Enroute Flight Navigation",
                                                              notificationIDs.value(notificationType, 0), // Notification to replace. 0 means: do not replace
                                                              "", // Icon
                                                              "Enroute Flight Navigation", // Title
                                                              title(notificationType), // Summary
                                                              QStringList(), // actions_list
                                                              QMap<QString,QVariant>(), // hint
                                                              0); // time: 0 means: never expire.
    if (reply.isValid()) {
        notificationIDs.insert(notificationType, reply.value());
    }

}
