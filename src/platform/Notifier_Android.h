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

#include "platform/Notifier.h"

namespace Platform {

// This class implements notifications for the linux desktop

class Notifier_Android: public Notifier
{
    Q_OBJECT

public:
    // Constructor
    explicit Notifier_Android(QObject* parent = nullptr);

    // Destructor
    ~Notifier_Android();

public slots:
    // Implementation of pure virtual function
    Q_INVOKABLE virtual void hideNotification(Platform::Notifier::NotificationTypes notificationType);

    // Implementation of pure virtual function
    virtual void showNotification(Platform::Notifier::NotificationTypes notificationType, const QString& text, const QString& longText);

    // This method is called from JAVA when the user interacts with the notification, via the exported "C" function
    // Java_de_akaflieg_1freiburg_enroute_MobileAdaptor_onNotificationClicked
    //
    // @param notificationType … the type of the notification that the user interacted with
    //
    // @param actionID - "0" when the user clicked into the notification body, not zero when the user clicked on a notification action
    void onNotificationClicked(Platform::Notifier::NotificationTypes notificationType, int actionID);
private:
    Q_DISABLE_COPY_MOVE(Notifier_Android)
};

}
