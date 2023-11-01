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

#include <QCoreApplication>
#include <QQmlEngine>
#include <QSettings>
#include <QStandardPaths>
#include <QtConcurrent>

#include "GlobalObject.h"
#include "GlobalSettings.h"
#include "dataManagement/DataManager.h"
#include "navigation/Navigator.h"
#include "notification/NotificationManager.h"
#include "notification/Notification_DataUpdateAvailable.h"
#include "platform/PlatformAdaptor_Abstract.h"
#include "traffic/TrafficDataProvider.h"
#include <chrono>

using namespace std::chrono_literals;


//
// Constructors and destructors
//

Notifications::NotificationManager::NotificationManager(QObject *parent) : GlobalObject(parent)
{
    // Test: uncomment next line to get a text notification
    // QTimer::singleShot(3s, this, &Notifications::NotificationManager::addTestNotification);
}

void Notifications::NotificationManager::deferredInitialization()
{
#if defined(Q_OS_LINUX) and not defined(Q_OS_ANDROID)
    // Under Linux, the constructor of QTextToSpeech is extremely slow. For that reason we run the constructor in a separate thread.
    m_speakerFuture = QtConcurrent::run([this]() { setupSpeaker();} );
#else
    // On other operating systems, we construct the QTextToSpeech object
    // directly.
    //
    // Note: under Android, QTextToSpeech MUST be created in the GUI thread
    setupSpeaker();
#endif

    m_speechBreakTimer.setInterval(1s);
    m_speechBreakTimer.setSingleShot(true);
    connect(&m_speechBreakTimer, &QTimer::timeout, this, &Notifications::NotificationManager::speakNext);

    connect(GlobalObject::trafficDataProvider(), &Traffic::TrafficDataProvider::trafficReceiverRuntimeErrorChanged,
            this, &Notifications::NotificationManager::onTrafficReceiverRuntimeError);
    connect(GlobalObject::trafficDataProvider(), &Traffic::TrafficDataProvider::trafficReceiverSelfTestErrorChanged,
            this, &Notifications::NotificationManager::onTrafficReceiverSelfTestError);

    // Maps and Data
    connect(GlobalObject::dataManager()->mapsAndData(), &DataManagement::Downloadable_Abstract::updateSizeChanged,
            this, &Notifications::NotificationManager::onMapAndDataUpdateSizeChanged);
    connect(&mapsAndDataNotificationTimer, &QTimer::timeout,
            this, &Notifications::NotificationManager::onMapAndDataUpdateSizeChanged);

    mapsAndDataNotificationTimer.setInterval(11min);
    mapsAndDataNotificationTimer.setSingleShot(true);

    onMapAndDataUpdateSizeChanged();
}



//
// Getter Methods
//

Notifications::Notification* Notifications::NotificationManager::currentVisualNotification() const
{
    Notifications::Notification* result = nullptr;
    if (!m_visualNotifications.isEmpty())
    {
        result = m_visualNotifications[0];
    }
    return result;
}


//
// Methods
//


void Notifications::NotificationManager::addTestNotification()
{
    auto* notification = new Notifications::Notification(tr("Test notification"), Notifications::Notification::Warning);
    notification->setText(u"Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."_qs);
    connect(GlobalObject::dataManager()->mapsAndData(), &DataManagement::Downloadable_MultiFile::downloadingChanged, notification, &QObject::deleteLater);
    addNotification(notification);
}

void Notifications::NotificationManager::voiceTest()
{
    auto* notification = new Notifications::Notification(tr("This is a test of the speech engine."), Notifications::Notification::Info, this);
    connect(GlobalObject::dataManager()->mapsAndData(), &DataManagement::Downloadable_MultiFile::downloadingChanged, notification, &QObject::deleteLater);
    m_voiceNotifications.append(notification);
    QQmlEngine::setObjectOwnership(notification, QQmlEngine::CppOwnership);
    speakNext();
}

void Notifications::NotificationManager::waitForSpeechEngine()
{
#if defined(Q_OS_LINUX) and not defined(Q_OS_ANDROID)
    m_speakerFuture.waitForFinished();
#endif
}




//
// Private Methods
//

void Notifications::NotificationManager::addNotification(Notifications::Notification* notification)
{
    if (notification == nullptr)
    {
        return;
    }

    // Re-parent the notification
    notification->setParent(this);
    QQmlEngine::setObjectOwnership(notification, QQmlEngine::CppOwnership);

    // Show notification if not already shown
    if (!m_visualNotifications.contains(notification))
    {
        // Wire up
        connect(notification, &Notifications::Notification::destroyed, this, &Notifications::NotificationManager::showNext);

        // Append and update
        m_visualNotifications.append(notification);
        showNext();
    }

    // Speak notification if not already spoken, and if chosen in the settings
    if (!m_voiceNotifications.contains(notification))
    {
        auto vn = GlobalObject::globalSettings()->voiceNotifications();
        if ((vn & notification->importance()) != 0)
        {
            // Append to spoken notification list
            m_voiceNotifications.append(notification);
            speakNext();
        }
    }
}



//
// Members used for visual notifications
//

void Notifications::NotificationManager::showNext()
{
    m_visualNotifications.removeAll(nullptr);
    std::sort(m_visualNotifications.begin(), m_visualNotifications.end(),
              [](const Notification* a, const Notification* b) { if (a->importance() == b->importance()) {return a->reactionTime() < b->reactionTime();} return a->importance() > b->importance(); });

    auto* cur = currentVisualNotification();
    if (currentVisualNotificationCache == cur)
    {
        return;
    }
    currentVisualNotificationCache = cur;
    if (cur != nullptr)
    {
        GlobalObject::platformAdaptor()->vibrateLong();
    }
    emit currentVisualNotificationChanged();
}



//
// Members used for voice notifications
//

void Notifications::NotificationManager::onSpeakerStateChanged(QTextToSpeech::State state)
{
    if (state == QTextToSpeech::Ready)
    {
        m_speechBreakTimer.start();
    }
    if (state == QTextToSpeech::Error)
    {
        if (m_speaker != nullptr)
        {
            qWarning() << "QTextToSpeech error" << m_speaker->errorReason() << m_speaker->errorString();
        }
        m_speechBreakTimer.start();
    }
}

void Notifications::NotificationManager::setupSpeaker()
{
    auto *speaker = new QTextToSpeech();
    speaker->moveToThread(thread());
    speaker->setParent(this);
    QQmlEngine::setObjectOwnership(speaker, QQmlEngine::CppOwnership);
    speaker->setLocale(QLocale(GlobalObject::platformAdaptor()->language()));
    connect(speaker, &QTextToSpeech::stateChanged, this, &Notifications::NotificationManager::onSpeakerStateChanged);
    m_speaker = speaker;

    emit speakerChanged();
}

void Notifications::NotificationManager::speakNext()
{
    // Check that the speaker has been constructed successfully. If not, then
    // check back in 2 seconds.
    if (m_speaker == nullptr)
    {
        QTimer::singleShot(2s, this, &Notifications::NotificationManager::speakNext);
        return;
    }

    // At his point, we have a valid speaker object. Make sure that the speaker is ready
    // to speak and that the break between two messages is not running.
    if (m_speaker->state() != QTextToSpeech::Ready)
    {
        return;
    }
    if (m_speechBreakTimer.isActive())
    {
        return;
    }

    // At this point, we have a valid speaker object, and we are at the right moment to say something.

    // Clean the m_spokenNotifications and sort it by importance, most important announcements first
    m_voiceNotifications.removeAll(nullptr);
    std::sort(m_voiceNotifications.begin(), m_voiceNotifications.end(),
              [](const Notification* a, const Notification* b) { if (a->importance() == b->importance()) {return a->reactionTime() < b->reactionTime();} return a->importance() > b->importance(); });

    // If there is nothing to say, then quit
    if (m_voiceNotifications.isEmpty())
    {
        return;
    }

    // Generate text to be spoken.
    auto notification = m_voiceNotifications.takeFirst();
    QStringList textBlocks;
    switch (notification->importance()) {
    case Notifications::Notification::Info:
    case Notifications::Notification::Info_Navigation:
        textBlocks << tr("Info.");
        break;
    case Notifications::Notification::Warning:
    case Notifications::Notification::Warning_Navigation:
        textBlocks << tr("Warning.");
        break;
    case Notifications::Notification::Alert:
        textBlocks << tr("Alert!");
        break;
    }
    if (notification->spokenText().isEmpty())
    {
        textBlocks << notification->title() + ".";
    }
    else
    {
        textBlocks << notification->spokenText();
    }

    // Speak!
    m_speaker->say( textBlocks.join(u" "_qs) );
}



//
// Members used to watch other app components, and to generate notifications
// when necessary
//

void Notifications::NotificationManager::onMapAndDataUpdateSizeChanged()
{
    // If there is no update, then we end here.
    if (GlobalObject::dataManager()->mapsAndData()->updateSize() == 0) {
        return;
    }

    // Do not notify when in flight, but ask again in 11min
    if (GlobalObject::navigator()->flightStatus() == Navigation::Navigator::Flight) {
        mapsAndDataNotificationTimer.start();
        return;
    }

    // Check if last notification is less than four hours ago. In that case, do not notify again,
    // and ask again in 11min.
    QSettings settings;
    auto lastGeoMapUpdateNotification = settings.value(QStringLiteral("lastGeoMapUpdateNotification")).toDateTime();
    if (lastGeoMapUpdateNotification.isValid()) {
        auto secsSinceLastNotification = lastGeoMapUpdateNotification.secsTo(QDateTime::currentDateTimeUtc());
        if (secsSinceLastNotification < static_cast<qint64>(4*60*60)) {
            mapsAndDataNotificationTimer.start();
            return;
        }
    }

    // Notify!
    auto* notification = new Notifications::Notification_DataUpdateAvailable(this);
    addNotification(notification);
    settings.setValue(QStringLiteral("lastGeoMapUpdateNotification"), QDateTime::currentDateTimeUtc());
}

void Notifications::NotificationManager::onTrafficReceiverRuntimeError()
{
    auto error = GlobalObject::trafficDataProvider()->trafficReceiverRuntimeError();
    if (error.isEmpty())
    {
        return;
    }
    auto* notification = new Notifications::Notification(tr("Traffic data receiver problem"), Notifications::Notification::Warning);
    notification->setText(error);
    notification->setTextBodyAction(Notifications::Notification::OpenTrafficReceiverPage);
    connect(GlobalObject::trafficDataProvider(),
            &Traffic::TrafficDataProvider::trafficReceiverRuntimeErrorChanged,
            notification,
            &Notifications::Notification::deleteLater);
    addNotification(notification);
}

void Notifications::NotificationManager::onTrafficReceiverSelfTestError()
{
    auto error = GlobalObject::trafficDataProvider()->trafficReceiverSelfTestError();
    if (error.isEmpty())
    {
        return;
    }
    auto* notification = new Notifications::Notification(tr("Traffic data receiver self test error"), Notifications::Notification::Warning);
    notification->setText(error);
    notification->setTextBodyAction(Notifications::Notification::OpenTrafficReceiverPage);
    connect(GlobalObject::trafficDataProvider(),
            &Traffic::TrafficDataProvider::trafficReceiverSelfTestErrorChanged,
            notification,
            &QObject::deleteLater);
    addNotification(notification);
}
