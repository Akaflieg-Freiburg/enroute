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

#include "Global.h"
#include "MobileAdaptor.h"
#include "traffic/TrafficDataSource_AbstractSocket.h"


// Member functions

Traffic::TrafficDataSource_AbstractSocket::TrafficDataSource_AbstractSocket(QObject *parent) :
    Traffic::TrafficDataSource_Abstract(parent) {

    // Connect WiFi locker/unlocker
    connect(this, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataSource_AbstractSocket::onReceivingHeartbeatChanged);

}


void Traffic::TrafficDataSource_AbstractSocket::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
    case QAbstractSocket::ConnectionRefusedError:
        setErrorString( tr("The connection was refused by the peer (or timed out).") );
        break;
    case QAbstractSocket::RemoteHostClosedError:
        setErrorString( tr("The remote host closed the connection.") );
        break;
    case QAbstractSocket::HostNotFoundError:
        setErrorString( tr("The host address was not found.") );
        break;
    case QAbstractSocket::SocketAccessError:
        setErrorString( tr("The socket operation failed because the application lacked the required privileges.") );
        break;
    case QAbstractSocket::SocketResourceError:
        setErrorString( tr("The local system ran out of resources.") );
        break;
    case QAbstractSocket::SocketTimeoutError:
        setErrorString( tr("The socket operation timed out.") );
        break;
    case QAbstractSocket::DatagramTooLargeError:
        setErrorString( tr("The datagram was larger than the operating system's limit.") );
        break;
    case QAbstractSocket::NetworkError:
        setErrorString( tr("An error occurred with the network.") );
        break;
    case QAbstractSocket::AddressInUseError:
        setErrorString( tr("The address specified to QAbstractSocket::bind() is already in use and was set to be exclusive.") );
        break;
    case QAbstractSocket::SocketAddressNotAvailableError:
        setErrorString( tr("The address specified to QAbstractSocket::bind() does not belong to the host.") );
        break;
    case QAbstractSocket::UnsupportedSocketOperationError:
        setErrorString( tr("The requested socket operation is not supported by the local operating system.") );
        break;
    case QAbstractSocket::ProxyAuthenticationRequiredError:
        setErrorString( tr("The socket is using a proxy, and the proxy requires authentication.") );
        break;
    case QAbstractSocket::SslHandshakeFailedError:
        setErrorString( tr("The SSL/TLS handshake failed, so the connection was closed.") );
        break;
    case QAbstractSocket::UnfinishedSocketOperationError:
        setErrorString( tr("The last operation attempted has not finished yet (still in progress in the background).") );
        break;
    case QAbstractSocket::ProxyConnectionRefusedError:
        setErrorString( tr("Could not contact the proxy server because the connection to that server was denied.") );
        break;
    case QAbstractSocket::ProxyConnectionClosedError:
        setErrorString( tr("The connection to the proxy server was closed unexpectedly (before the connection to the final peer was established).") );
        break;
    case QAbstractSocket::ProxyConnectionTimeoutError:
        setErrorString( tr("The connection to the proxy server timed out or the proxy server stopped responding in the authentication phase.") );
        break;
    case QAbstractSocket::ProxyNotFoundError:
        setErrorString( tr("The proxy address set with setProxy() (or the application proxy) was not found.") );
        break;
    case QAbstractSocket::ProxyProtocolError:
        setErrorString( tr("The connection negotiation with the proxy server failed, because the response from the proxy server could not be understood.") );
        break;
    case QAbstractSocket::OperationError:
        setErrorString( tr("An operation was attempted while the socket was in a state that did not permit it.") );
        break;
    case QAbstractSocket::SslInternalError:
        setErrorString( tr("The SSL library being used reported an internal error. This is probably the result of a bad installation or misconfiguration of the library.") );
        break;
    case QAbstractSocket::SslInvalidUserDataError:
        setErrorString( tr("Invalid data (certificate, key, cypher, etc.) was provided and its use resulted in an error in the SSL library.") );
        break;
    case QAbstractSocket::TemporaryError:
        setErrorString( tr("A temporary error occurred (e.g., operation would block and socket is non-blocking).") );
        break;
    case QAbstractSocket::UnknownSocketError:
        setErrorString( tr("An unidentified error occurred.") );
        break;
    }

}


void Traffic::TrafficDataSource_AbstractSocket::onReceivingHeartbeatChanged(bool receivingHB)
{
    // Acquire or release WiFi lock as appropriate
    auto* mobileAdaptor = Global::mobileAdaptor();
    if (mobileAdaptor != nullptr) {
        MobileAdaptor::lockWifi(receivingHB);
    }

}


void Traffic::TrafficDataSource_AbstractSocket::onStateChanged(QAbstractSocket::SocketState socketState)
{

    // Compute new status
    switch( socketState ) {
    case QAbstractSocket::HostLookupState:
        setConnectivityStatus( tr("Performing host name lookup.") );
        break;
    case QAbstractSocket::ConnectingState:
        setConnectivityStatus( tr("Trying to establish a connection.") );
        break;
    case QAbstractSocket::ConnectedState:
        setConnectivityStatus( tr("Connected.") );
        break;
    case QAbstractSocket::BoundState:
        setConnectivityStatus( tr("Bound to an address and port, but not connected yet.") );
        break;
    case QAbstractSocket::ClosingState:
        setConnectivityStatus( tr("Closing.") );
        break;
    default:
        setConnectivityStatus( tr("Not connected.") );
        break;
    }

}

