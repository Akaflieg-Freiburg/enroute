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

#include "navigation/Navigator.h"
#include "platform/Notifier.h"
#include "GlobalObject.h"
#include "UpdateNotifier.h"



DataManagement::UpdateNotifier::UpdateNotifier(DataManager* parent) :
    QObject(parent)
{

    connect(GlobalObject::dataManager()->geoMaps(), &DataManagement::DownloadableGroup::updatableChanged, this, &DataManagement::UpdateNotifier::updateNotification);
    connect(&notificationTimer, &QTimer::timeout, this, &DataManagement::UpdateNotifier::updateNotification);

    notificationTimer.setInterval(11*60*1000);
    notificationTimer.setSingleShot(true);

    updateNotification();
}


void DataManagement::UpdateNotifier::updateNotification()
{

    // Get pointer to geoMaps
    auto* geoMaps = GlobalObject::dataManager()->geoMaps();
    if (geoMaps == nullptr) {
        GlobalObject::notifier()->hideNotification(Platform::Notifier::GeoMapUpdatePending);
        notificationTimer.start();
        return;
    }

    // If there is no update, then we end here.
    if (!geoMaps->updatable()) {
        GlobalObject::notifier()->hideNotification(Platform::Notifier::GeoMapUpdatePending);
        return;
    }

    // Do not notify when in flight, but ask again in 11min
    if (GlobalObject::navigator()->flightStatus() == Navigation::Navigator::Flight) {
        GlobalObject::notifier()->hideNotification(Platform::Notifier::GeoMapUpdatePending);
        notificationTimer.start();
        return;
    }

    // Check if last notification is less than four hours ago. In that case, do not notify again,
    // and ask again in 11min.
    QSettings settings;
    auto lastGeoMapUpdateNotification = settings.value("lastGeoMapUpdateNotification").toDateTime();
    if (lastGeoMapUpdateNotification.isValid()) {
        auto secsSinceLastNotification = lastGeoMapUpdateNotification.secsTo(QDateTime::currentDateTimeUtc());
        if (secsSinceLastNotification < 4*60*60) {
            notificationTimer.start();
            return;
        }
    }

    // Notify!
    auto text = tr("The estimated download size is %1.").arg(geoMaps->updateSize());
    GlobalObject::notifier()->showNotification(Platform::Notifier::GeoMapUpdatePending, text, text);
    settings.setValue("lastGeoMapUpdateNotification", QDateTime::currentDateTimeUtc());

}
