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


Traffic::BT::BT(QObject* parent)
    : TrafficDataSource_AbstractSocket(parent)
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
    auto* discoveryAgent = new QBluetoothDeviceDiscoveryAgent(localDevice.address(), this);
    connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)), this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));
    discoveryAgent->start();

    auto* m_discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
    connect(m_discoveryAgent, &QBluetoothServiceDiscoveryAgent::serviceDiscovered, this, &Traffic::BT::serviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothServiceDiscoveryAgent::finished, this, &Traffic::BT::discoveryFinished);
    connect(m_discoveryAgent, &QBluetoothServiceDiscoveryAgent::canceled, this, &Traffic::BT::discoveryFinished);
    m_discoveryAgent->start(QBluetoothServiceDiscoveryAgent::FullDiscovery);

    connect(&socket, &QBluetoothSocket::connected, this, &Traffic::BT::onConnected);
    connect(&socket, &QBluetoothSocket::disconnected, this, &Traffic::BT::onDisconnected);
    connect(&socket, &QBluetoothSocket::errorOccurred, this, &Traffic::BT::onErrorOccurred);
    connect(&socket, &QBluetoothSocket::stateChanged, this, &Traffic::BT::onStateChanged);
    connect(&socket, &QBluetoothSocket::readyRead, this, &Traffic::BT::onReadyRead);

}

void Traffic::BT::deviceDiscovered(QBluetoothDeviceInfo device)
{
    qDebug() << "Found new device:" << device.name();
    if (device.serviceUuids().contains(QBluetoothUuid::ServiceClassUuid::SerialPort))
    {
        socket.connectToService(device.address(), QBluetoothUuid::ServiceClassUuid::SerialPort);
        qWarning() << "Serial port found";

    }

}


void Traffic::BT::onConnected()
{
    qWarning() << "onConnected";
}

void Traffic::BT::onDisconnected()
{
    qWarning() << "onConnected";
}

void Traffic::BT::onErrorOccurred(QBluetoothSocket::SocketError error)
{
    qWarning() << "onError" << error;
}

void Traffic::BT::onStateChanged(QBluetoothSocket::SocketState state)
{
    qWarning() << "onState" << state;
}

void Traffic::BT::serviceDiscovered(const QBluetoothServiceInfo &info)
{

    if (info.serviceClassUuids().contains(QBluetoothUuid::ServiceClassUuid::SerialPort))
    {
//        socket.connectToService(info);
        qWarning() << "Serial port service found";

    }
}

void Traffic::BT::discoveryFinished()
{
    qDebug() << "Traffic::BT::servicediscoveryFinished()";
}

void Traffic::BT::onReadyRead()
{
    qDebug() << "Traffic::BT::onReadyRead()";


    QString sentence;
    while( m_textStream.readLineInto(&sentence) ) {
        qWarning() << sentence;
        processFLARMSentence(sentence);
    }
}


