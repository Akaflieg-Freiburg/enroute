/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothServiceDiscoveryAgent>
#include <QDebug>

#include "traffic/BT.h"


Traffic::BT::BT(QObject* parent) : QObject(parent)
{
    qWarning() << "BT constructor" << localDevice.isValid();

    // Turn Bluetooth on
    localDevice.powerOn();

    // Read local device name
    qWarning() << "Bluetooth device name" << localDevice.name();

    // Get connected devices
    QList<QBluetoothAddress> remotes;
    remotes = localDevice.connectedDevices();
    qWarning() << remotes;

    // Create a discovery agent and connect to its signals
    QBluetoothDeviceDiscoveryAgent *discoveryAgent = new QBluetoothDeviceDiscoveryAgent(localDevice.address(), this);
    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
            this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));

    // Start a discovery
   // discoveryAgent->start();

    auto* m_discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);

    connect(m_discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered,
            this, &BT::serviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished,
            this, &BT::discoveryFinished);
    connect(m_discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled,
            this, &BT::discoveryFinished);
    m_discoveryAgent->start();
    // m_discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);
}

void Traffic::BT::deviceDiscovered(QBluetoothDeviceInfo device)
{
    qDebug() << "Found new device:" << device.name() << '(' << device.address().toString() << ')';
}

void Traffic::BT::serviceDiscovered(const QBluetoothServiceInfo &info)
{
    qDebug() << "Traffic::BT::serviceDiscovered"
             << info.device().name()
             << info.serviceName()
             << info.serviceClassUuids();
}

void Traffic::BT::discoveryFinished()
{
    qDebug() << "Traffic::BT::servicediscoveryFinished()";
}


