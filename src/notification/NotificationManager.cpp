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

#include <QQmlEngine>
#include <QSettings>
#include <QStandardPaths>

#include "GlobalObject.h"
#include "dataManagement/DataManager.h"
#include "navigation/Navigator.h"
#include "notification/NotificationManager.h"
#include "notification/Notification_DataUpdateAvailable.h"
#include "traffic/TrafficDataProvider.h"


//
// Constructors and destructors
//

Notifications::NotificationManager::NotificationManager(QObject *parent) : GlobalObject(parent)
{
}

void Notifications::NotificationManager::deferredInitialization()
{
    connect(GlobalObject::trafficDataProvider(), &Traffic::TrafficDataProvider::trafficReceiverRuntimeErrorChanged,
            this, &Notifications::NotificationManager::onTrafficReceiverRuntimeError);
    connect(GlobalObject::trafficDataProvider(), &Traffic::TrafficDataProvider::trafficReceiverSelfTestErrorChanged,
            this, &Notifications::NotificationManager::onTrafficReceiverSelfTestError);

    // Maps and Data
    connect(GlobalObject::dataManager()->mapsAndData(), &DataManagement::Downloadable_Abstract::updateSizeChanged,
            this, &Notifications::NotificationManager::onMapAndDataUpdateSizeChanged);
    connect(GlobalObject::dataManager()->mapsAndData(), &DataManagement::Downloadable_Abstract::downloadingChanged,
            this, &Notifications::NotificationManager::onMapAndDataDownloadingChanged);
    connect(&mapsAndDataNotificationTimer, &QTimer::timeout,
            this, &Notifications::NotificationManager::onMapAndDataUpdateSizeChanged);

    mapsAndDataNotificationTimer.setInterval(11min);
    mapsAndDataNotificationTimer.setSingleShot(true);

    onMapAndDataUpdateSizeChanged();
    onMapAndDataUpdateSizeChanged();
}


//
// Getter Methods
//

Notifications::Notification* Notifications::NotificationManager::currentNotification() const {
    if (m_notifications.isEmpty())
    {
        return nullptr;
    }
    return m_notifications[0];
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
    if (m_notifications.contains(notification))
    {
        return;
    }

    // Re-parent the notification
    notification->setParent(this);
    QQmlEngine::setObjectOwnership(notification, QQmlEngine::CppOwnership);

    // Wire up
    connect(notification, &Notifications::Notification::destroyed, this, &Notifications::NotificationManager::updateNotificationList);
    connect(notification, &Notifications::Notification::importanceChanged, this, &Notifications::NotificationManager::updateNotificationList);

    // Append and update
    m_notifications.append(notification);
    updateNotificationList();
}

void Notifications::NotificationManager::onMapAndDataDownloadingChanged()
{
    auto* mapsAndData = GlobalObject::dataManager()->mapsAndData();
    if (mapsAndData == nullptr)
    {
        return;
    }
    if (mapsAndData->downloading())
    {
        // Notify!
        auto* notification = new Notifications::Notification(this);
        notification->setTitle(tr("Downloading map and data…"));
                               connect(GlobalObject::dataManager()->mapsAndData(), &DataManagement::Downloadable_MultiFile::downloadingChanged, notification, &QObject::deleteLater);
        addNotification(notification);
    }

}

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
    auto* notification = new Notifications::Notification(this);
    notification->setTitle(tr("Traffic data receiver problem"));
    notification->setText(error);
    notification->setImportance(Notifications::Notification::Warning);
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
    auto* notification = new Notifications::Notification(this);
    notification->setTitle(tr("Traffic data receiver self test error"));
    notification->setText(error);
    notification->setImportance(Notifications::Notification::Warning);
    connect(GlobalObject::trafficDataProvider(),
            &Traffic::TrafficDataProvider::trafficReceiverSelfTestErrorChanged,
            notification,
            &QObject::deleteLater);
    addNotification(notification);
}

void Notifications::NotificationManager::updateNotificationList()
{
    m_notifications.removeAll(nullptr);
    std::sort(m_notifications.begin(), m_notifications.end(),
              [](const Notification* a, const Notification* b) { if (a->importance() == b->importance()) return a->reactionTime() < b->reactionTime(); return a->importance() > b->importance(); });

    if (currentNotificationCache == currentNotification())
    {
        return;
    }
    currentNotificationCache = currentNotification();
    emit currentNotificationChanged();
}
