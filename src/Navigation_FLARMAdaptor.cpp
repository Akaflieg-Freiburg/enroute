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
#include "Navigation_Traffic.h"
#include "SatNav.h"
#include <chrono>

using namespace std::chrono_literals;

// Static instance of this class
Q_GLOBAL_STATIC(Navigation::FLARMAdaptor, FLARMAdaptorStatic);


Navigation::FLARMAdaptor::FLARMAdaptor(QObject *parent) : QObject(parent) {
    // Create targets
    for(int i = 0; i<8; i++) {
        auto *traffic = new Navigation::Traffic(this);
        targets.append( traffic );
    }

    // Wire up socket
    connect(&socket, &QTcpSocket::connected, this, &Navigation::FLARMAdaptor::receiveSocketConnected);
    connect(&socket, &QTcpSocket::disconnected, this, &Navigation::FLARMAdaptor::receiveSocketDisconnected);
    connect(&socket, &QTcpSocket::errorOccurred, this, &Navigation::FLARMAdaptor::receiveSocketErrorOccurred);
    connect(&socket, &QTcpSocket::readyRead, this, &Navigation::FLARMAdaptor::readFromStream);

    // Wire up the connectTimer. Set it to attempt a FLARM connection every 5 Minutes
    connect(&connectTimer, &QTimer::timeout, this, &Navigation::FLARMAdaptor::connectToFLARM);
    connectTimer.start(5min);

    // Wire up the setStatus()
    heartBeatTimer.setSingleShot(true);
    heartBeatTimer.setInterval(5s);
    connect(&heartBeatTimer, &QTimer::timeout, this, &Navigation::FLARMAdaptor::setStatus);
    connect(&socket, &QTcpSocket::connected, this, &Navigation::FLARMAdaptor::setStatus);
    connect(&socket, &QTcpSocket::disconnected, this, &Navigation::FLARMAdaptor::setStatus);
    connect(&socket, &QTcpSocket::errorOccurred, this, &Navigation::FLARMAdaptor::setStatus);

    // Wire up setStatusString()
    connect(this, &Navigation::FLARMAdaptor::statusChanged, this, &Navigation::FLARMAdaptor::setStatusString);
    connect(&socket, &QTcpSocket::connected, this, &Navigation::FLARMAdaptor::setStatusString);
    connect(&socket, &QTcpSocket::disconnected, this, &Navigation::FLARMAdaptor::setStatusString);
    connect(&socket, &QTcpSocket::errorOccurred, this, &Navigation::FLARMAdaptor::setStatusString);

    // Try our first connect 30sec after startup
    QTimer::singleShot(30s, this, &Navigation::FLARMAdaptor::connectToFLARM);

    // Setup simulator
    //    auto simulatorFile = new QFile("/home/kebekus/Software/standards/FLARM/expiry-hard.txt");
    //    auto simulatorFile = new QFile("/home/kebekus/Software/standards/FLARM/expiry-soft.txt");
    //    auto simulatorFile = new QFile("/home/kebekus/Software/standards/FLARM/obstacles_from_gurtnellen_to_lake_constance.txt");
    connect(&simulatorTimer, &QTimer::timeout, this, &Navigation::FLARMAdaptor::readFromSimulatorStream);
        setSimulatorFile("/home/kebekus/Software/standards/FLARM/single_opponent.txt");
    // setSimulatorFile(QStringLiteral("/home/kebekus/Software/standards/FLARM/single_opponent_mode_s.txt"));
    //setSimulatorFile("/home/kebekus/Software/standards/FLARM/many_opponents.txt");

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
    socket.connectToHost(QStringLiteral("192.168.1.1"), 2000);
}


auto interpretNMEALatLong(const QString& A, const QString& B) -> qreal
{
    bool ok1 = false;
    bool ok2 = false;
    qreal result = A.leftRef(2).toDouble(&ok1) + A.midRef(2).toDouble(&ok2)/60.0;
    if (!ok1 || !ok2) {
        return qQNaN();
    }

    if ((B == u"S") || (B == u"W")) {
        result *= -1.0;
    }
    return result;
}


auto interpretNMEATime(const QString& timeString) -> QDateTime
{
    auto HH = timeString.mid(0,2);
    auto MM = timeString.mid(2,2);
    auto SS = timeString.mid(4,2);
    auto MS = timeString.mid(6);
    QTime time(HH.toInt(), MM.toInt(), SS.toInt());
    if (MS.isEmpty()) {
        time = time.addMSecs(qRound(MS.toDouble()*1000.0));
    }
    auto dateTime = QDateTime::currentDateTimeUtc();
    dateTime.setTime(time);
    return dateTime;
}


void Navigation::FLARMAdaptor::processFLARMMessage(QString msg)
{
    //    qWarning() << "Navigation::FLARMAdaptor::processFLARMMessage()" << msg;
    if (msg.isEmpty()) {
        return;
    }

    // Check that line starts with a dollar sign
    if (msg.isEmpty()) {
        return;
    }
    if (msg[0] != QStringLiteral("$")) {
        return;
    }
    msg = msg.mid(1);

    // Check the NMEA checksum
    auto pieces = msg.split(QStringLiteral("*"));
    if (pieces.length() != 2) {
        return;
    }
    msg = pieces[0];
    auto checksum = pieces[1].toInt(nullptr, 16);
    int myChecksum = 0;
    for(auto && i : msg) {
        myChecksum ^= i.toLatin1();
    }
    if (checksum != myChecksum) {
        return;
    }

    // Split the message into pieces
    auto arguments = msg.split(QStringLiteral(","));
    if (arguments.isEmpty()) {
        return;
    }
    auto messageType = arguments.takeFirst();

    // NMEA GPS 3D-fix data
    if (messageType == u"GPGGA") {
        if (arguments.length() < 9) {
            return;
        }

        // Quality check
        if (arguments[5] == u"0") {
            return;
        }

        // Get Time
        auto dateTime = interpretNMEATime(arguments[0]);
        if (!dateTime.isValid()) {
            return;
        }

        // Get coordinate
        bool ok = false;
        auto alt = arguments[8].toDouble(&ok);
        if (!ok) {
            return;
        }

        GPGGAHeight = AviationUnits::Distance::fromM(alt);
        GPGGATime = dateTime;
        return;
    }

    // Recommended minimum specific GPS/Transit data
    if (messageType == u"GPRMC") {
        if (arguments.length() < 8) {
            return;
        }

        // Quality check
        if (arguments[1] != u"A") {
            return;
        }

        // Get Time
        auto dateTime = interpretNMEATime(arguments[0]);
        if (!dateTime.isValid()) {
            return;
        }

        // Get coordinate
        auto lat = interpretNMEALatLong(arguments[2], arguments[3]);
        auto lon = interpretNMEALatLong(arguments[4], arguments[5]);
        QGeoCoordinate coordinate(lat, lon);
        if (!coordinate.isValid()) {
            return;
        }
        if (GPGGATime.secsTo(dateTime) < 5) {
            coordinate.setAltitude(GPGGAHeight.toM());
        }
        QGeoPositionInfo pInfo(coordinate, dateTime);

        // Ground speed
        bool ok = false;
        auto groundSpeed = AviationUnits::Speed::fromKT(arguments[6].toDouble(&ok));
        if (!ok) {
            return;
        }
        if (groundSpeed.isFinite()) {
            pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, groundSpeed.toMPS() );
        }

        // Track
        auto TT = arguments[7].toDouble(&ok);
        if (!ok) {
            return;
        }
        if (TT != qQNaN()) {
            pInfo.setAttribute(QGeoPositionInfo::Direction, TT );
        }

        qWarning() << "positionInfo       " << pInfo;
        return;
    }

    // Self-test result and errors codes
    if (messageType == u"PFLAE") {
        if (arguments.length() < 3) {
            return;
        }

        auto severity = arguments[1];
        auto errorCode = arguments[2];

        QStringList results;
        if (severity == u"0") {
            results << tr("No Error");
        }
        if (severity == u"1") {
            results << tr("Normal Operation");
        }
        if (severity == u"2") {
            results << tr("Reduced Functionality");
        }
        if (severity == u"3") {
            results << tr("Device INOP");
        }

        if (!errorCode.isEmpty()) {
            results << tr("Error code: %1").arg(errorCode);
        }
        if (errorCode == u"11") {
            results << tr("Firmware expired");
        }
        if (errorCode == u"12") {
            results << tr("Firmware update error");
        }
        if (errorCode == u"21") {
            results << tr("Power (Voltage < 8V)");
        }
        if (errorCode == u"22") {
            results << tr("UI error");
        }
        if (errorCode == u"23") {
            results << tr("Audio error");
        }
        if (errorCode == u"24") {
            results << tr("ADC error");
        }
        if (errorCode == u"25") {
            results << tr("SD card error");
        }
        if (errorCode == u"26") {
            results << tr("USB error");
        }
        if (errorCode == u"27") {
            results << tr("LED error");
        }
        if (errorCode == u"28") {
            results << tr("EEPROM error");
        }
        if (errorCode == u"29") {
            results << tr("General hardware error");
        }
        if (errorCode == u"2A") {
            results << tr("Transponder receiver Mode-C/S/ADS-B unserviceable");
        }
        if (errorCode == u"2B") {
            results << tr("EEPROM error");
        }
        if (errorCode == u"2C") {
            results << tr("GPIO error");
        }
        if (errorCode == u"31") {
            results << tr("GPS communication");
        }
        if (errorCode == u"32") {
            results << tr("Configuration of GPS module");
        }
        if (errorCode == u"33") {
            results << tr("GPS antenna");
        }
        if (errorCode == u"41") {
            results << tr("RF communication");
        }
        if (errorCode == u"42") {
            results << tr("Another FLARM device with the same Radio ID is being received. Alarms are suppressed for the relevant device.");
        }
        if (errorCode == u"43") {
            results << tr("Wrong ICAO 24-bit address or radio ID");
        }
        if (errorCode == u"51") {
            results << tr("Communication");
        }
        if (errorCode == u"61") {
            results << tr("Flash memory");
        }
        if (errorCode == u"71") {
            results << tr("Pressure sensor");
        }
        if (errorCode == u"81") {
            results << tr("Obstacle database (e.g. incorrect file type)");
        }
        if (errorCode == u"82") {
            results << tr("Obstacle database expired.");
        }
        if (errorCode == u"91") {
            results << tr("Flight recorder");
        }
        if (errorCode == u"93") {
            results << tr("Engine-noise recording not possible");
        }
        if (errorCode == u"94") {
            results << tr("Range analyzer");
        }
        if (errorCode == u"A1") {
            results << tr("Configuration error, e.g. while reading flarmcfg.txt from SD/USB.");
        }
        if (errorCode == u"B1") {
            results << tr("Invalid obstacle database license (e.g. wrong serial number)");
        }
        if (errorCode == u"B2") {
            results << tr("Invalid IGC feature license");
        }
        if (errorCode == u"B3") {
            results << tr("Invalid AUD feature license");
        }
        if (errorCode == u"B4") {
            results << tr("Invalid ENL feature license");
        }
        if (errorCode == u"B5") {
            results << tr("Invalid RFB feature license");
        }
        if (errorCode == u"B6") {
            results << tr("Invalid TIS feature license");
        }
        if (errorCode == u"100") {
            results << tr("Generic error");
        }
        if (errorCode == u"101") {
            results << tr("Flash File System error");
        }
        if (errorCode == u"110") {
            results << tr("Failure updating firmware of external display");
        }
        if (errorCode == u"120") {
            results << tr("Device is operated outside the designated region. The device does not work.");
        }
        auto result = results.join(QStringLiteral(" • "));

        // Emit results of self-test
        if (result != _FLARMSelfTest) {
            _FLARMSelfTest = result;
            emit FLARMSelfTestChanged();
            if ((severity == u"2") || (severity == u"3")) {
                emit flarmSelfTestFailed();
            }
        }
        return;
    }

    // Debug Information -- Ignore
    if (messageType == u"PFLAS") {
        return;
    }

    // FLARM Heartbeat
    if (messageType == u"PFLAU") {
        heartBeatTimer.start();
        setStatus();
        return;
    }

    // Version information
    if (messageType == u"PFLAV") {
        if (arguments.length() < 4) {
            return;
        }

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
    if (messageType == u"PGRMZ") {
        if (arguments.length() < 2) {
            return;
        }

        // Quality check
        if (arguments[1] != u"F") {
            return;
        }

        bool ok = false;
        auto barometricAlt = AviationUnits::Distance::fromFT(arguments[0].toDouble(&ok));
        if (!ok) {
            return;
        }
        if (!barometricAlt.isFinite()) {
            return;
        }

        setBarometricAltitude(barometricAlt);
        return;
    }

    // ===============0

    // Data on other proximate aircraft
    if (messageType == u"PFLAA") {
        //qWarning() << "Data on other proximate aircraft"  << arguments;

        auto *satNav = SatNav::globalInstance();
        if (satNav == nullptr) {
            return;
        }

        // Helper variable
        bool ok = false;

        //
        // We begin by reading a few fields that are relevant both for directional and for non-directional targets
        //

        // Alarm level is mandatory
        auto alarmLevel = arguments[0].toInt(&ok);
        if (!ok) {
            return;
        }
        if ((alarmLevel < 0)||(alarmLevel > 3)) {
            return;
        }

        // Relative vertical information is optional
        auto relativeVertical = arguments[3].toDouble(&ok);
        if (!ok) {
            relativeVertical = qQNaN();
        }

        // Target type is optional
        auto targetType = arguments[10];
        Navigation::Traffic::AircraftType type = Navigation::Traffic::unknown;
        if (targetType == u"1") {
            type = Navigation::Traffic::Glider;
        }
        if (targetType == u"2") {
            type = Navigation::Traffic::TowPlane;
        }
        if (targetType == u"3") {
            type = Navigation::Traffic::Copter;
        }
        if (targetType == u"4") {
            type = Navigation::Traffic::Skydiver;
        }
        if (targetType == u"5") {
            type = Navigation::Traffic::Aircraft;
        }
        if (targetType == u"6") {
            type = Navigation::Traffic::HangGlider;
        }
        if (targetType == u"7") {
            type = Navigation::Traffic::Paraglider;
        }
        if (targetType == u"8") {
            type = Navigation::Traffic::Aircraft;
        }
        if (targetType == u"9") {
            type = Navigation::Traffic::Jet;
        }
        if (targetType == u"B") {
            type = Navigation::Traffic::Balloon;
        }
        if (targetType == u"C") {
            type = Navigation::Traffic::Airship;
        }
        if (targetType == u"F") {
            type = Navigation::Traffic::StaticObstacle;
        }

        // Target ID is optional
        auto targetID = arguments[5];


        //
        // Handle non-directional targets
        //
        if (arguments[2] == u"") {
            // Horizontal distance is mandatory
            auto hDist = AviationUnits::Distance::fromM(arguments[1].toDouble(&ok));
            if (!ok) {
                return;
            }

            // Vertical distance is optional
            auto vDist = AviationUnits::Distance::fromM(arguments[3].toDouble(&ok));
            if (!ok) {
                vDist = AviationUnits::Distance::fromM(qQNaN());
            }

            auto trafficNP = Navigation::Traffic(this);
            qWarning() << hDist.toM();
            trafficNP.setData(alarmLevel, targetID, hDist, vDist, type, QGeoPositionInfo(QGeoCoordinate(), QDateTime::currentDateTimeUtc()));
            if ((trafficNP.ID() == targetNoPos.ID()) || trafficNP.hasHigherPriorityThan(targetNoPos)) {
                targetNoPos.copyFrom(trafficNP);
            }
            return;
        }

        //
        // From now on, we assume that we have a directional target
        //

        // As a first step, we obtain the target's coordinate. We take our own coordinate as a starting point.
        auto targetCoordinate = satNav->lastValidCoordinate();
        if (!targetCoordinate.isValid()) {
            return;
        }
        auto relativeNorth = arguments[1].toDouble(&ok);
        if (!ok) {
            return;
        }
        targetCoordinate = targetCoordinate.atDistanceAndAzimuth(relativeNorth, 0);
        auto relativeEast = arguments[2].toDouble(&ok);
        if (!ok) {
            return;
        }
        targetCoordinate = targetCoordinate.atDistanceAndAzimuth(relativeEast, 90);
        if (qIsFinite(relativeVertical)) {
            targetCoordinate = targetCoordinate.atDistanceAndAzimuth(0, 0, relativeVertical);
        }
        auto hDist = AviationUnits::Distance::fromM(sqrt(relativeNorth*relativeNorth+relativeEast*relativeEast));

        // Construct a PositionInfo object that contains additional information (such as ground speed, if available)
        QGeoPositionInfo pInfo(targetCoordinate, QDateTime::currentDateTimeUtc());
        auto targetTT = arguments[6].toInt(&ok);
        if (ok) {
            pInfo.setAttribute(QGeoPositionInfo::Direction, targetTT);
        }
        auto targetGS = arguments[8].toDouble(&ok);
        if (ok) {
            pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, targetGS);
        }
        auto targetVS = arguments[9].toDouble(&ok);
        if (ok) {
            pInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, targetVS);
        }

        // Construct a traffic object
        auto traffic = Navigation::Traffic(this);

        traffic.setData(alarmLevel, targetID, hDist, AviationUnits::Distance::fromM(relativeVertical), type, pInfo);

        foreach(auto target, targets)
            if (targetID == target->ID()) {
                target->copyFrom(traffic);
                return;
            }

        auto *lowestPriObject = targets[0];
        foreach(auto target, targets)
            if (lowestPriObject->hasHigherPriorityThan(*target)) {
                lowestPriObject = target;
            }
        if (traffic.hasHigherPriorityThan(*lowestPriObject)) {
            lowestPriObject->copyFrom(traffic);
        }

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


void Navigation::FLARMAdaptor::readFromStream()
{
    qWarning() << "Navigation::FLARMAdaptor::readFromStream()" << stream.readLine();
}


void Navigation::FLARMAdaptor::readFromSimulatorStream()
{
    //    qWarning() << "Navigation::FLARMAdaptor::readFromSimulatorStream()";
    if ((simulatorFile.error() != QFileDevice::NoError) || simulatorStream.atEnd()) {
        setSimulatorFile();
        return;
    }

    if (!lastPayload.isEmpty()) {
        processFLARMMessage(lastPayload);
    }

    auto line = simulatorStream.readLine();
    auto tuple = line.split(QStringLiteral(" "));
    if (tuple.length() < 2) {
        return;
    }
    auto time = tuple[0].toInt();
    lastPayload = tuple[1];

    if (lastTime == 0) {
        simulatorTimer.setInterval(0);
    } else {
        simulatorTimer.setInterval(time-lastTime);
    }
    simulatorTimer.start();
    lastTime = time;
}


void Navigation::FLARMAdaptor::setStatus()
{
    Status newStatus = Status::InOp;
    if (heartBeatTimer.isActive()) {
        newStatus = Status::OK;
    }

    if (newStatus == _status) {
        return;
    }
    _status = newStatus;
    qWarning() << "status" << _status;
    emit statusChanged();
}


void Navigation::FLARMAdaptor::setStatusString()
{
    auto computeString = [=, this]()
    {
        // Simulator mode
        if (simulatorFile.isOpen()) {
            if (status() == Status::OK) {
                return tr("Simulation • OK");
            }
            return tr("Simulation • Waiting for data");
        }

        if (status() == Status::OK) {
            return tr("OK");
        }
        if (socket.state() == QAbstractSocket::ConnectedState) {
            return tr("Connected • Waiting for data");
        }
        if (socket.state() == QAbstractSocket::ConnectingState) {
            return tr("Trying to connect…");
        }
        return tr("Not connected");
    };


    auto result = computeString();
    if  (result == _statusString) {
        return;
    }

    _statusString = result;
    emit statusStringChanged();
}


void Navigation::FLARMAdaptor::setSimulatorFile(const QString& fileName)
{
    lastPayload = QString();
    simulatorFile.close();

    if (fileName.isEmpty()) {
        setStatus();
        setStatusString();
        return;
    }

    simulatorFile.setFileName(fileName);
    if (!simulatorFile.open(QIODevice::ReadOnly)) {
        return;
    }

    simulatorStream.setDevice(&simulatorFile);
    readFromSimulatorStream();
    setStatus();
    setStatusString();
}


void Navigation::FLARMAdaptor::setNonDirectionalTargetDistance(AviationUnits::Distance hDist, AviationUnits::Distance vDist)
{
    if (hDist != _nonDirectionalTargetHDistance) {
        _nonDirectionalTargetHDistance = hDist;
        emit nonDirectionalTargetHDistanceChanged();
    }

    if (vDist != _nonDirectionalTargetVDistance) {
        _nonDirectionalTargetVDistance = vDist;
        emit nonDirectionalTargetVDistanceChanged();
    }
}


void Navigation::FLARMAdaptor::setBarometricAltitude(AviationUnits::Distance alt)
{
    if (alt == _barometricAltitude) {
        return;
    }

    _barometricAltitude = alt;
    emit barometricAltitudeChanged();
}
