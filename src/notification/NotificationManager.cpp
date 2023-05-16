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

Notifications::NotificationManager::NotificationManager(QObject *parent) : GlobalObject(parent)
{
    testNotification = new Notifications::Notification(this);
    testNotification->setTitle(u"This is the title"_qs);
    testNotification->setText(u"This is a dummy text. Beautification lies in the eye of the beholder. I will come back to that later."_qs);
    testNotification->setButton1Text(u"Dismiss"_qs);
    testNotification->setButton2Text(u"Cancel"_qs);
    testNotification->setDismissed(false);
    add(testNotification);
}


void Notifications::NotificationManager::deferredInitialization()
{
}


void Notifications::NotificationManager::add(Notifications::Notification* notification)
{
    if (notification == nullptr)
    {
        return;
    }
    if (m_notifications.contains(notification))
    {
        return;
    }
    notification->setParent(this);
    QQmlEngine::setObjectOwnership(notification, QQmlEngine::CppOwnership);
    m_notifications.append(QSharedPointer<Notifications::Notification>(notification));

    connect(notification, &Notifications::Notification::dismissedChanged, this, &Notifications::NotificationManager::update);
}

void Notifications::NotificationManager::update()
{
    auto *currentNotificationCache = currentNotification();

    QVector<QSharedPointer<Notifications::Notification>> updatedNotifications;
    foreach(auto notification, m_notifications)
    {
        if (notification->dismissed() == false)
        {
            updatedNotifications.append(notification);
        }
    }
    m_notifications = updatedNotifications;
    if (currentNotification() == currentNotificationCache)
    {
        return;
    }
    emit currentNotificationChanged();
}
