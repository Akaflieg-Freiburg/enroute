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

#include "traffic/TrafficDataSource_BluetoothLowEnergy.h"

Traffic::TrafficDataSource_BluetoothLowEnergy::TrafficDataSource_BluetoothLowEnergy(bool isCanonical, const QBluetoothDeviceInfo& info, QObject* parent) :
    TrafficDataSource_AbstractSocket(isCanonical, parent),
    m_info(info),
    m_control(QLowEnergyController::createCentral(info, this))
{
#warning
    qWarning() << "New data source" << info.name();

    connect(m_control,
            &QLowEnergyController::serviceDiscovered,
            this,
            &Traffic::TrafficDataSource_BluetoothLowEnergy::onServiceDiscovered);
    connect(m_control, &QLowEnergyController::stateChanged, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onStateChanged);
    connect(m_control, &QLowEnergyController::errorOccurred, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onErrorOccurred);
    connect(m_control, &QLowEnergyController::connected, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onConnected);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onDiscoveryFinished);
    /*
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        setError("LowEnergy controller disconnected");
        setIcon(IconError);
    });
    */

    m_control->discoverServices();
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onConnected()
{
    qWarning() << "Traffic::TrafficDataSource_BTLE::onConnected" << m_info.name();
    m_control->discoverServices();
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onServiceDiscovered(const QBluetoothUuid& newService)
{
    qWarning() << "Traffic::TrafficDataSource_BTLE::onServiceDiscovered" << m_info.name() << newService;

    // 6e400001-b5a3-f393-e0a9-e50e24dcca9e

    /*
     * The Bluetooth Service ID 6e400001-b5a3-f393-e0a9-e50e24dcca9e corresponds to the Nordic UART Service (NUS). This is a custom service developed by Nordic Semiconductor for Bluetooth Low Energy (BLE) devices18.

Key features of the Nordic UART Service:

    It acts as a bridge between BLE and UART (Universal Asynchronous Receiver/Transmitter) interfaces.

    It allows for bidirectional communication between devices using a simple serial protocol over BLE.

    The service typically includes two main characteristics:

        TX Characteristic (6e400002-b5a3-f393-e0a9-e50e24dcca9e): Used for transmitting data from the peripheral to the central device1.

        RX Characteristic (6e400003-b5a3-f393-e0a9-e50e24dcca9e): Used for receiving data on the peripheral from the
     */
}


void Traffic::TrafficDataSource_BluetoothLowEnergy::onDiscoveryFinished()
{
    qWarning() << "Traffic::TrafficDataSource_BTLE::onDiscoveryFinished" << m_info.name();

    // Look for the FLARM service among discovered services
    const QBluetoothUuid flarmServiceUuid("6e400001-b5a3-f393-e0a9-e50e24dcca9e");
    if (m_control->services().contains(flarmServiceUuid))
    {
        m_flarmService = m_control->createServiceObject(flarmServiceUuid, this);
        if (m_flarmService != nullptr)
        {
            qWarning() << "FLARM Service created";
            connect(m_flarmService, &QLowEnergyService::stateChanged, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onServiceStateChanged);
            connect(m_flarmService, &QLowEnergyService::characteristicChanged, this, &Traffic::TrafficDataSource_BluetoothLowEnergy::onCharChanged);
            m_flarmService->discoverDetails();            
            setConnectivityStatus( tr("UART service found. Requestiong service characteristics.") );
        }
    }
    else
    {
        setErrorString( tr("No UART service found.") );
    }
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onServiceStateChanged(QLowEnergyService::ServiceState newState)
{
    if (newState == QLowEnergyService::RemoteServiceDiscovered)
    {
        auto charact = m_flarmService->characteristic(QBluetoothUuid("6e400003-b5a3-f393-e0a9-e50e24dcca9e"));
        if (charact.isValid())
        {
            auto m_notificationDesc = charact.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            if (m_notificationDesc.isValid())
            {
                // Enable Notifications
                m_flarmService->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
                setConnectivityStatus(tr("Notification enabled"));
            }
            else
            {
                setErrorString(tr("Cannot open descriptor."));
            }
        }
        else
        {
            setErrorString(tr("No TX characteristc"));
        }
/*
        for (auto charact : m_flarmService->characteristics())
        {
            // https://docs.ruuvi.com/communication/bluetooth-connection/nordic-uart-service-nus
            qWarning() << "BTLE Char Discovered" << charact.name() << charact.uuid();
            auto m_notificationDesc = charact.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);
            if (m_notificationDesc.isValid())
            {
                qWarning() << "Descriptor valid, enable notifications";
                // Enable Notifications
                m_flarmService->writeDescriptor(m_notificationDesc, QByteArray::fromHex("0100"));
            }
        }
*/
    }
}

void Traffic::TrafficDataSource_BluetoothLowEnergy::onCharChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue)
{
    qWarning() << "BTLE Char updated" << characteristic.name() << QString(newValue);
    processFLARMData(QString(newValue));
}


void Traffic::TrafficDataSource_BluetoothLowEnergy::connectToTrafficReceiver()
{
    qWarning() << "Traffic::TrafficDataSource_BTLE::connectToTrafficReceiver()";
    m_control->connectToDevice();
}


void Traffic::TrafficDataSource_BluetoothLowEnergy::disconnectFromTrafficReceiver()
{
    qWarning() << "Traffic::TrafficDataSource_BTLE::disconnectFromTrafficReceiver()";
    m_control->disconnectFromDevice();
    setErrorString();
}


void Traffic::TrafficDataSource_BluetoothLowEnergy::onErrorOccurred(QLowEnergyController::Error error)
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


void Traffic::TrafficDataSource_BluetoothLowEnergy::onStateChanged(QLowEnergyController::ControllerState state)
{
    // Compute new status
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


void Traffic::TrafficDataSource_BluetoothLowEnergy::onReadyRead()
{
}


QString Traffic::TrafficDataSource_BluetoothLowEnergy::sourceName() const
{
    auto name = m_info.name();
    if (name.isEmpty())
    {
        name = tr("Unnamed Device");
    }
    return tr("Bluetooth LE connection to %1").arg(name);
}
