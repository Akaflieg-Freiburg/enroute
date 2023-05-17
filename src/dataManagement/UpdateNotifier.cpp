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

#include <QCoreApplication>
#include <QSettings>
#include <chrono>

#include "GlobalObject.h"
#include "UpdateNotifier.h"
#include "dataManagement/DataManager.h"
#include "navigation/Navigator.h"
#include "notification/NotificationManager.h"
#include "notification/Notification_DataUpdate.h"

using namespace std::chrono_literals;



DataManagement::UpdateNotifier::UpdateNotifier(QObject* parent) :
    QObject(parent)
{
    connect(GlobalObject::dataManager()->mapsAndData(), &DataManagement::Downloadable_Abstract::updateSizeChanged, this, &DataManagement::UpdateNotifier::updateNotification);
    connect(&notificationTimer, &QTimer::timeout, this, &DataManagement::UpdateNotifier::updateNotification);

    notificationTimer.setInterval(11min);
    notificationTimer.setSingleShot(true);

    updateNotification();
}


void DataManagement::UpdateNotifier::updateNotification()
{
    // If there is no update, then we end here.
    if (GlobalObject::dataManager()->mapsAndData()->updateSize() == 0) {
        return;
    }

    // Do not notify when in flight, but ask again in 11min
    if (GlobalObject::navigator()->flightStatus() == Navigation::Navigator::Flight) {
        notificationTimer.start();
        return;
    }

#warning
    // Check if last notification is less than four hours ago. In that case, do not notify again,
    // and ask again in 11min.
    QSettings settings;
    /*
    auto lastGeoMapUpdateNotification = settings.value(QStringLiteral("lastGeoMapUpdateNotification")).toDateTime();
    if (lastGeoMapUpdateNotification.isValid()) {
        auto secsSinceLastNotification = lastGeoMapUpdateNotification.secsTo(QDateTime::currentDateTimeUtc());
        if (secsSinceLastNotification < static_cast<qint64>(4*60*60)) {
            notificationTimer.start();
            return;
        }
    }
*/

    // Notify!
    auto* notification = new Notifications::Notification_DataUpdateAvailable(this);
    GlobalObject::notificationManager()->add(notification);
    settings.setValue(QStringLiteral("lastGeoMapUpdateNotification"), QDateTime::currentDateTimeUtc());
}
