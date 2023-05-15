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


QString Notification::NotificationManager::currentNotificationTitle()
{
#warning
    return u"Test Notification"_qs;
}


QString Notification::NotificationManager::currentNotificationText()
{
#warning
    return u"Schön ist das jetzt nicht, aber da kümmere ich mich später drum. - Stefan."_qs;
}

QString Notification::NotificationManager::currentNotificationButton1Text()
{
#warning
    return u"Button1"_qs;
}

QString Notification::NotificationManager::currentNotificationButton2Text()
{
#warning
    return u"Button2"_qs;
}

bool Notification::NotificationManager::currentNotificationVisible()
{
#warning
    return true;
}

void Notification::NotificationManager::hideNotification(Notification::NotificationManager::NotificationTypes notificationType)
{
#warning
}

void Notification::NotificationManager::hideAll()
{
#warning
}

void Notification::NotificationManager::showNotification(Notification::NotificationManager::NotificationTypes notificationType, const QString& text, const QString& longText)
{
#warning
}
