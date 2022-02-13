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

#pragma once

#include <QDBusAbstractInterface>
#include <QMap>

#include "platform/Notifier_Abstract.h"

namespace Platform {

/*! \brief This class shows platform-native notifications to the user.
 *
 *  The enum NotificationType names a number pre-defined notifications that can be
 *  shown to the user with the method showNotification(). The method hideNotification()
 *  removes a notification. The signal notificationClicked() is emitted when the user
 *  clicks on a notification.
 */

class Notifier : public Notifier_Abstract
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit Notifier(QObject* parent = nullptr);

    ~Notifier();

public slots:
    /*! \brief Hides a notification
     *
     *  This method hides a notification that is currently shown.  If the notification is not
     *  shown, this method does nothing.
     *
     *  @param notificationType Type of the notification
     */
    Q_INVOKABLE virtual void hideNotification(Platform::Notifier_Abstract::NotificationTypes notificationType);

    /*! \brief Shows a notification
     *
     *  This method shows a notification to the user. On platforms where notifications have
     *  titles, an appropriate (translated) title is shown.
     *
     *  @param notificationType Type of the notification
     *
     *  @param text One-line notification text ("Device INOP · Maintenance required · Battery low")
     *
     *  @param longText If not empty, then the notification might be expandable. When expanded, the one-line "text" is replaced by the content of this "longText".
     *  Depending on the platform, this parameter might also be ignored.
     */
    virtual void showNotification(Platform::Notifier_Abstract::NotificationTypes notificationType, const QString& text, const QString& longText);

private slots:
    void onActionInvoked(uint id, QString key);
    void onNotificationClosed(uint id, uint reason);

private:
    Q_DISABLE_COPY_MOVE(Notifier)

    QMap<Platform::Notifier_Abstract::NotificationTypes, uint> notificationIDs;
    QPointer<QDBusInterface> notficationInterface;
};

}
