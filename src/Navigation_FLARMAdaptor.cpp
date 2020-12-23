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
#include <QGeoCoordinate>
#include <QGeoPositionInfo>

#include "AviationUnits.h"
#include "Navigation_FLARMAdaptor.h"

// Static instance of this class
Q_GLOBAL_STATIC(Navigation::FLARMAdaptor, FLARMAdaptorStatic);


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



auto Navigation::FLARMAdaptor::globalInstance() -> Navigation::FLARMAdaptor *
{
    return FLARMAdaptorStatic;
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


qreal interpretNMEALatLong(QString A, QString B)
{
    bool ok1, ok2;
    qreal result = A.left(2).toDouble(&ok1) + A.mid(2).toDouble(&ok2)/60.0;
    if (!ok1 || !ok2)
        return qQNaN();

    if ((B == "S") || (B == "W"))
        result *= -1.0;
    return result;
}

QDateTime interpretNMEATime(QString timeString)
{
    auto HH = timeString.mid(0,2);
    auto MM = timeString.mid(2,2);
    auto SS = timeString.mid(4,2);
    auto MS = timeString.mid(6);
    QTime time(HH.toInt(), MM.toInt(), SS.toInt());
    if (MS.isEmpty())
        time = time.addMSecs(qRound(MS.toDouble()*1000.0));
    auto dateTime = QDateTime::currentDateTimeUtc();
    dateTime.setTime(time);
    return dateTime;
}


void Navigation::FLARMAdaptor::processFLARMMessage(QString msg)
{
//    qWarning() << "Navigation::FLARMAdaptor::processFLARMMessage()" << msg;
    if (msg.isEmpty())
        return;

    // Check that line starts with a dollar sign
    if (msg[0] != "$")
        return;
    msg = msg.mid(1);

    // Check the NMEA checksum
    auto pieces = msg.split("*");
    if (pieces.length() != 2)
        return;
    msg = pieces[0];
    auto checksum = pieces[1].toInt(nullptr, 16);
    int myChecksum = 0;
    for(int i=0; i<msg.length(); i++)
       myChecksum ^= msg[i].toLatin1();
    if (checksum != myChecksum)
        return;

    // Split the message into pieces
    auto arguments = msg.split(",");
    if (arguments.isEmpty())
        return;
    auto messageType = arguments.takeFirst();

    // FLARM Heartbeat
    if (pieces[0] == "$PFLAU") {
//        qWarning() << "FLARM Heartbeat";

        if ((pieces.size() < 9) || (pieces.size() > 10)) {
            qWarning() << "Message ill-formed";
            return;
        }

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
    if (messageType == "GPRMC") {
        if (arguments.length() < 8)
            return;

        // Quality check
        if (arguments[1] != "A")
            return;

        // Get Time
        auto dateTime = interpretNMEATime(arguments[0]);
        if (!dateTime.isValid())
            return;

        // Get coordinate
        auto lat = interpretNMEALatLong(arguments[2], arguments[3]);
        auto lon = interpretNMEALatLong(arguments[4], arguments[5]);
        QGeoCoordinate coordinate(lat, lon);
        if (!coordinate.isValid())
            return;
        QGeoPositionInfo pInfo(coordinate, dateTime);

        // Ground speed
        bool ok;
        auto groundSpeed = AviationUnits::Speed::fromKT(arguments[6].toDouble(&ok));
        if (!ok)
            return;
        if (groundSpeed.isFinite())
            pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, groundSpeed.toMPS() );

        // Track
        auto TT = arguments[7].toDouble(&ok);
        if (!ok)
            return;
        if (TT != qQNaN())
            pInfo.setAttribute(QGeoPositionInfo::Direction, TT );

        qWarning() << "GPRMC" << pInfo;
        return;
    }

    // NMEA GPS 3D-fix data
    if (messageType == "GPGGA") {
        if (arguments.length() < 9)
            return;

        // Quality check
        if (arguments[5] == "0")
            return;

        // Get Time
        auto dateTime = interpretNMEATime(arguments[0]);
        if (!dateTime.isValid())
            return;

        // Get coordinate
        auto lat = interpretNMEALatLong(arguments[1], arguments[2]);
        auto lon = interpretNMEALatLong(arguments[3], arguments[4]);
        auto alt = arguments[8].toDouble();
        QGeoCoordinate coordinate(lat, lon, alt);
        if (!coordinate.isValid())
            return;

        // Construct positionInfo
        QGeoPositionInfo pInfo(coordinate, dateTime);

        qWarning() << "GPGGA" << pInfo;
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
