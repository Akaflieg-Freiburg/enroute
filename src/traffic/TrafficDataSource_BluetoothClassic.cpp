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

#include <QApplication>

#include "traffic/TrafficDataSource_BluetoothClassic.h"


Traffic::TrafficDataSource_BluetoothClassic::TrafficDataSource_BluetoothClassic(const QBluetoothDeviceInfo& info, QObject* parent)
    : TrafficDataSource_AbstractSocket(parent), m_info(info)
{
    // Rectify Permissions
    m_bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);

    // Connect socket
    connect(&m_socket, &QBluetoothSocket::errorOccurred, this, &Traffic::TrafficDataSource_BluetoothClassic::onErrorOccurred);
    connect(&m_socket, &QBluetoothSocket::stateChanged, this, &Traffic::TrafficDataSource_BluetoothClassic::onStateChanged);
    connect(&m_socket, &QBluetoothSocket::readyRead, this, &Traffic::TrafficDataSource_BluetoothClassic::onReadyRead);
}

void Traffic::TrafficDataSource_BluetoothClassic::connectToTrafficReceiver()
{
    // Do not do anything if the traffic receiver is connected and is receiving.
    if (receivingHeartbeat())
    {
        return;
    }

#if defined(Q_OS_IOS)
    setError( tr("Due to platform limitations, Bluetooth Classic is not supported on iOS."));
    return;
#endif

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

    // Start new connection
    m_socket.abort();
    setErrorString();
    m_socket.connectToService(m_info.address(), QBluetoothUuid::ServiceClassUuid::SerialPort);

    // Update properties
    onStateChanged(m_socket.state());
}

void Traffic::TrafficDataSource_BluetoothClassic::disconnectFromTrafficReceiver()
{
    m_socket.disconnectFromService();
}

void Traffic::TrafficDataSource_BluetoothClassic::onErrorOccurred(QBluetoothSocket::SocketError error)
{
    switch (error)
    {
    case QBluetoothSocket::SocketError::UnknownSocketError:
        setErrorString( tr("An unknown error has occurred.") );
        break;
    case QBluetoothSocket::SocketError::NoSocketError:
        setErrorString( tr("No error.") );
        break;
    case QBluetoothSocket::SocketError::HostNotFoundError:
        setErrorString( tr("Could not find the remote host.") );
        break;
    case QBluetoothSocket::SocketError::ServiceNotFoundError:
        setErrorString( tr("Could not find the service UUID on remote host.") );
        break;
    case QBluetoothSocket::SocketError::NetworkError:
        setErrorString( tr("Attempt to read or write from socket returned an error") );
        break;
    case QBluetoothSocket::SocketError::UnsupportedProtocolError:
        setErrorString( tr("The Protocol is not supported on this platform.") );
        break;
    case QBluetoothSocket::SocketError::OperationError:
        setErrorString( tr("An operation was attempted while the socket was in a state that did not permit it.") );
        break;
    case QBluetoothSocket::SocketError::RemoteHostClosedError:
        setErrorString( tr("The remote host closed the connection.") );
        break;
    case QBluetoothSocket::SocketError::MissingPermissionsError:
        setErrorString( tr("The operating system requests permissions which were not granted by the user.") );
        break;
    }
}

void Traffic::TrafficDataSource_BluetoothClassic::onStateChanged(QBluetoothSocket::SocketState state)
{
    // Compute new status
    switch(state)
    {
    case QBluetoothSocket::SocketState::UnconnectedState:
        setConnectivityStatus( tr("The socket is not connected.") );
        break;
    case QBluetoothSocket::SocketState::ServiceLookupState:
        setConnectivityStatus( tr("The socket is querying connection parameters.") );
        break;
    case QBluetoothSocket::SocketState::ConnectingState:
        setConnectivityStatus( tr("The socket is attempting to connect to a device.") );
        break;
    case QBluetoothSocket::SocketState::ConnectedState:
        setConnectivityStatus( tr("The socket is connected to a device.") );
        break;
    case QBluetoothSocket::SocketState::BoundState:
        setConnectivityStatus( tr("The socket is bound to a local address and port.") );
        break;
    case QBluetoothSocket::SocketState::ClosingState:
        setConnectivityStatus( tr("The socket is connected and will be closed once all pending data is written to the socket.") );
        break;
    case QBluetoothSocket::SocketState::ListeningState:
        setConnectivityStatus( tr("The socket is listening for incoming connections.") );
        break;
    }
}

void Traffic::TrafficDataSource_BluetoothClassic::onReadyRead()
{
    QString sentence;
    while(m_textStream.readLineInto(&sentence) )
    {
#warning
        qWarning() << sentence;
        processFLARMSentence(sentence);
    }
}

QString Traffic::TrafficDataSource_BluetoothClassic::sourceName() const
{
    auto name = m_info.name();
    if (name.isEmpty())
    {
        name = tr("Unnamed Device");
    }
    return tr("Bluetooth Classic connection to %1").arg(name);
}
