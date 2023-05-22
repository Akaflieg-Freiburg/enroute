/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include <QFuture>
#include <QQmlEngine>
#include <QTextToSpeech>
#include <QTimer>

#include "GlobalObject.h"
#include "Notification.h"


namespace Notifications {

/*! \brief This class manages notifications and presents them to the GUI
 *
 *  This class manages notifications. It watches the global objects in the app, creates notifications, sorts them according
 *  to importance, and presents the most important notification to the GUI.
 */

class NotificationManager : public GlobalObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
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
    ~NotificationManager();

    // factory function for QML singleton
    static Notifications::NotificationManager* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::notificationManager();
    }


    //
    // PROPERTIES
    //

    /*! \brief Most important notification
     *
     *  This property holds the most important notification, or nullptr if there is no notification. The notifications
     *  are owned by this NotificationManager and have ownership set to QQmlEngine::CppOwnership.
     *
     *  @note The notification objects returned here can be deleted anytime, so it is wise to store the result in a QPointer that tracks deletion.
     */
    Q_PROPERTY(Notifications::Notification* currentNotification READ currentNotification NOTIFY currentNotificationChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property currentNotification
     */
    [[nodiscard]] Notifications::Notification* currentNotification() const;


    //
    // Methods
    //

    /*! \brief Issue test notification */
    Q_INVOKABLE void showTestNotification();

signals:
    /*! \brief Notification signal */
    void currentNotificationChanged();

private slots:
    // This method clears all nullptrs from m_notifications, sorts the elements
    // by importance and emits currentNotificationChanged() whenever the first element
    // the list changes.
    void updateNotificationList();

    // Called whenever the traffic receiver reports a runtime error,
    // or clears the error status.
    void onTrafficReceiverRuntimeError();

    // Called whenever the traffic receiver reports a self test error,
    // or clears the error status.
    void onTrafficReceiverSelfTestError();

    // Called whenever map and data updates become (un)available
    void onMapAndDataUpdateSizeChanged();

    // Called whenever map and data are being downloaded.
    void onMapAndDataDownloadingChanged();

private:
    // Adds a notification to m_notifications and rearranges the list.
    // Removes nullptrs and sorts list by importance.
    void addNotification(Notifications::Notification* notification);

    // List of Notifications, sorted so that the most important notification comes first.
    QVector<QPointer<Notifications::Notification>> m_visualNotifications;


    // When notifications for maps and data are temporarily not possible, then use this timer
    // to notify again.
    QTimer mapsAndDataNotificationTimer;

    // Internal to updateNotificationList
    Notifications::Notification* currentNotificationCache {nullptr};

    //
    // Members used for spoken text
    //

    // List of Notifications, sorted so that the most important notification comes first.
    QVector<QPointer<Notifications::Notification>> m_spokenNotifications;

    // Pointer to the speaker. This will be the nullpointer while the thread the constructs
    // the object is still ongoing.
    QTextToSpeech* m_speaker {nullptr};

    // Future for the worker that constructs QTextToSpeek under Linux in a different thread
    QFuture<void> m_speakerFuture;

    // This timer is used to stop for one second between two spoken texts.
    // The slot start() is called when m_speaker is done speaking.
    // The signal timeout() is connected to speakNext().
    QTimer m_speechBreakTimer;

    // If the speaker is finished, start m_speechBreakTimer, in order to call speakNext()
    // after a one-second break.
    void onSpeakerStateChanged(QTextToSpeech::State state);

    // Setup speaker: construct the speaker, move it to the GUI thread, make it a
    // child of this and wire it up.
    void setupSpeaker();

    // This method cleans the list m_spokenNotifications. If a notification is in the list,
    // it speaks the
    void speakNext();
};


} // namespace Notifications
