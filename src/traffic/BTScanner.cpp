/***************************************************************************
 *   Copyright (C) 2021-2024 by Stefan Kebekus                             *
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

#include "traffic/BTScanner.h"


// Member functions

Traffic::BTScanner::BTScanner(QObject* parent)
    : QObject(parent)
{
    // Wire up Bluetooth-related members
    m_bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);

    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &Traffic::BTScanner::onErrorOccurred);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &Traffic::BTScanner::onCanceled);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &Traffic::BTScanner::onFinished);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &Traffic::BTScanner::onDeviceDiscovered);

    start();
}


void Traffic::BTScanner::onCanceled()
{
    setIsScanning(m_discoveryAgent.isActive());
}

void Traffic::BTScanner::onDeviceDiscovered(const QBluetoothDeviceInfo&info)
{
}

void Traffic::BTScanner::onErrorOccurred(QBluetoothDeviceDiscoveryAgent::Error error)
{
    setIsScanning(m_discoveryAgent.isActive());

    switch(error)
    {
    case QBluetoothDeviceDiscoveryAgent::NoError:
        setError({});
        break;
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        setError( tr("The Bluetooth adaptor is powered off. Power it on before doing discovery.") );
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        setError( tr("Read/Write error.") );
        break;
    case QBluetoothDeviceDiscoveryAgent::InvalidBluetoothAdapterError:
        setError( tr("Invalid Bluetooth adaptor.") );
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedPlatformError:
        setError( tr("Device discovery is not possible or implemented on the current platform.") );
        break;
    case QBluetoothDeviceDiscoveryAgent::UnsupportedDiscoveryMethod:
        setError( tr("One of the requested discovery methods is not supported by the current platform.") );
        break;
    case QBluetoothDeviceDiscoveryAgent::LocationServiceTurnedOffError:
        setError( tr("The location service is turned off. Usage of Bluetooth is not possible when location service is turned off.") );
        break;
    case QBluetoothDeviceDiscoveryAgent::MissingPermissionsError:
        setError( tr("The operating system requests permissions which were not granted by the user.") );
        break;
    case QBluetoothDeviceDiscoveryAgent::UnknownError:
        setError( tr("An unknown error has occurred.") );
        break;
    }
}

void Traffic::BTScanner::onFinished()
{
    setIsScanning(m_discoveryAgent.isActive());

}

void Traffic::BTScanner::setError(const QString& errorString)
{
    if (errorString == m_error)
    {
        return;
    }
    m_error = errorString;
    emit errorChanged();
}

void Traffic::BTScanner::setIsScanning(bool _isScanning)
{
    if (_isScanning == m_isScanning)
    {
        return;
    }
    m_isScanning = _isScanning;
    emit isScanningChanged();

}

void Traffic::BTScanner::start()
{
    m_discoveryAgent.start();
    setIsScanning(m_discoveryAgent.isActive());
}
