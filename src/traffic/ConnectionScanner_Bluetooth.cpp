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

#include <QBluetoothLocalDevice>
#include <QCoreApplication>

#include "traffic/ConnectionScanner_Bluetooth.h"


// Member functions

Traffic::ConnectionScanner_Bluetooth::ConnectionScanner_Bluetooth(QObject* parent)
    : ConnectionScanner_Abstract(parent)
{
#if defined(Q_OS_IOS)
    setError( tr("Due to platform limitations, Bluetooth is not supported on iOS."));
    return;
#endif

    // Rectify Permissions
    m_bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);

    // Wire up Bluetooth-related members
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &Traffic::ConnectionScanner_Bluetooth::onCanceled);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &Traffic::ConnectionScanner_Bluetooth::onDeviceDiscovered);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this, &Traffic::ConnectionScanner_Bluetooth::onDeviceUpdated);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &Traffic::ConnectionScanner_Bluetooth::onErrorOccurred);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &Traffic::ConnectionScanner_Bluetooth::onFinished);
}


QString Traffic::ConnectionScanner_Bluetooth::bluetoothStatus()
{
    QBluetoothPermission const m_bluetoothPermission;
    switch (qApp->checkPermission(m_bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        return tr("Waiting for permissions.");
    case Qt::PermissionStatus::Denied:
        return tr("Necessary permissions have been denied.");
    case Qt::PermissionStatus::Granted:
        break;
    }

    QBluetoothLocalDevice const localBTDevice;
    if (!localBTDevice.isValid())
    {
        return tr("No Bluetooth adaptor has been found.");
    }
    if (localBTDevice.hostMode() == QBluetoothLocalDevice::HostPoweredOff)
    {
        return tr("Bluetooth is powered off.");
    }
    return {};
}

void Traffic::ConnectionScanner_Bluetooth::onCanceled()
{
    setIsScanning(m_discoveryAgent.isActive());
    updateConnectionInfos();
}

void Traffic::ConnectionScanner_Bluetooth::onDeviceDiscovered(const QBluetoothDeviceInfo& info)
{
    // Ignore devices that we cannot connect to
    ConnectionInfo const connectionInfo(info);
    if (!connectionInfo.canConnect())
    {
        return;
    }

    setIsScanning(m_discoveryAgent.isActive());
    updateConnectionInfos();
}

void Traffic::ConnectionScanner_Bluetooth::onDeviceUpdated(const QBluetoothDeviceInfo& info, QBluetoothDeviceInfo::Fields updatedFields)
{
    // Ignore if only irrelevant fields are updated
    if ((updatedFields & QBluetoothDeviceInfo::Field::ServiceData) != 0)
    {
        onDeviceDiscovered(info);
    }
}

void Traffic::ConnectionScanner_Bluetooth::onErrorOccurred(QBluetoothDeviceDiscoveryAgent::Error error)
{
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
        setError( tr("An unknown error has occurred.") + " " + bluetoothStatus() );
        break;
    }
    setIsScanning(m_discoveryAgent.isActive());
}

void Traffic::ConnectionScanner_Bluetooth::onFinished()
{
    setIsScanning(m_discoveryAgent.isActive());
    updateConnectionInfos();
}

void Traffic::ConnectionScanner_Bluetooth::start()
{
#if defined(Q_OS_IOS)
    return;
#endif

    if (m_discoveryAgent.isActive())
    {
        return;
    }

    switch (qApp->checkPermission(m_bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(m_bluetoothPermission, this, [this]() { start(); });
        return;
    case Qt::PermissionStatus::Denied:
        setError( tr("Necessary permission have been denied.") );
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }

    setError({});
    m_discoveryAgent.start();
    setIsScanning(m_discoveryAgent.isActive());
}

void Traffic::ConnectionScanner_Bluetooth::stop()
{
    if (!m_discoveryAgent.isActive())
    {
        return;
    }

    m_discoveryAgent.stop();
}

void Traffic::ConnectionScanner_Bluetooth::updateConnectionInfos()
{
    QVector<Traffic::ConnectionInfo> result;
    auto deviceInfos = m_discoveryAgent.discoveredDevices();
    foreach (auto deviceInfo, deviceInfos)
    {
        result += ConnectionInfo(deviceInfo);
    }

    std::sort(result.begin(), result.end());
    setDevices(result);
}

