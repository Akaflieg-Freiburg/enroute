/***************************************************************************
 *   Copyright (C) 2021-2025 by Stefan Kebekus                             *
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

#pragma once

#include <QString>
#include <QStringView>
#include <QGeoCoordinate>
#include "TrafficFactorAircraftType.h"
using namespace Qt::Literals::StringLiterals;

// Forward declaration for the test class
class TrafficDataSource_OgnTest;

namespace Traffic::Ogn {
Q_NAMESPACE
struct OgnMessage;

/*! \brief Parser for OGN glidernet.org traffic receiver.
* 
* sentences similar to NMEA like the following: 
* 
*   FLRDDE626>APRS,qAS,EGHL:/074548h5111.32N/00102.04W'086/007/A=000607 id0ADDE626 -019fpm +0.0rot 5.5dB 3e -4.3kHz
* 
* \see http://wiki.glidernet.org/wiki:subscribe-to-ogn-data
* \see http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
* \see http://wiki.glidernet.org/aprs-interaction-examples
* \see https://github.com/svoop/ogn_client-ruby/wiki/SenderBeacon
*/
class TrafficDataSource_OgnParser {
    friend class TrafficDataSource_OgnTest;
public:
    static OgnMessage parseAprsisMessage(const QString& message);
    static QString formatLoginString(const QStringView callSign, const QGeoCoordinate& receiveLocation, const unsigned int receiveRadius, const QStringView appName, const QStringView appVersion);
    static QString formatPositionReport(const QStringView callSign, const QGeoCoordinate& coordinate, double course, double speed, double altitude, Traffic::AircraftType aircraftType);
    static QString formatFilterCommand(const QGeoCoordinate& receiveLocation, const unsigned int receiveRadius);

private:
    static QString formatFilter(const QGeoCoordinate& receiveLocation, const unsigned int receiveRadius);
    static QString formatLatitude(double latitude);
    static QString formatLongitude(double longitude);
    static QString calculatePassword(const QStringView callSign);
    static double decodeLatitude(const QStringView nmeaLatitude, QChar latitudeDirection);
    static double decodeLongitude(const QStringView nmeaLongitude, QChar longitudeDirection);
    static OgnMessage parseTrafficReport(const QStringView header, const QStringView body);
    static OgnMessage parseCommentMessage(const QStringView message);
    static OgnMessage parseStatusMessage(const QStringView header, const QStringView body);
};

enum class OgnMessageType
{
    UNKNOWN,
    TRAFFIC_REPORT,
    COMMENT,
    STATUS,
    WEATHER,
};
Q_ENUM_NS(OgnMessageType);

// see http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
enum class OgnAddressType
{ 
    UNKNOWN = 0,
    ICAO = 1,
    FLARM = 2,
    OGN_TRACKER = 3,
};
Q_ENUM_NS(OgnAddressType);

enum class OgnSymbol
{ 
    UNKNOWN,
    GLIDER,
    HELICOPTER,
    PARACHUTE,
    AIRCRAFT,
    JET,
    BALLOON,
    STATIC_OBJECT,
    UAV,
    WEATHERSTATION,
};
Q_ENUM_NS(OgnSymbol);

struct OgnMessage
{
    QString sentence;
    OgnMessageType type = OgnMessageType::UNKNOWN;

    QStringView sourceId;
    QStringView timestamp;
    QGeoCoordinate coordinate;
    OgnSymbol symbol = OgnSymbol::UNKNOWN;

    QStringView course;
    QStringView speed;
    QStringView aircraftID;
    double verticalSpeed = {}; // m/s
    QStringView rotationRate;
    QStringView signalStrength;
    QStringView errorCount;
    QStringView frequencyOffset;
    QStringView squawk;
    QStringView flightlevel;
    QStringView flightnumber; // Flight number, e.g., "DLH2AV" or "SRR6119"
    QStringView gpsInfo;
    Traffic::AircraftType aircraftType = Traffic::AircraftType::unknown;
    OgnAddressType addressType = OgnAddressType::UNKNOWN;
    QStringView address;
    bool stealthMode = false;
    bool noTrackingFlag = false;

    uint32_t wind_direction = {};  // degree 0..359
    uint32_t wind_speed = {};
    uint32_t wind_gust_speed = {}; 
    uint32_t temperature = {};     // degree C
    uint32_t humidity = {};        // percent
    double pressure = {};
};
}
