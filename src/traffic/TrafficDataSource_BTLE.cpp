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

#include "traffic/TrafficDataSource_BTLE.h"

Traffic::TrafficDataSource_BTLE::TrafficDataSource_BTLE(const QBluetoothDeviceInfo& info, QObject* parent)
    : TrafficDataSource_AbstractSocket(parent),
    m_info(info),
    m_control(QLowEnergyController::createCentral(info, this))
{
#warning
    qWarning() << "New data source" << info.name();

    connect(m_control,
            &QLowEnergyController::serviceDiscovered,
            this,
            &Traffic::TrafficDataSource_BTLE::onServiceDiscovered);
    connect(m_control, &QLowEnergyController::stateChanged, this, &Traffic::TrafficDataSource_BTLE::onStateChanged);
    connect(m_control, &QLowEnergyController::errorOccurred, this, &Traffic::TrafficDataSource_BTLE::onErrorOccurred);
    connect(m_control, &QLowEnergyController::connected, this, &Traffic::TrafficDataSource_BTLE::onConnected);
    /*
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &DeviceHandler::serviceScanDone);
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        setError("LowEnergy controller disconnected");
        setIcon(IconError);
    });
    */
}

void Traffic::TrafficDataSource_BTLE::onConnected()
{
    qWarning() << "Traffic::TrafficDataSource_BTLE::onConnected" << m_info.name();
    m_control->discoverServices();
}

void Traffic::TrafficDataSource_BTLE::onServiceDiscovered(const QBluetoothUuid& newService)
{
    qWarning() << "Traffic::TrafficDataSource_BTLE::onServiceDiscovered" << m_info.name() << newService;
}


void Traffic::TrafficDataSource_BTLE::connectToTrafficReceiver()
{
    qWarning() << "Traffic::TrafficDataSource_BTLE::connectToTrafficReceiver()";
    m_control->connectToDevice();
}


void Traffic::TrafficDataSource_BTLE::disconnectFromTrafficReceiver()
{
    qWarning() << "Traffic::TrafficDataSource_BTLE::disconnectFromTrafficReceiver()";
}


void Traffic::TrafficDataSource_BTLE::onErrorOccurred(QLowEnergyController::Error error)
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


void Traffic::TrafficDataSource_BTLE::onStateChanged(QLowEnergyController::ControllerState state)
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


void Traffic::TrafficDataSource_BTLE::onReadyRead()
{
}


QString Traffic::TrafficDataSource_BTLE::sourceName() const
{
    auto name = m_info.name();
    if (name.isEmpty())
    {
        name = tr("Unnamed Device");
    }
    return tr("Bluetooth LE connection to %1 (not implemented yet, please ignore)").arg(name);
}
