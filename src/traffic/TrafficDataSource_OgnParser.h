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
* \see https://github.com/svoop/ogn_client-ruby/wiki/SenderBeacon
*/
class TrafficDataSource_OgnParser {
    friend class TrafficDataSource_OgnTest;
public:
    static OgnMessage parseAprsisMessage(const QString& message);

    // New static helper functions for formatting
    static QString formatPositionReport(const QString& callSign, const QGeoCoordinate& coordinate, double course, double speed, double altitude);
    static QString formatLatitude(double latitude);
    static QString formatLongitude(double longitude);

    // New helper functions for decoding
    static double decodeLatitude(const QStringView& nmeaLatitude, QChar latitudeDirection);
    static double decodeLongitude(const QStringView& nmeaLongitude, QChar longitudeDirection);

private:
    static OgnMessage parseTrafficReport(const QStringView& header, const QStringView& body);
    static OgnMessage parseCommentMessage(const QString& message);
    static OgnMessage parseStatusMessage(const QStringView& header, const QStringView& body);
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
    QStringView verticalSpeed;
    QStringView rotationRate;
    QStringView signalStrength;
    QStringView errorCount;
    QStringView frequencyOffset;
    Traffic::AircraftType aircraftType = Traffic::AircraftType::unknown;
    OgnAddressType addressType = OgnAddressType::UNKNOWN;
    uint32_t address = {};
    bool stealthMode = false;
    bool noTrackingFlag = false;

    uint32_t wind_direction = {};
    uint32_t wind_speed = {};
    uint32_t wind_gust_speed = {};
    uint32_t temperature = {};
    uint32_t humidity = {};
    double pressure = {};
};
}
