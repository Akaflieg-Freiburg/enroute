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

#include <QQmlEngine>
#include <chrono>
#include <utility>

#include "MobileAdaptor.h"
#include "Navigation_SatNav.h"
#include "traffic/TcpTrafficDataSource.h"

using namespace std::chrono_literals;


// Member functions

Traffic::TcpTrafficDataSource::TcpTrafficDataSource(QString hostName, quint16 port, QObject *parent) :
    Traffic::AbstractTrafficDataSource(parent), _hostName(std::move(hostName)), _port(port) {

    // Create socket
    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::errorOccurred, this, &Traffic::TcpTrafficDataSource::onErrorOccurred);
    connect(socket, &QTcpSocket::readyRead, this, &Traffic::TcpTrafficDataSource::readFromStream);
    connect(socket, &QTcpSocket::stateChanged, this, &Traffic::TcpTrafficDataSource::onStateChanged);

    // Set up text stream
    textStream.setDevice(socket);
    textStream.setCodec("ISO 8859-1");

    //
    // Initialize properties
    //
    onStateChanged();

}


void Traffic::TcpTrafficDataSource::connectToTrafficReceiver()
{
    // Paranoid safety check
    if (socket.isNull()) {
        return;
    }

    socket->abort();
    socket->connectToHost(_hostName, _port);
    textStream.setDevice(socket);

    // Update properties
    onStateChanged();
}


void Traffic::TcpTrafficDataSource::disconnectFromTrafficReceiver()
{
    // Paranoid safety check
    if (socket.isNull()) {
        return;
    }

    // Disconnect socket.
    socket->abort();

    // Update properties
    onStateChanged();
}


void Traffic::TcpTrafficDataSource::readFromStream()
{
    QString sentence;
    while( textStream.readLineInto(&sentence) ) {
        processFLARMMessage(sentence);
    }
}


void Traffic::TcpTrafficDataSource::onErrorOccurred(QAbstractSocket::SocketError socketError)
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



void Traffic::TcpTrafficDataSource::onStateChanged()
{
    // Paranoid safety check
    if (socket.isNull()) {
        return;
    }

    // Compute new status
    Traffic::AbstractTrafficDataSource::ConnectivityStatus newStatus = Traffic::AbstractTrafficDataSource::Disconnected;
    if (socket->state() != QAbstractSocket::UnconnectedState) {
        newStatus = Connecting;
    }
    if ( (socket->state() == QAbstractSocket::ConnectedState) ) {
        newStatus = Connected;
    }
    setConnectivityStatus(newStatus);

    // Acquire or release WiFi lock as appropriate
/*
 *     auto* mobileAdaptor = MobileAdaptor::globalInstance();
    if (mobileAdaptor != nullptr) {
        mobileAdaptor->lockWifi(_status == Receiving);
    }
    */
}
