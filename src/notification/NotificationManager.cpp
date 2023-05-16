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
    auto* testNotification = new Notifications::Notification(this);
    testNotification->setTitle(u"This is the title"_qs);
    testNotification->setText(u"This is a dummy text. Beautification lies in the eye of the beholder. I will come back to that later."_qs);
    testNotification->setButton1Text(u"Dismiss"_qs);
    testNotification->setButton2Text(u"Cancel"_qs);
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

    // Re-parent the notification
    notification->setParent(this);
    QQmlEngine::setObjectOwnership(notification, QQmlEngine::CppOwnership);

    // Wire up
    connect(notification, &Notifications::Notification::destroyed, this, &Notifications::NotificationManager::update);
    connect(notification, &Notifications::Notification::importanceChanged, this, &Notifications::NotificationManager::update);

    // Append and update
    m_notifications.append(notification);
    update();
}

void Notifications::NotificationManager::update()
{
    m_notifications.removeAll(nullptr);
    std::sort(m_notifications.begin(), m_notifications.end(), [](const Notification* a, const Notification* b)
              { return a->importance() > b->importance(); });

    if (currentNotificationCache == currentNotification())
    {
        return;
    }
    qWarning() << "Update" << m_notifications;
    currentNotificationCache = currentNotification();
    emit currentNotificationChanged();
}
