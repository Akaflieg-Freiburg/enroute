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
#include <QFile>

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

    // Setup simulator
//    auto simulatorFile = new QFile("/home/kebekus/Software/standards/FLARM/single_opponent.txt");
    auto simulatorFile = new QFile("/home/kebekus/Software/standards/FLARM/obstacles_from_gurtnellen_to_lake_constance.txt");

    simulatorFile->open(QIODevice::ReadOnly);
    simulatorStream.setDevice(simulatorFile);
    connect(&simulatorTimer, &QTimer::timeout, this, &Navigation::FLARMAdaptor::readFromSimulatorStream);
    simulatorTimer.setSingleShot(true);
    readFromSimulatorStream();
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


void Navigation::FLARMAdaptor::processFLARMMessage(QString msg)
{
//    qWarning() << "Navigation::FLARMAdaptor::processFLARMMessage()" << msg;
    auto pieces = msg.split(",");

    // FLARM Heartbeat
    if (pieces[0] == "$PFLAU") {
//        qWarning() << "FLARM Heartbeat";

        if ((pieces.size() < 9) || (pieces.size() > 10)) {
            qWarning() << "Message ill-formed";
            return;
        }

        auto RX = pieces[1];
        auto TX = pieces[2];
        auto GPSPower = pieces[3];
        auto AlarmLevel = pieces[4];
        auto RelativeBearing = pieces[5];
        auto AlarmType = pieces[6];
        auto RelativeVertical = pieces[7];
        auto RelativeDistance = pieces[8];
        auto ID = pieces[9];
        return;
    }

    // Data on other proximate aircraft
    if (pieces[0] == "$PFLAA") {
//        qWarning() << "Data on other proximate aircraft";
        return;
    }

    // Garmin's barometric altitude
    if (pieces[0] == "$PGRMZ") {
//        qWarning() << "Garmin's barometric altitude";
        return;
    }

    // Recommended minimum specific GPS/Transit data
    if (pieces[0] == "$GPRMC") {
//        qWarning() << "Recommended minimum specific GPS/Transit data";
        return;
    }

    // NMEA GPS 3D-fix data
    if (pieces[0] == "$GPGGA") {
//        qWarning() << "NMEA GPS 3D-fix data";
        return;
    }

    // Self-test result and errors codes
    if (pieces[0] == "$PFLAE") {
//        qWarning() << "Self-test result and errors codes";
        return;
    }

    // Version information
    if (pieces[0] == "$PFLAV") {
//        qWarning() << "Version information";
        return;
    }

    qWarning() << "FLARM Sentence not understood" << pieces[0];
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


void Navigation::FLARMAdaptor::readFromSimulatorStream()
{
//    qWarning() << "Navigation::FLARMAdaptor::readFromSimulatorStream()";

    if (!lastPayload.isEmpty())
        processFLARMMessage(lastPayload);

    auto line = simulatorStream.readLine();
    auto tuple = line.split(" ");
    auto time = tuple[0].toInt();
    lastPayload = tuple[1];

    if (lastTime == 0)
        simulatorTimer.setInterval(0);
    else
        simulatorTimer.setInterval(time-lastTime);
    simulatorTimer.start();
    lastTime = time;
}
