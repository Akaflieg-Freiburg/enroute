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

#include "platform/Notifier_Linux.h"


// This is a template file without actual implementation.

Platform::Notifier::Notifier(QObject *parent)
    : Platform::Notifier::Notifier_Abstract(parent)
{
    // Standard constructor. Recall that the constructor must not call virtual functions.
    // If you need virtual functions, use the methode deferredInitialization below.
#warning Not implemented
}


void Platform::Notifier::deferredInitialization()
{
    // This method is called immediately after the instance has been constructed.
    // It can be used to implement initialization that calls virtual methods.
#warning Not implemented
}


void Platform::Notifier::hideNotification(Platform::Notifier_Abstract::NotificationTypes notificationType)
{
    // This method is supposed to hide the notification "notificationType".
#warning Not implemented
}


void Platform::Notifier::showNotification(NotificationTypes notificationType, const QString& text, const QString& longText)
{
    // This method is supposed to show the notification "notificationType".
#warning Not implemented
}
