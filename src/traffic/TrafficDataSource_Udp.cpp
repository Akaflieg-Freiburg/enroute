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

#include <QNetworkDatagram>

#include "traffic/TrafficDataSource_Udp.h"


// Member functions

Traffic::TrafficDataSource_Udp::TrafficDataSource_Udp(quint16 port, QObject *parent) :
    Traffic::TrafficDataSource_AbstractSocket(parent), m_port(port) {

    // Initialize timers
    m_trueAltitudeTimer.setInterval(5s);
    m_trueAltitudeTimer.setSingleShot(true);

    //
    // Initialize properties
    //
    TrafficDataSource_Udp::disconnectFromTrafficReceiver();
}


Traffic::TrafficDataSource_Udp::~TrafficDataSource_Udp()
{

    Traffic::TrafficDataSource_Udp::disconnectFromTrafficReceiver();
    setReceivingHeartbeat(false); // This will release the WiFi lock if necessary

}


void Traffic::TrafficDataSource_Udp::connectToTrafficReceiver()
{
    // Do not do anything if the traffic receiver is connected and is receiving.
    if (receivingHeartbeat()) {
        return;
    }

    // Paranoid safety checks
    if (!m_socket.isNull()) {
        delete m_socket;
    }

    // Create socket
    m_socket = new QUdpSocket(this);
    connect(m_socket, &QUdpSocket::errorOccurred, this, &Traffic::TrafficDataSource_Udp::onErrorOccurred);
    connect(m_socket, &QUdpSocket::readyRead, this, &Traffic::TrafficDataSource_Udp::onReadyRead);
    connect(m_socket, &QUdpSocket::stateChanged, this, &Traffic::TrafficDataSource_Udp::onStateChanged);
    m_socket->bind(m_port);

    // Update properties
    setErrorString();
    onStateChanged(m_socket->state());
}


void Traffic::TrafficDataSource_Udp::disconnectFromTrafficReceiver()
{

    // Disconnect socket.
    if (!m_socket.isNull()) {
        m_socket->abort();
    }
    delete m_socket;

    // Update properties
    onStateChanged(QAbstractSocket::UnconnectedState);

}


void Traffic::TrafficDataSource_Udp::onReadyRead()
{
    // Paranoid safety checks
    if (m_socket.isNull()) {
        return;
    }

    // Read datagrams
    while (m_socket->hasPendingDatagrams()) {
        QByteArray data = m_socket->receiveDatagram().data();

        // Return immediately if the datagram has already been received.
        auto currentDatagramHash = qHash(data);
        foreach(auto hash, receivedDatagramHashes) {
            if (hash == currentDatagramHash) {
                return;
            }
        }
        receivedDatagramHashes[nextHashIndex] = currentDatagramHash;
        nextHashIndex = (nextHashIndex+1) % receivedDatagramHashes.size();

        // Process datagrams, depending on content type
        if (data.startsWith("XGPS") || data.startsWith("XTRA")) {
            processXGPSString(data);
        } else {
            // Split data into raw messages
            foreach(auto rawMessage, data.split(0x7e)) {
                if (!rawMessage.isEmpty()) {
                    processGDLMessage(rawMessage);
                }
            }

        }
    }

}
