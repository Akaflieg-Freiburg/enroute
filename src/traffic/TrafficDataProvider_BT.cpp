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

using namespace std::chrono_literals;


// Member functions


QString Traffic::TrafficDataProvider::BTStatus()
{
#if defined(Q_OS_IOS)
    return "<p>" + tr("Bluetooth INOP: Classic BT is not supported on iOS.") + "<p>";
#endif

    if (!localBTDevice.isValid())
    {
        return "<p>" + tr("Bluetooth INOP: No valid device or insufficient permissions.") + "<p>";
    }

    switch (qApp->checkPermission(m_bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        return "<p>" + tr("Bluetooth INOP: Waiting for permission.") + "<p>";
    case Qt::PermissionStatus::Denied:
        return "<p>" + tr("Bluetooth INOP: No permission.") + "<p>";
    case Qt::PermissionStatus::Granted:
        break;
    }

    if (localBTDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff)
    {
        return "<p>" + tr("Bluetooth INOP: Powered off.") + "<p>";
    }

    if (discoveryAgent.error() != QBluetoothDeviceDiscoveryAgent::NoError)
    {
        return "<p>" + tr("Bluetooth INOP: %1").arg(discoveryAgent.errorString()) + "<p>";
    }

    if (discoveryAgent.isActive())
    {
        return "<p>" + tr("Bluetooth: Scanning for classic BT devices offering serial port service.") + "<p>";
    }

    return "<p>" + tr("Bluetooth: Operational.") + "<p>";
}


void Traffic::TrafficDataProvider::deviceDiscovered(const QBluetoothDeviceInfo& info)
{
    qDebug() << "Found new device:" << info.name();
    foreach(auto _dataSource, m_dataSources)
    {
        auto* dataSource = qobject_cast<TrafficDataSource_BTClassic*>(_dataSource);
        if (dataSource == nullptr)
        {
            continue;
        }
        if (info.address() == dataSource->sourceInfo().address())
        {
            return;
        }
    }

    if (info.serviceUuids().contains(QBluetoothUuid::ServiceClassUuid::SerialPort))
    {
        qWarning() << "Serial port device found";
        auto* source = new TrafficDataSource_BTClassic(info, this);
        source->connectToTrafficReceiver();
        addDataSource(source);
    }

}
