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

#include <QGeoCoordinate>
#include <QStringView>

#include "TrafficFactorAircraftType.h"
#include "units/Angle.h"
#include "units/Speed.h"

using namespace Qt::Literals::StringLiterals;

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
*
* This class is used in the UnitTest. It should not have external dependencies like GlobalObject.
*/
class TrafficDataSource_OgnParser {
public:
    static void parseAprsisMessage(OgnMessage& ognMessage);
    static QString formatLoginString(QStringView callSign,
                                     const QGeoCoordinate &receiveLocation,
                                     unsigned int receiveRadius,
                                     QStringView appName,
                                     QStringView appVersion);
    static QString formatPositionReport(QStringView callSign,
                                        const QGeoCoordinate &coordinate,
                                        double course,
                                        double speed,
                                        double altitude,
                                        Traffic::AircraftType aircraftType);

private:
    static QString formatFilter(const QGeoCoordinate &receiveLocation, unsigned int receiveRadius);
    static QString formatLatitude(double latitude);
    static QString formatLongitude(double longitude);
    static QString calculatePassword(QStringView callSign);
    static double decodeLatitude(QStringView nmeaLatitude, QChar latitudeDirection, QChar latEnhancement);
    static double decodeLongitude(QStringView nmeaLongitude, QChar longitudeDirection, QChar lonEnhancement);
    static void parseTrafficReport(OgnMessage &ognMessage, QStringView header, QStringView body);
    static void parseCommentMessage(OgnMessage& ognMessage);
    static void parseStatusMessage(OgnMessage &ognMessage, QStringView header, QStringView body);
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
    QString sentence;           // e.g. "FLRDDE626>APRS,qAS,EGHL:/074548h5111.32N/00102.04W'086/007/A=000607 id0ADDE626 -019fpm +0.0rot 5.5dB 3e -4.3kHz"
    OgnMessageType type = OgnMessageType::UNKNOWN; // e.g. OgnMessageType::TRAFFIC_REPORT

    QStringView sourceId;       // like ENROUTE12345
    QStringView timestamp;      // hhmmss
    QGeoCoordinate coordinate; 
    OgnSymbol symbol = OgnSymbol::UNKNOWN; // the symbol that should be shown on the map, typically Aircraft.

    Units::Angle course;        // course
    Units::Speed speed;         // speed
    QStringView aircraftID;     // aircraft ID, e.g. "id0ADDE626"
    double verticalSpeed = {};  // in m/s
    QStringView rotationRate;   // like "+0.0rot"
    QStringView signalStrength; // like "5.5dB"
    QStringView errorCount;     // like "3e"
    QStringView frequencyOffset;// like "-4.3kHz"
    QStringView squawk;         // like "sq2244"
    QStringView flightlevel;    // like "FL350.00"
    QStringView flightnumber;   // Flight number, e.g., "DLH2AV" or "SRR6119"
    QStringView gpsInfo;        // like "gps:0.0"
    Traffic::AircraftType aircraftType = Traffic::AircraftType::unknown; // e.g., "Glider", "Tow Plane", etc.
    OgnAddressType addressType = OgnAddressType::UNKNOWN; // e.g., "ICAO", "FLARM", "OGN Tracker"
    QStringView address;        // like "4D21C2"
    bool stealthMode = false;   // true if the aircraft shall be hidden
    bool noTrackingFlag = false;// true if the aircraft shall not be tracked

    uint32_t wind_direction = {};  // degree 0..359
    uint32_t wind_speed = {};      // m/s
    uint32_t wind_gust_speed = {}; // m/s
    uint32_t temperature = {};     // degree C
    uint32_t humidity = {};        // percent
    double pressure = {};          // hPa

    void reset()
    {
        sentence.clear();
        type = OgnMessageType::UNKNOWN;
        sourceId.truncate(0);       
        timestamp.truncate(0);      
        coordinate = QGeoCoordinate();
        symbol = OgnSymbol::UNKNOWN;
        course = {};
        speed = {};
        aircraftID.truncate(0);     
        verticalSpeed = 0.0;
        rotationRate.truncate(0);   
        signalStrength.truncate(0); 
        errorCount.truncate(0);     
        frequencyOffset.truncate(0);
        squawk.truncate(0);         
        flightlevel.truncate(0);    
        flightnumber.truncate(0);   
        gpsInfo.truncate(0);        
        aircraftType = Traffic::AircraftType::unknown;
        addressType = OgnAddressType::UNKNOWN;
        address.truncate(0);        
        stealthMode = false;
        noTrackingFlag = false;
        wind_direction = 0;
        wind_speed = 0;
        wind_gust_speed = 0;
        temperature = 0;
        humidity = 0;
        pressure = 0.0;
    }
};
}
