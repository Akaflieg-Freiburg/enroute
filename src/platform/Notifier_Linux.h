/***************************************************************************
 *   Copyright (C) 2021-2022 by Stefan Kebekus                             *
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

#pragma once

#include <QDBusInterface>
#include <QMap>

#include "platform/Notifier.h"

namespace Platform {

// This class implements notifications for the linux desktop

class Notifier_Linux: public Notifier
{
    Q_OBJECT

public:
    // Constructor
    explicit Notifier_Linux(QObject* parent = nullptr);

    // Destructor
    ~Notifier_Linux();

public slots:
    // Implementation of pure virtual function
    Q_INVOKABLE virtual void hideNotification(Platform::Notifier::NotificationTypes notificationType);

    // Implementation of pure virtual function
    virtual void showNotification(Platform::Notifier::NotificationTypes notificationType, const QString& text, const QString& longText);

private slots:
    // This slot receives ActionInvoked messages from the DBus
    void onActionInvoked(uint id, QString key);

    // This slot receives NotificationClosed messages from the DBus
    void onNotificationClosed(uint id, uint reason);

private:
    Q_DISABLE_COPY_MOVE(Notifier_Linux)

    // Maps NotificationTypes, to IDs of the ongoing notation for the NotificationType
    QMap<Platform::Notifier::NotificationTypes, uint> notificationIDs;

    // QDBusInterface to org.freedesktop.Notifications
    // This implementation of notifications uses the specification found here:
    // https://specifications.freedesktop.org/notification-spec/latest/ar01s09.html
    //
    // Help with DBus programming is found here:
    // https://develop.kde.org/docs/d-bus/accessing_dbus_interfaces/
    QDBusInterface notificationInterface {"org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications", QDBusConnection::sessionBus()};
};

}
