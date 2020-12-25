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
//    auto simulatorFile = new QFile("/home/kebekus/Software/standards/FLARM/expiry-hard.txt");
    auto simulatorFile = new QFile("/home/kebekus/Software/standards/FLARM/expiry-soft.txt");
//    auto simulatorFile = new QFile("/home/kebekus/Software/standards/FLARM/obstacles_from_gurtnellen_to_lake_constance.txt");
//    auto simulatorFile = new QFile("/home/kebekus/Software/standards/FLARM/single_opponent.txt");

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


void Navigation::FLARMAdaptor::clearFlarmDeviceInfo()
{
    _FLARMHwVersion = QString();
    _FLARMSwVersion = QString();
    _FLARMObstVersion = QString();

    if (!_FLARMSelfTest.isEmpty()) {
        _FLARMSelfTest = QString();
        emit FLARMSelfTestChanged();
    }
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
        bool ok;
        auto alt = arguments[8].toDouble(&ok);
        if (!ok)
            return;

        GPGGAHeight = AviationUnits::Distance::fromM(alt);
        GPGGATime = dateTime;
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
        if (GPGGATime.secsTo(dateTime) < 5)
            coordinate.setAltitude(GPGGAHeight.toM());
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

        qWarning() << "positionInfo       " << pInfo;
        return;
    }

    // Self-test result and errors codes
    if (messageType == "PFLAE") {
        if (arguments.length() < 3)
            return;

        auto severity = arguments[1];
        auto errorCode = arguments[2];

        QStringList results;
        if (severity == "0")
            results << tr("No Error");
        if (severity == "1")
            results << tr("Normal Operation");
        if (severity == "2")
            results << tr("Reduced Functionality");
        if (severity == "3")
            results << tr("Device INOP");

        if (!errorCode.isEmpty())
            results << tr("Error code: %1").arg(errorCode);
        if (errorCode == "11")
            results << tr("Firmware expired");
        if (errorCode == "12")
            results << tr("Firmware update error");
        if (errorCode == "21")
            results << tr("Power (Voltage < 8V)");
        if (errorCode == "22")
            results << tr("UI error");
        if (errorCode == "23")
            results << tr("Audio error");
        if (errorCode == "24")
            results << tr("ADC error");
        if (errorCode == "25")
            results << tr("SD card error");
        if (errorCode == "26")
            results << tr("USB error");
        if (errorCode == "27")
            results << tr("LED error");
        if (errorCode == "28")
            results << tr("EEPROM error");
        if (errorCode == "29")
            results << tr("General hardware error");
        if (errorCode == "2A")
            results << tr("Transponder receiver Mode-C/S/ADS-B unserviceable");
        if (errorCode == "2B")
            results << tr("EEPROM error");
        if (errorCode == "2C")
            results << tr("GPIO error");
        if (errorCode == "31")
            results << tr("GPS communication");
        if (errorCode == "32")
            results << tr("Configuration of GPS module");
        if (errorCode == "33")
            results << tr("GPS antenna");
        if (errorCode == "41")
            results << tr("RF communication");
        if (errorCode == "42")
            results << tr("Another FLARM device with the same Radio ID is being received. Alarms are suppressed for the relevant device.");
        if (errorCode == "43")
            results << tr("Wrong ICAO 24-bit address or radio ID");
        if (errorCode == "51")
            results << tr("Communication");
        if (errorCode == "61")
            results << tr("Flash memory");
        if (errorCode == "71")
            results << tr("Pressure sensor");
        if (errorCode == "81")
            results << tr("Obstacle database (e.g. incorrect file type)");
        if (errorCode == "82")
            results << tr("Obstacle database expired.");
        if (errorCode == "91")
            results << tr("Flight recorder");
        if (errorCode == "93")
            results << tr("Engine-noise recording not possible");
        if (errorCode == "94")
            results << tr("Range analyzer");
        if (errorCode == "A1")
            results << tr("Configuration error, e.g. while reading flarmcfg.txt from SD/USB.");
        if (errorCode == "B1")
            results << tr("Invalid obstacle database license (e.g. wrong serial number)");
        if (errorCode == "B2")
            results << tr("Invalid IGC feature license");
        if (errorCode == "B3")
            results << tr("Invalid AUD feature license");
        if (errorCode == "B4")
            results << tr("Invalid ENL feature license");
        if (errorCode == "B5")
            results << tr("Invalid RFB feature license");
        if (errorCode == "B6")
            results << tr("Invalid TIS feature license");
        if (errorCode == "100")
            results << tr("Generic error");
        if (errorCode == "101")
            results << tr("Flash File System error");
        if (errorCode == "110")
            results << tr("Failure updating firmware of external display");
        if (errorCode == "120")
            results << tr("Device is operated outside the designated region. The device does not work.");
        auto result = results.join(" • ");

        // Emit results of self-test
        if (result != _FLARMSelfTest) {
            _FLARMSelfTest = result;
            emit FLARMSelfTestChanged();
            if ((severity == "2") || (severity == "3"))
                emit flarmSelfTestFailed();
        }
        return;
    }

    // Version information
    if (messageType == "PFLAV") {
        if (arguments.length() < 4)
            return;

        if (_FLARMHwVersion != arguments[1]) {
            _FLARMHwVersion = arguments[1];
            emit FLARMHwVersionChanged();
        }

        if (_FLARMSwVersion != arguments[2]) {
            _FLARMSwVersion = arguments[2];
            emit FLARMSwVersionChanged();
        }

        if (_FLARMObstVersion != arguments[3]) {
            _FLARMObstVersion = arguments[3];
            emit FLARMObstVersionChanged();
        }
        return;
    }

    // Garmin's barometric altitude
    if (messageType == "PGRMZ") {
        if (arguments.length() < 2)
            return;

        // Quality check
        if (arguments[1] != "F")
            return;

        bool ok;
        auto barometricAlt = AviationUnits::Distance::fromFT(arguments[0].toDouble(&ok));
        if (!ok)
            return;
        if (!barometricAlt.isFinite())
            return;

        qWarning() << "Barometric Altitude" << barometricAlt.toM() << "m";
        return;
    }

    // ===============0

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
    clearFlarmDeviceInfo();
}


void Navigation::FLARMAdaptor::receiveSocketErrorOccurred(QAbstractSocket::SocketError socketError)
{
    qWarning() << "Navigation::FLARMAdaptor::receiveSocketErrorOccurred()" << socketError;
    clearFlarmDeviceInfo();
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
