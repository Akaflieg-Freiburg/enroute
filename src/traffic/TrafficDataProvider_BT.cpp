/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

#include "GlobalObject.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_BTClassic.h"
#include "traffic/TrafficDataSource_BTLE.h"

using namespace std::chrono_literals;


// Member functions

QString Traffic::TrafficDataProvider::addDataSource(const Traffic::ConnectionInfo &deviceInfo)
{
    if (hasSource(deviceInfo.bluetoothDeviceInfo()))
    {
        return tr("Device Already Added");
    }

    auto* source = new TrafficDataSource_BTClassic(deviceInfo.bluetoothDeviceInfo(), this);
    source->connectToTrafficReceiver();
    addDataSource(source);
#warning
    return u"not implemented"_qs;
}


bool Traffic::TrafficDataProvider::hasSource(const QBluetoothDeviceInfo& device)
{
    // Ignore new device if data source already exists.
    foreach(auto _dataSource, m_dataSources)
    {
        auto* dataSourceBTClassic = qobject_cast<TrafficDataSource_BTClassic*>(_dataSource);
        if (dataSourceBTClassic != nullptr)
        {
            if (device.address() == dataSourceBTClassic->sourceInfo().address())
            {
                return true;
            }
        }
        auto* dataSourceBTLE = qobject_cast<TrafficDataSource_BTLE*>(_dataSource);
        if (dataSourceBTLE != nullptr)
        {
            if (device.address() == dataSourceBTLE->sourceInfo().address())
            {
                return true;
            }
        }
    }
    return false;
}


void Traffic::TrafficDataProvider::onBTDeviceDiscovered(const QBluetoothDeviceInfo& info)
{
    // Ignore new device if data source already exists.
    foreach(auto _dataSource, m_dataSources)
    {
        auto* dataSourceBTClassic = qobject_cast<TrafficDataSource_BTClassic*>(_dataSource);
        if (dataSourceBTClassic != nullptr)
        {
            if (info.address() == dataSourceBTClassic->sourceInfo().address())
            {
                return;
            }
        }
        auto* dataSourceBTLE = qobject_cast<TrafficDataSource_BTLE*>(_dataSource);
        if (dataSourceBTLE != nullptr)
        {
            if (info.address() == dataSourceBTLE->sourceInfo().address())
            {
                return;
            }
        }
    }

/*
    // Check if we have a BT LE device
    if (info.coreConfigurations() & QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
    {
//        auto* source = new TrafficDataSource_BTLE(info, this);
//        source->connectToTrafficReceiver();
//        addDataSource(source);
        return;
    }
*/

    // Check if we have a BT classic device offering serial port service.
    if (info.serviceUuids().contains(QBluetoothUuid::ServiceClassUuid::SerialPort))
    {
        auto* source = new TrafficDataSource_BTClassic(info, this);
        source->connectToTrafficReceiver();
        addDataSource(source);
        return;
    }

}

