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

#include "traffic/TrafficDataSource_Tcp.h"


// Member functions

Traffic::TrafficDataSource_Tcp::TrafficDataSource_Tcp(QString hostName, quint16 port, QObject *parent) :
    Traffic::TrafficDataSource_AbstractSocket(parent), m_hostName(std::move(hostName)), m_port(port) {

    // Create socket
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Traffic::TrafficDataSource_Tcp::onErrorOccurred);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Traffic::TrafficDataSource_Tcp::onReadyRead);
    connect(&m_socket, &QTcpSocket::stateChanged, this, &Traffic::TrafficDataSource_Tcp::onStateChanged);

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

    m_socket.abort();
    setErrorString();
    m_socket.connectToHost(m_hostName, m_port);
    m_textStream.setDevice(&m_socket);

    // Update properties
    onStateChanged(m_socket.state());

}


void Traffic::TrafficDataSource_Tcp::disconnectFromTrafficReceiver()
{

    // Disconnect socket.
    m_socket.abort();

    // Update properties
    onStateChanged(m_socket.state());

}


void Traffic::TrafficDataSource_Tcp::onReadyRead()
{

    QString sentence;
    while( m_textStream.readLineInto(&sentence) ) {
        processFLARMSentence(sentence);
    }

}

