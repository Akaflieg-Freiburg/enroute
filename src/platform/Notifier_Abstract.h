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

#include <QQmlEngine>

#include "GlobalObject.h"

namespace Platform {

/*! \brief This class shows platform-native notifications to the user.
 *
 *  The enum NotificationType names a number pre-defined notifications that can be
 *  shown to the user with the method showNotification(). The method hideNotification()
 *  removes a notification. The signal notificationClicked() is emitted when the user
 *  clicks on a notification.
 */

class Notifier_Abstract : public GlobalObject
{
    Q_OBJECT
    QML_NAMED_ELEMENT(Notifier)
    QML_SINGLETON

public:
    /*! \brief Standard constructor
     *
     * @param parent Standard QObject parent pointer
    */
    explicit Notifier_Abstract(QObject* parent = nullptr);

    // No default constructor, important for QML singleton
    explicit Notifier_Abstract() = delete;

    ~Notifier_Abstract() override = default;

    // factory function for QML singleton
    static Platform::Notifier_Abstract* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::notifier();
    }

    /*! \brief Notification types
     *
     *  This enum lists a number of predefined notification types
     *  only these notifications can be shown.
     */
    enum NotificationTypes
    {
        TrafficReceiverSelfTestError = 1, /*< Traffic receiver reports problem on self-test */
        TrafficReceiverRuntimeError = 2   /*< Traffic receiver reports problem while running */
    };
    Q_ENUM(NotificationTypes)

    /*! \brief Notification actions
     *
     *  This enum lists a number of actions that the user can undertake when a notification is shown.
     */
    enum NotificationActions
    {
        TrafficReceiverSelfTestError_Clicked, /*< User clicks on body of traffic receiver self-test problem report */
        TrafficReceiverRuntimeError_Clicked  /*< User clicks on body of traffic receiver runtime problem report */
    };
    Q_ENUM(NotificationActions)

public slots:
    /*! \brief Hides a notification
     *
     *  This method hides a notification that is currently shown.  If the notification is not
     *  shown, this method does nothing.
     *
     *  @param notificationType Type of the notification
     */
    virtual void hideNotification(Platform::Notifier_Abstract::NotificationTypes notificationType) = 0;

    /*! \brief Hides all notifications */
    void hideAll();

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
    virtual void showNotification(Platform::Notifier_Abstract::NotificationTypes notificationType, const QString& text, const QString& longText) = 0;

signals:
    /*! \brief User action
     *
     * This signal is emitted in response to user interaction with a notification
     */
    void action(Platform::Notifier_Abstract::NotificationActions action);

protected:
    // Get translated title for specific notification
    static auto title(Platform::Notifier_Abstract::NotificationTypes notificationType) -> QString;

private:
    Q_DISABLE_COPY_MOVE(Notifier_Abstract)
};

} // namespace Platform
