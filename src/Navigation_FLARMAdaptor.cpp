/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include <QDebug>

#include "Navigation_FLARMAdaptor.h"


Navigation::FLARMAdaptor::FLARMAdaptor(QObject *parent) : QObject(parent) {

    // Wire up socket
    connect(&socket, &QTcpSocket::connected, this, &Navigation::FLARMAdaptor::receiveSocketConnected);
    connect(&socket, &QTcpSocket::disconnected, this, &Navigation::FLARMAdaptor::receiveSocketDisconnected);
    connect(&socket, &QTcpSocket::errorOccurred, this, &Navigation::FLARMAdaptor::receiveSocketErrorOccurred);
    connect(&socket, &QTcpSocket::readyRead, this, &Navigation::FLARMAdaptor::readFromStream);

    // Wire up the connectTimer. Set it to attempt a FLARM connection every 5 Minutes
    connect(&connectTimer, &QTimer::timeout, this, &Navigation::FLARMAdaptor::connectToFLARM);
    connectTimer.start(1000*60*5);

    // Try our first connect 30sec after startup
    QTimer::singleShot(1000*30, this, &Navigation::FLARMAdaptor::connectToFLARM);
}


void Navigation::FLARMAdaptor::connectToFLARM()
{
    qWarning() << "Navigation::FLARMAdaptor::connectToFLARM()";

    if (socket.state() != QAbstractSocket::UnconnectedState) {
        qWarning() << "Navigation::FLARMAdaptor::connectToFLARM(): socket is not unconnected";
        return;
    }

    // Expect a FLARM device at address 192.168.1.1, port 2000^
    qWarning() << "Navigation::FLARMAdaptor::connectToFLARM() initiate connection";
    socket.connectToHost("192.168.1.1", 2000);
}


void Navigation::FLARMAdaptor::receiveSocketConnected()
{
    qWarning() << "Navigation::FLARMAdaptor::receiveSocketConnected()";
    stream.setDevice(&socket);
}


void Navigation::FLARMAdaptor::receiveSocketDisconnected()
{
    qWarning() << "Navigation::FLARMAdaptor::receiveSocketDisconnected()";
}


void Navigation::FLARMAdaptor::receiveSocketErrorOccurred(QAbstractSocket::SocketError socketError)
{
    qWarning() << "Navigation::FLARMAdaptor::receiveSocketErrorOccurred()" << socketError;
}


bool Navigation::FLARMAdaptor::receiving() const
{
    return false;
}


void Navigation::FLARMAdaptor::readFromStream()
{
    qWarning() << "Navigation::FLARMAdaptor::readFromStream()" << stream.readLine();
}
