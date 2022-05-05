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
#include "positioning/PositionProvider.h"
#include "traffic/FlarmnetDB.h"
#include "traffic/TrafficDataSource_Abstract.h"


// Static Helper functions

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


// Member functions

void Traffic::TrafficDataSource_Abstract::processFLARMSentence(QString sentence)
{
    if (sentence.isEmpty()) {
        return;
    }

    // Check that line starts with a dollar sign
    if (sentence.isEmpty()) {
        return;
    }
    if (sentence[0] != QStringLiteral("$")) {
        return;
    }
    sentence = sentence.mid(1);

    // Check the NMEA checksum
    auto pieces = sentence.split(QStringLiteral("*"));
    if (pieces.length() != 2) {
        return;
    }
    sentence = pieces[0];
    auto checksum = pieces[1].toInt(nullptr, 16);
    quint8 myChecksum = 0;
    for(auto && i : sentence) {
        myChecksum ^= static_cast<quint8>(i.toLatin1());
    }
    if (checksum != myChecksum) {
        return;
    }

    // Split the message into pieces
    auto arguments = sentence.split(QStringLiteral(","));
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
            m_trueAltitude = {};
            m_trueAltitudeFOM = {};
            m_trueAltitudeTimer.stop();
            return;
        }

        m_trueAltitude = Units::Distance::fromM(alt);
        m_trueAltitudeFOM = {};
        m_trueAltitudeTimer.start();
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
        bool ok1 = false;
        bool ok2 = false;
        auto lat = arguments[2].leftRef(2).toDouble(&ok1) + arguments[2].midRef(2).toDouble(&ok2)/60.0;
        if (!ok1 || !ok2) {
            return;
        }
        if (arguments[3] == u"S") {
            lat *= -1.0;
        }

        auto lon = arguments[4].leftRef(3).toDouble(&ok1) + arguments[4].midRef(3).toDouble(&ok2)/60.0;
        if (!ok1 || !ok2) {
            return;
        }
        if (arguments[5] == u"W") {
            lon *= -1.0;
        }

        QGeoCoordinate coordinate(lat, lon);
        if (!coordinate.isValid()) {
            return;
        }
        if (m_trueAltitudeTimer.isActive()) {
            coordinate.setAltitude(m_trueAltitude.toM());
        }
        QGeoPositionInfo pInfo(coordinate, QDateTime::currentDateTimeUtc());

        // Ground speed
        bool ok = false;
        auto groundSpeed = Units::Speed::fromKN(arguments[6].toDouble(&ok));
        if (!ok) {
            groundSpeed = Units::Speed::fromKN(qQNaN());
        }
        if (groundSpeed.isFinite()) {
            pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, groundSpeed.toMPS() );
        }

        // Track
        auto TT = arguments[7].toDouble(&ok);
        if (!ok) {
            TT = qQNaN();
        }
        if (TT != qQNaN()) {
            pInfo.setAttribute(QGeoPositionInfo::Direction, TT );
        }

        emit positionUpdated( Positioning::PositionInfo(pInfo));
        return;
    }

    // Data on other proximate aircraft
    if (messageType == u"PFLAA") {

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
        // Vertical distance is optional
        auto vDist = Units::Distance::fromM(arguments[3].toDouble(&ok));
        if (!ok) {
            vDist = Units::Distance::fromM(qQNaN());
        }

        // Target type is optional
        Traffic::TrafficFactor_Abstract::AircraftType type = Traffic::TrafficFactor_Abstract::unknown;
        {
            auto targetType = arguments[10];
            if (targetType == u"1") {
                type = Traffic::TrafficFactor_Abstract::Glider;
            }
            if (targetType == u"2") {
                type = Traffic::TrafficFactor_Abstract::TowPlane;
            }
            if (targetType == u"3") {
                type = Traffic::TrafficFactor_Abstract::Copter;
            }
            if (targetType == u"4") {
                type = Traffic::TrafficFactor_Abstract::Skydiver;
            }
            if (targetType == u"5") {
                type = Traffic::TrafficFactor_Abstract::Aircraft;
            }
            if (targetType == u"6") {
                type = Traffic::TrafficFactor_Abstract::HangGlider;
            }
            if (targetType == u"7") {
                type = Traffic::TrafficFactor_Abstract::Paraglider;
            }
            if (targetType == u"8") {
                type = Traffic::TrafficFactor_Abstract::Aircraft;
            }
            if (targetType == u"9") {
                type = Traffic::TrafficFactor_Abstract::Jet;
            }
            if (targetType == u"B") {
                type = Traffic::TrafficFactor_Abstract::Balloon;
            }
            if (targetType == u"C") {
                type = Traffic::TrafficFactor_Abstract::Airship;
            }
            if (targetType == u"D") {
                type = Traffic::TrafficFactor_Abstract::Drone;
            }
            if (targetType == u"F") {
                type = Traffic::TrafficFactor_Abstract::StaticObstacle;
            }
        }

        // Ground speed it optimal. If ground speed is zero that means:
        // target is on the ground. Ignore these targets, unless they are known static obstacles!
        auto groundSpeedInMPS = arguments[8].toDouble(&ok);
        if (!ok) {
            groundSpeedInMPS = qQNaN();
        }
        if ((groundSpeedInMPS == 0.0) && (type != Traffic::TrafficFactor_Abstract::StaticObstacle)) {
            return;
        }


        // Target ID is optional
        auto targetID = arguments[5];


        //
        // Handle non-directional targets
        //
        if (arguments[2] == u"") {
            // Horizontal distance is mandatory
            auto hDist = Units::Distance::fromM(arguments[1].toDouble(&ok));
            if (!ok) {
                return;
            }

            // Construct a PositionInfo object that contains additional information (such as ground speed, if available)
            QGeoPositionInfo pInfo(QGeoCoordinate(), QDateTime::currentDateTimeUtc());
            auto targetGS = arguments[8].toDouble(&ok);
            if (ok) {
                pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, targetGS);
            }
            auto targetVS = arguments[9].toDouble(&ok);
            if (ok) {
                pInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, targetVS);
            }

            m_factorDistanceOnly.setAlarmLevel(alarmLevel);
            m_factorDistanceOnly.setCallSign( GlobalObject::flarmnetDB()->getRegistration(targetID) );
            m_factorDistanceOnly.setCoordinate(Positioning::PositionProvider::lastValidCoordinate());
            m_factorDistanceOnly.setID(targetID);
            m_factorDistanceOnly.setHDist(hDist);
            m_factorDistanceOnly.setType(type);
            m_factorDistanceOnly.setVDist(vDist);
            m_factorDistanceOnly.startLiveTime();
            emit factorWithoutPosition(m_factorDistanceOnly);
            return;
        }

        //
        // From now on, we assume that we have a directional target
        //

        // As a first step, we obtain the target's coordinate. We take our own coordinate as a starting point.
        auto targetCoordinate = Positioning::PositionProvider::lastValidCoordinate();
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
        if (vDist.isFinite()) {
            targetCoordinate = targetCoordinate.atDistanceAndAzimuth(0, 0, vDist.toM());
        }
        auto hDist = Units::Distance::fromM(sqrt(relativeNorth*relativeNorth+relativeEast*relativeEast));

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
        m_factor.setAlarmLevel(alarmLevel);
        m_factor.setCallSign( GlobalObject::flarmnetDB()->getRegistration(targetID) );
        m_factor.setHDist(hDist);
        m_factor.setID(targetID);
        m_factor.setPositionInfo(pInfo);
        m_factor.setType(type);
        m_factor.setVDist(vDist);
        m_factor.startLiveTime();
        emit factorWithPosition(m_factor);
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
        if ((severity == u"2") || (severity == u"3")) {
            setTrafficReceiverSelfTestError(result);
        }
        return;
    }

    // Debug Information -- Ignore
    if (messageType == u"PFLAS") {
        return;
    }

    // FLARM Heartbeat
    if (messageType == u"PFLAU") {
        // Heartbeat received.
        setReceivingHeartbeat(true);

        if (arguments.length() < 9) {
            return;
        }

        // Handle runtime errors
        QStringList results;
        // auto RX = arguments[0];
        auto TX = arguments[1];
        if (TX == QLatin1String("0")) {
            results += tr("No FLARM transmission");
        }
        auto GPS = arguments[2];
        if (GPS == QLatin1String("0")) {
            results += tr("No GPS reception");
        }
        auto Power = arguments[3];
        if (Power == QLatin1String("0")) {
            results += tr("Under- or Overvoltage");
        }
        setTrafficReceiverRuntimeError(results.join(QStringLiteral(" • ")));

        auto AlarmLevel = arguments[4];
        auto RelativeBearing = arguments[5];
        auto AlarmType = arguments[6];
        auto RelativeVertical = arguments[7];
        auto RelativeDistance = arguments[8];

        auto wrning = Traffic::Warning(AlarmLevel, RelativeBearing, AlarmType, RelativeVertical, RelativeDistance);
        emit warning(wrning);

        return;
    }

    // Version information
    if (messageType == u"PFLAV") {
        if (arguments.length() < 4) {
            return;
        }

        emit trafficReceiverHwVersion(arguments[1]);
        emit trafficReceiverSwVersion(arguments[2]);
        emit trafficReceiverObVersion(arguments[3]);


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
        auto barometricAlt = Units::Distance::fromFT(arguments[0].toDouble(&ok));
        if (!ok) {
            return;
        }
        if (!barometricAlt.isFinite()) {
            return;
        }

        emit pressureAltitudeUpdated(barometricAlt);
        return;
    }
}
