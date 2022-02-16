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

#include "platform/Notifier.h"
#include "UpdateNotifier.h"


DataManagement::UpdateNotifier::UpdateNotifier(DataManager* parent) :
    QObject(parent)
{

    connect(GlobalObject::dataManager()->geoMaps(), &DataManagement::DownloadableGroup::updatableChanged,
            this, &DataManagement::UpdateNotifier::updateNotification);
    updateNotification();

}


void DataManagement::UpdateNotifier::updateNotification()
{

    auto* geoMaps = GlobalObject::dataManager()->geoMaps();
    if (geoMaps == nullptr) {
        GlobalObject::notifier()->hideNotification(Platform::Notifier::GeoMapUpdatePending);
        return;
    }

    if (!geoMaps->updatable()) {
        GlobalObject::notifier()->hideNotification(Platform::Notifier::GeoMapUpdatePending);
        return;
    }

    auto text = tr("The estimated download size is %1.").arg(geoMaps->updateSize());
    GlobalObject::notifier()->showNotification(Platform::Notifier::GeoMapUpdatePending, text, text);

}
