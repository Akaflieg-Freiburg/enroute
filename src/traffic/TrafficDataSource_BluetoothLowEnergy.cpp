/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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

#include "traffic/TrafficDataSource_BluetoothLowEnergy.h"

Traffic::TrafficDataSource_BluetoothLowEnergy::TrafficDataSource_BluetoothLowEnergy(bool isCanonical, const QBluetoothDeviceInfo& info, QObject* parent) :
    TrafficDataSource_AbstractSocket(isCanonical, parent),
    m_info(info),
    m_control(QLowEnergyController::createCentral(info, this)),
    m_discoveryAgent(new QBluetoothDeviceDiscoveryAgent(this))
{
    m_connectionInfo = Traffic::ConnectionInfo(m_info, canonical());
    m_bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);

    // Initialize the discovery agent
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(5000); // 5-second timeout

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onDeviceDiscovered);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onDiscoveryFinished);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onDiscoveryError);

    // Initial setup of the controller
    setupController(m_info);
}


//
// Getter Methods
//

QString Traffic::TrafficDataSource_BluetoothLowEnergy::sourceName() const
{
    auto name = m_info.name();
    if (name.isEmpty())
    {
        name = tr("Unnamed Device");
    }
    return tr("Bluetooth LE connection to %1").arg(name);
}


//
// Public Slots
//

void Traffic::TrafficDataSource_BluetoothLowEnergy::connectToTrafficReceiver()
{
    // Do not do anything if the traffic receiver is connected and is receiving.
    if (receivingHeartbeat())
    {
        return;
    }
    // Cannot connect unless we are unconnected
    if (m_control->state() != QLowEnergyController::UnconnectedState)
    {
        return;
    }

    // Check if we have sufficient permissions
    switch (qApp->checkPermission(m_bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(m_bluetoothPermission, this, [this]() { connectToTrafficReceiver(); });
        return;
    case Qt::PermissionStatus::Denied:
        setErrorString( tr("Necessary permission have been denied.") );
        return;
    case Qt::PermissionStatus::Granted:
        break;
    }

    setErrorString();
    setConnectivityStatus(tr("Searching for device."));
    if (m_discoveryAgent->isActive())
    {
        m_discoveryAgent->stop();
    }
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::disconnectFromTrafficReceiver()
{
    delete m_nordicUARTService;
    m_nordicUARTService = nullptr;
    delete m_simpleUARTService;
    m_simpleUARTService = nullptr;

    m_control->disconnectFromDevice();
    setErrorString();
}


//
// Private Slots
//

void Traffic::TrafficDataSource_BluetoothLowEnergy::onCharacteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    if ((characteristic.uuid() == nordicUARTTxCharacteristicID) || (characteristic.uuid() == simpleUARTCharacteristicID))
    {
        emit dataReceived(QString(newValue));
        processFLARMData(QString(newValue));
        return;
    }
    setErrorString( tr("Received data from unknown characteristic %1.").arg(characteristic.name()) );
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onControlerError(QLowEnergyController::Error error)
{
    switch (error)
    {
    case QLowEnergyController::NoError:
        break;
    case QLowEnergyController::UnknownError:
        setErrorString( tr("An unknown error has occurred.") );
        break;
    case QLowEnergyController::UnknownRemoteDeviceError:
        setErrorString( tr("The remote Bluetooth Low Energy device with the address passed to the constructor of this class cannot be found.") );
        break;
    case QLowEnergyController::NetworkError:
        setErrorString( tr("The attempt to read from or write to the remote device failed.") );
        break;
    case QLowEnergyController::InvalidBluetoothAdapterError:
        setErrorString( tr("The local Bluetooth device with the address passed to the constructor of this class cannot be found or there is no local Bluetooth device.") );
        break;
    case QLowEnergyController::ConnectionError:
        setErrorString( tr("The attempt to connect to the remote device failed.") );
        break;
    case QLowEnergyController::AdvertisingError:
        setErrorString( tr("The attempt to start advertising failed.") );
        break;
    case QLowEnergyController::RemoteHostClosedError:
        setErrorString( tr("The remote device closed the connection.") );
        break;
    case QLowEnergyController::AuthorizationError:
        setErrorString( tr("The local Bluetooth device closed the connection due to insufficient authorization.") );
        break;
    case QLowEnergyController::MissingPermissionsError:
        setErrorString( tr("The operating system requests permissions which were not granted by the user.") );
        break;
    case QLowEnergyController::RssiReadError:
        setErrorString( tr("An attempt to read RSSI (received signal strength indicator) of a remote device finished with error.") );
        break;
    }
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onDiscoveryError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    switch (error) {
    case QBluetoothDeviceDiscoveryAgent::PoweredOffError:
        setErrorString(tr("Bluetooth is turned off. Please enable it in System Settings."));
        break;
    case QBluetoothDeviceDiscoveryAgent::InputOutputError:
        setErrorString(tr("A hardware error occurred while searching for devices."));
        break;
    default:
        setErrorString(tr("An error occurred during device discovery (%1).").arg(error));
        break;
    }
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onServiceDiscoveryFinished()
{
    if (m_control->services().contains(nordicUARTServiceUuid))
    {
        m_nordicUARTService = m_control->createServiceObject(nordicUARTServiceUuid, this);
        if (m_nordicUARTService != nullptr)
        {
            setConnectivityStatus( tr("Nordic UART Service found. Requesting service characteristics.") );
            connect(m_nordicUARTService, &QLowEnergyService::stateChanged, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onServiceStateChanged);
            connect(m_nordicUARTService, &QLowEnergyService::characteristicChanged, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onCharacteristicChanged);
            m_nordicUARTService->discoverDetails();
            return;
        }
    }
    if (m_control->services().contains(simpleUARTServiceUuid))
    {
        m_simpleUARTService = m_control->createServiceObject(simpleUARTServiceUuid, this);
        if (m_simpleUARTService != nullptr)
        {
            setConnectivityStatus( tr("Simple UART Service found. Requesting service characteristics.") );
            connect(m_simpleUARTService, &QLowEnergyService::stateChanged, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onServiceStateChanged);
            connect(m_simpleUARTService, &QLowEnergyService::characteristicChanged, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onCharacteristicChanged);
            m_simpleUARTService->discoverDetails();
            return;
        }
    }
    setErrorString( tr("No UART service found.") );
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onServiceStateChanged(QLowEnergyService::ServiceState newState)
{
    switch(newState)
    {
    case QLowEnergyService::InvalidService:
        setErrorString(tr("Invalid Service."));
        return;
    case QLowEnergyService::RemoteService:
        setConnectivityStatus(tr("Service details unknown."));
        return;
    case QLowEnergyService::RemoteServiceDiscovering:
        setConnectivityStatus(tr("Requesting service details."));
        return;
    case QLowEnergyService::RemoteServiceDiscovered:
        break;
    case QLowEnergyService::LocalService:
        return;
    }

    if (m_nordicUARTService != nullptr)
    {
        auto txCharacteristic = m_nordicUARTService->characteristic(nordicUARTTxCharacteristicID);
        if (!txCharacteristic.isValid())
        {
            setErrorString(tr("The Nordic UART Service does not contain the TX characteristic."));
            return;
        }
        auto m_notificationDescriptor = txCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
        if (!m_notificationDescriptor.isValid())
        {
            setErrorString(tr("Cannot open the client characteristic configuration descriptor."));
            return;
        }
        m_nordicUARTService->writeDescriptor(m_notificationDescriptor, QByteArray::fromHex("0100"));
        setConnectivityStatus(tr("Data transfer enabled."));
        return;
    }
    if (m_simpleUARTService != nullptr)
    {
        auto txCharacteristic = m_simpleUARTService->characteristic(simpleUARTCharacteristicID);
        if (!txCharacteristic.isValid())
        {
            setErrorString(tr("The Simple UART Service does not contain the TX characteristic."));
            return;
        }
        auto m_notificationDescriptor = txCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
        if (!m_notificationDescriptor.isValid())
        {
            setErrorString(tr("Cannot open the client characteristic configuration descriptor."));
            return;
        }
        m_simpleUARTService->writeDescriptor(m_notificationDescriptor, QByteArray::fromHex("0100"));
        setConnectivityStatus(tr("Data transfer enabled."));
        return;
    }

    setErrorString(tr("No UART Service available."));
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onStateChanged(QLowEnergyController::ControllerState state)
{
    switch(state)
    {
    case QLowEnergyController::UnconnectedState:
        setConnectivityStatus( tr("The controller is not connected to a remote device.") );
        break;
    case QLowEnergyController::ConnectingState:
        setConnectivityStatus( tr("The controller is attempting to connect to a remote device.") );
        break;
    case QLowEnergyController::ConnectedState:
        setConnectivityStatus( tr("The controller is connected to a remote device.") );
        break;
    case QLowEnergyController::DiscoveringState:
        setConnectivityStatus( tr("The controller is retrieving the list of services offered by the remote device.") );
        break;
    case QLowEnergyController::DiscoveredState:
        setConnectivityStatus( tr("The controller has discovered all services offered by the remote device.") );
        break;
    case QLowEnergyController::ClosingState:
        setConnectivityStatus( tr("The controller is about to be disconnected from the remote device.") );
        break;
    case QLowEnergyController::AdvertisingState:
        setConnectivityStatus( tr("The controller is currently advertising data.") );
        break;
    }
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onDeviceDiscovered(const QBluetoothDeviceInfo &info)
{
    // Compare by name. If you have multiple devices with the same name,
    // you might need a more specific filter (like Service UUIDs).
    if (info.name() == m_info.name()) {
        m_discoveryAgent->stop();

        // Update our stored info with the fresh UUID/handle provided by the OS
        m_info = info;
        setupController(m_info);

        setConnectivityStatus(tr("Device found. Connecting..."));
        m_control->connectToDevice();
    }
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onDiscoveryFinished()
{
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        setErrorString(tr("Unable to find device within range."));
        onStateChanged(m_control->state());
    }
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::setupController(const QBluetoothDeviceInfo &info)
{
    // Clean up old controller if it exists
    if (m_control) {
        m_control->disconnect();
        m_control->deleteLater();
    }

    m_control = QLowEnergyController::createCentral(info, this);

    connect(m_control, &QLowEnergyController::connected, m_control, &QLowEnergyController::discoverServices);
    connect(m_control, &QLowEnergyController::errorOccurred, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onControlerError);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onServiceDiscoveryFinished);
    connect(m_control, &QLowEnergyController::stateChanged, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onStateChanged);
}
