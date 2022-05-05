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

#include "GlobalObject.h"
#include "MobileAdaptor.h"
#include "traffic/PasswordDB.h"
#include "traffic/TrafficDataSource_Tcp.h"

// Member functions

Traffic::TrafficDataSource_Tcp::TrafficDataSource_Tcp(QString hostName, quint16 port, QObject *parent) :
    Traffic::TrafficDataSource_AbstractSocket(parent), m_hostName(std::move(hostName)), m_port(port) {

    // Connect socket
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Traffic::TrafficDataSource_Tcp::onErrorOccurred);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Traffic::TrafficDataSource_Tcp::onReadyRead);
    connect(&m_socket, &QTcpSocket::stateChanged, this, &Traffic::TrafficDataSource_Tcp::onStateChanged);
    connect(&m_socket, &QAbstractSocket::disconnected, this, &Traffic::TrafficDataSource_Tcp::connectToTrafficReceiver, Qt::ConnectionType::QueuedConnection);

    // Set up text stream
    m_textStream.setDevice(&m_socket);
    m_textStream.setCodec("ISO 8859-1");

    //
    // Initialize properties
    //
    onStateChanged(m_socket.state());

}


Traffic::TrafficDataSource_Tcp::~TrafficDataSource_Tcp()
{

    Traffic::TrafficDataSource_Tcp::disconnectFromTrafficReceiver();
    setReceivingHeartbeat(false); // This will release the WiFi lock if necessary

}


void Traffic::TrafficDataSource_Tcp::connectToTrafficReceiver()
{
    // Do not do anything if the traffic receiver is connected and is receiving.
    if (receivingHeartbeat()) {
        return;
    }

    // Reset password lifecycle
    resetPasswordLifecycle();

    // Start new connection
    m_socket.abort();
    setErrorString();
    m_socket.setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_socket.setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    m_socket.connectToHost(m_hostName, m_port);
    m_textStream.setDevice(&m_socket);

    // Update properties
    onStateChanged(m_socket.state());

}


void Traffic::TrafficDataSource_Tcp::disconnectFromTrafficReceiver()
{

    // Reset password lifecycle
    resetPasswordLifecycle();

    // Disconnect socket.
    m_socket.abort();

    // Update properties
    onStateChanged(m_socket.state());

}


void Traffic::TrafficDataSource_Tcp::onReadyRead()
{

    QString sentence;
    while( m_textStream.readLineInto(&sentence) ) {

        // Check if the TCP connection asks for a password
        if (sentence.startsWith(QLatin1String("PASS?"))) {
            passwordRequest_Status = waitingForPassword;
            passwordRequest_SSID = MobileAdaptor::getSSID();
            auto* passwordDB = GlobalObject::passwordDB();
            if (passwordDB->contains(passwordRequest_SSID)) {
                setPassword(passwordRequest_SSID, passwordDB->getPassword(passwordRequest_SSID));
            } else {
                emit passwordRequest(passwordRequest_SSID);
            }
            continue;
        }

        // Process FLARM sentence
        processFLARMSentence(sentence);
    }

}


void Traffic::TrafficDataSource_Tcp::resetPasswordLifecycle()
{
    passwordRequest_Status = idle;
    passwordRequest_SSID = QString();
    passwordRequest_password = QString();

    disconnect(this, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataSource_Tcp::updatePasswordStatusOnHeartbeatChange);
    disconnect(&m_socket, &QTcpSocket::disconnected, this, &Traffic::TrafficDataSource_Tcp::updatePasswordStatusOnDisconnected);

}


void Traffic::TrafficDataSource_Tcp::setPassword(const QString& SSID, const QString& password)
{
    if (passwordRequest_Status != waitingForPassword) {
        return;
    }
    if (SSID != passwordRequest_SSID) {
        return;
    }

    // First case: the device is already delivering data. This happens for Stratux devices
    // that request a password for historical reasons, but really do not need one.
    // In this case, accept the password immediately and issue a password storage request
    // if appropriate
    if (receivingHeartbeat()) {
        // emit a password storage request if appropriate
        auto* passwordDB = GlobalObject::passwordDB();
        if (!passwordDB->contains(passwordRequest_SSID) ||
            (passwordDB->getPassword(passwordRequest_SSID) != passwordRequest_password)) {
            emit passwordStorageRequest(passwordRequest_SSID, passwordRequest_password);
        }
        return;
    }

    // Second case: the devise is not yet delivering data. This is the normal case.
    passwordRequest_password = password;
    QTimer::singleShot(0, this, &Traffic::TrafficDataSource_Tcp::sendPassword_internal);
}


void Traffic::TrafficDataSource_Tcp::sendPassword_internal()
{

    // Make sure that this instance is in the state that we think it is
    // Otherwise, abort.
    if ((passwordRequest_Status != waitingForPassword)
        || (m_socket.state() != QAbstractSocket::ConnectedState)
        || receivingHeartbeat()) {
        resetPasswordLifecycle();
        return;
    }

    // Connect signals
    connect(this, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataSource_Tcp::updatePasswordStatusOnHeartbeatChange);
    connect(&m_socket, &QTcpSocket::disconnected, this, &Traffic::TrafficDataSource_Tcp::updatePasswordStatusOnDisconnected);

    m_textStream << passwordRequest_password+"\n";
    m_textStream.flush();
    passwordRequest_Status = waitingForDevice;

}


void Traffic::TrafficDataSource_Tcp::updatePasswordStatusOnDisconnected()
{
    if (passwordRequest_Status != waitingForDevice) {
        resetPasswordLifecycle();
        return;
    }

    // Remove password from database
    GlobalObject::passwordDB()->removePassword(passwordRequest_SSID);

    // Schedule reconnection in 500ms
    QTimer::singleShot(500ms, this, &Traffic::TrafficDataSource_Tcp::connectToTrafficReceiver);

    // Clear and reset
    resetPasswordLifecycle();
}


void Traffic::TrafficDataSource_Tcp::updatePasswordStatusOnHeartbeatChange(bool newHeartbeat)
{

    if (!newHeartbeat) {
        return;
    }
    if (passwordRequest_Status != waitingForDevice) {
        resetPasswordLifecycle();
        return;
    }

    // emit a password storage request if appropriate
    auto* passwordDB = GlobalObject::passwordDB();
    if (!passwordDB->contains(passwordRequest_SSID) ||
        (passwordDB->getPassword(passwordRequest_SSID) != passwordRequest_password)) {
        emit passwordStorageRequest(passwordRequest_SSID, passwordRequest_password);
    }

    resetPasswordLifecycle();
}
