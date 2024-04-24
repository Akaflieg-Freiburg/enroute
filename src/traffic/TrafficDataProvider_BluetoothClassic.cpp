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

#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_BluetoothClassic.h"

QString Traffic::TrafficDataProvider::addDataSource_BluetoothClassic(const Traffic::ConnectionInfo &connectionInfo)
{
#if defined(Q_OS_IOS)
    return tr("Due to platform limitation, connections to Bluetooth Classic devices are not supported in iOS.");
#endif

    // Sanity check
    auto bluetoothDeviceInfo = connectionInfo.bluetoothDeviceInfo();
    if (!bluetoothDeviceInfo.isValid())
    {
        return tr("Invalid connection.");
    }

    // Ignore new device if data source already exists.
    foreach(auto _dataSource, m_dataSources)
    {
        auto* dataSourceBTClassic = qobject_cast<TrafficDataSource_BluetoothClassic*>(_dataSource);
        if (dataSourceBTClassic != nullptr)
        {
            if (connectionInfo.bluetoothDeviceInfo().address() == dataSourceBTClassic->sourceInfo().address())
            {
                return tr("A connection to this device already exists.");
            }
        }
    }

    auto* source = new TrafficDataSource_BluetoothClassic(connectionInfo.bluetoothDeviceInfo(), this);
    source->connectToTrafficReceiver();
    addDataSource(source);
    return {};
}
