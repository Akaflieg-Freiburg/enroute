/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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


namespace Notification {

#warning

class NotificationManager : public GlobalObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief Notification types
     *
     *  This enum lists a number of predefined notification types
     *  only these notifications can be shown.
     */
    enum NotificationTypes
    {
        DownloadInfo = 0,                 /*< Info that  download is in progress */
        TrafficReceiverSelfTestError = 1, /*< Traffic receiver reports problem on self-test */
        TrafficReceiverRuntimeError = 2,  /*< Traffic receiver reports problem while running */
        GeoMapUpdatePending = 3           /*< Updates of geographic maps are available */
    };
    Q_ENUM(NotificationTypes)

    /*! \brief Notification actions
     *
     *  This enum lists a number of actions that the user can undertake when a notification is shown.
     */
    enum NotificationActions
    {
        DownloadInfo_Clicked,                 /*< User clicks on body of download notification */
        TrafficReceiverSelfTestError_Clicked, /*< User clicks on body of traffic receiver self-test problem report */
        TrafficReceiverRuntimeError_Clicked,  /*< User clicks on body of traffic receiver runtime problem report */
        GeoMapUpdatePending_Clicked,          /*< User clicks on body of update message */
        GeoMapUpdatePending_UpdateRequested   /*< User requests geo map update */
    };
    Q_ENUM(NotificationActions)


    //
    // Constructors and destructors
    //

    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit NotificationManager(QObject* parent = nullptr);

    // deferred initialization
    void deferredInitialization() override;

    // No default constructor, important for QML singleton
    explicit NotificationManager() = delete;

    /*! \brief Standard destructor */
    ~NotificationManager() override = default;

    // factory function for QML singleton
    static Notification::NotificationManager* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::notificationManager();
    }


    //
    // PROPERTIES
    //

    Q_PROPERTY(QString currentNotificationTitle READ currentNotificationTitle NOTIFY currentNotificationTitleChanged)
    QString currentNotificationTitle();

    Q_PROPERTY(QString currentNotificationText READ currentNotificationText NOTIFY currentNotificationTextChanged)
    QString currentNotificationText();

    Q_PROPERTY(QString currentNotificationButton1Text READ currentNotificationButton1Text NOTIFY currentNotificationButton1TextChanged)
    QString currentNotificationButton1Text();

    Q_PROPERTY(QString currentNotificationButton2Text READ currentNotificationButton2Text NOTIFY currentNotificationButton2TextChanged)
    QString currentNotificationButton2Text();

    Q_PROPERTY(bool currentNotificationVisible READ currentNotificationVisible NOTIFY currentNotificationVisibleChanged)
    bool currentNotificationVisible();

    void hideNotification(Notification::NotificationManager::NotificationTypes notificationType);
    void hideAll();
    void showNotification(Notification::NotificationManager::NotificationTypes notificationType, const QString& text, const QString& longText);

signals:
    void currentNotificationTitleChanged();
    void currentNotificationTextChanged();
    void currentNotificationButton1TextChanged();
    void currentNotificationButton2TextChanged();
    void currentNotificationVisibleChanged();

    void action(Notification::NotificationManager::NotificationActions action);

};

} // namespace Notification
