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

#include "dataManagement/DataManager.h"
#include "navigation/Navigator.h"
#include "notification/Notification_DataUpdateAvailable.h"



//
// Constructors and destructors
//

Notifications::Notification_DataUpdateAvailable::Notification_DataUpdateAvailable(QObject* parent)
    : Notification(tr("Map and data updates available"), Notifications::Notification::Info, parent)
{
    setButton1Text(tr("Update"));
    setButton2Text(tr("Dismiss"));
    setTextBodyAction(OpenMapsAndDataPage);
    update();

    auto* mapsAndData = GlobalObject::dataManager()->mapsAndData();
    connect(mapsAndData, &DataManagement::Downloadable_MultiFile::updateSizeChanged, this, &Notifications::Notification_DataUpdateAvailable::update);
    connect(mapsAndData, &DataManagement::Downloadable_MultiFile::downloadingChanged, this, &Notifications::Notification_DataUpdateAvailable::update);
    connect(GlobalObject::navigator(), &Navigation::Navigator::flightStatusChanged, this, &Notifications::Notification_DataUpdateAvailable::update);
}



//
// Methods
//

void Notifications::Notification_DataUpdateAvailable::onButton1Clicked()
{
    GlobalObject::dataManager()->mapsAndData()->update();
    deleteLater();
}

void Notifications::Notification_DataUpdateAvailable::update()
{
    auto* mapsAndData = GlobalObject::dataManager()->mapsAndData();
    if (mapsAndData == nullptr)
    {
        return;
    }
    if (mapsAndData->updateSize() == Units::ByteSize(0))
    {
        deleteLater();
        return;
    }
    if (mapsAndData->downloading())
    {
        deleteLater();
        return;
    }
    if (GlobalObject::navigator()->flightStatus() == Navigation::Navigator::Flight) {
        deleteLater();
        return;
    }

    setText(tr("Estimated download size: %1").arg(GlobalObject::dataManager()->mapsAndData()->updateSizeString()));
}
