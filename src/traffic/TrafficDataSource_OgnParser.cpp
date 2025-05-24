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

#include "TrafficDataSource_OgnParser.h"
#include "TrafficFactorAircraftType.h"
#include "units/Distance.h"
#include "units/Speed.h"

#include <QDebug>
#include <QMetaEnum>
#include <QRegularExpression>

#define OGN_DEBUG 1

using namespace Qt::Literals::StringLiterals;

namespace {

// see http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
using ATMap = QMap<QString, Traffic::AircraftType>; // Necessary because Q_GLOBAL_STATIC does not like templates
Q_GLOBAL_STATIC(ATMap, AircraftTypeMap,
                {
                    {"/z",  Traffic::AircraftType::unknown},         // Unknown
                    {"/'",  Traffic::AircraftType::Glider},          // Glider
                    {"/X",  Traffic::AircraftType::Copter},          // Helicopter
                    {"/g",  Traffic::AircraftType::Paraglider},      // Parachute, Hang Glider, Paraglider
                    {"\\^", Traffic::AircraftType::Aircraft},        // Drop Plane, Powered Aircraft
                    {"/^",  Traffic::AircraftType::Jet},             // Jet Aircraft
                    {"/O",  Traffic::AircraftType::Balloon},         // Balloon, Airship
                    {"\\n", Traffic::AircraftType::StaticObstacle},  // Static Object
                });

using ASMap = QMap<QString, Traffic::Ogn::OgnSymbol>; // Necessary because Q_GLOBAL_STATIC does not like templates
Q_GLOBAL_STATIC(ASMap, AprsSymbolMap,
                {
                    {"/z",  Traffic::Ogn::OgnSymbol::UNKNOWN},        // Unknown
                    {"/'",  Traffic::Ogn::OgnSymbol::GLIDER},         // Glider
                    {"/X",  Traffic::Ogn::OgnSymbol::HELICOPTER},     // Helicopter
                    {"/g",  Traffic::Ogn::OgnSymbol::PARACHUTE},      // Parachute, Hang Glider, Paraglider
                    {"\\^", Traffic::Ogn::OgnSymbol::AIRCRAFT},       // Drop Plane, Powered Aircraft
                    {"/^",  Traffic::Ogn::OgnSymbol::JET},            // Jet Aircraft
                    {"/O",  Traffic::Ogn::OgnSymbol::BALLOON},        // Balloon, Airship
                    {"\\n", Traffic::Ogn::OgnSymbol::STATIC_OBJECT},  // Static Object
                    {"/_",  Traffic::Ogn::OgnSymbol::WEATHERSTATION},  // WeatherStation
                });

// see http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
using ACMap = QMap<uint32_t, Traffic::AircraftType>;
Q_GLOBAL_STATIC(ACMap, AircraftCategoryMap,
                {
                    {0x0, Traffic::AircraftType::unknown},         // Reserved
                    {0x1, Traffic::AircraftType::Glider},          // Glider/Motor Glider/TMG
                    {0x2, Traffic::AircraftType::TowPlane},        // Tow Plane/Tug Plane
                    {0x3, Traffic::AircraftType::Copter},          // Helicopter/Gyrocopter/Rotorcraft
                    {0x4, Traffic::AircraftType::Skydiver},        // Skydiver/Parachute
                    {0x5, Traffic::AircraftType::Aircraft},        // Drop Plane for Skydivers
                    {0x6, Traffic::AircraftType::HangGlider},      // Hang Glider (hard)
                    {0x7, Traffic::AircraftType::Paraglider},      // Paraglider (soft)
                    {0x8, Traffic::AircraftType::Aircraft},        // Aircraft with reciprocating engine(s)
                    {0x9, Traffic::AircraftType::Jet},             // Aircraft with jet/turboprop engine(s)
                    {0xA, Traffic::AircraftType::unknown},         // Unknown
                    {0xB, Traffic::AircraftType::Balloon},         // Balloon (hot, gas, weather, static)
                    {0xC, Traffic::AircraftType::Airship},         // Airship/Blimp/Zeppelin
                    {0xD, Traffic::AircraftType::Drone},           // UAV/RPAS/Drone
                    {0xE, Traffic::AircraftType::unknown},         // Reserved
                    {0xF, Traffic::AircraftType::StaticObstacle}   // Static Obstacle
                });

} // namespace

namespace Traffic::Ogn {


void TrafficDataSource_OgnParser::parseAprsisMessage(OgnMessage& ognMessage)
{
    if(ognMessage.type != OgnMessageType::UNKNOWN)
    {
        qDebug() << ognMessage.type;
    }
    Q_ASSERT(ognMessage.type == OgnMessageType::UNKNOWN); // Expect that data Structure OgnMessage is reset or initialized to default values.

    const QStringView sentence(ognMessage.sentence);
    if (sentence.startsWith(u"#"_s))
    {
        // Comment message  
        parseCommentMessage(ognMessage);
        return;
    }

    // Split the sentence into header and body at the first colon
    auto const colonIndex = sentence.indexOf(u':');
    if (colonIndex == -1)
    {
#if OGN_DEBUG
        qDebug() << "Invalid message format:" << sentence;
#endif
        ognMessage.type = OgnMessageType::UNKNOWN;
        return;
    }

    // This function runs all the time, so it is performance critical.
    // I try to avoid heap allocations and use a lot of QStringView.
    QStringView const header = sentence.sliced(0, colonIndex);
    QStringView const body = sentence.sliced(colonIndex + 1);

    // Check if header and body are valid
    if (header.size() < 5 || body.size() < 5)
    {
#if OGN_DEBUG
        qDebug() << "Invalid message header or body:" << sentence;
#endif
        ognMessage.type = OgnMessageType::UNKNOWN;
        return;
    }

    // Determine the type of message based on the first character in the body
    if (body.startsWith(u"/"_s))
    {
        // "/" indicates a Traffic Report
        parseTrafficReport(ognMessage, header, body);
        return;
    }
    if (body.startsWith(u">"_s))
    {
        // ">" indicates a Receiver Status
        parseStatusMessage(ognMessage, header, body);
        return;
    }

    ognMessage.type = OgnMessageType::UNKNOWN;
#if OGN_DEBUG
    qDebug() << "Unknown message type:" << sentence;
#endif
    return;
}


double TrafficDataSource_OgnParser::decodeLatitude(const QStringView nmeaLatitude, QChar latitudeDirection, QChar latEnhancement)
{
    // e.g. "5111.32"
    if (nmeaLatitude.size() < 7) {
        qDebug() << "invalid input";
        return qQNaN(); // Invalid input
    }

    bool ok = false;

    // Parse degrees (first 2 characters)
    double latitudeDegrees = nmeaLatitude.left(2).toDouble(&ok);
    if (!ok) {
        qDebug() << nmeaLatitude << "decodeLatitude toDouble failed 1" << nmeaLatitude.left(2);
        return qQNaN();
    }

    // Parse minutes (remaining characters after the first 2)
    double latitudeMinutes = nmeaLatitude.mid(2).toDouble(&ok);
    if (!ok) {
        qDebug() << "decodeLatitude toDouble failed 2";
        return qQNaN();
    }

    // Combine degrees and minutes
    double latitude = latitudeDegrees + (latitudeMinutes / 60.0);

    // Apply precision enhancement
    if(latEnhancement != 0) {
        latitude += static_cast<double>(latEnhancement.digitValue()) * 0.001 / 60;
    }

    // Adjust for direction (South is negative)
    if (latitudeDirection == 'S') {
        latitude = -latitude;
    }

    return latitude;
}

double TrafficDataSource_OgnParser::decodeLongitude(const QStringView nmeaLongitude, QChar longitudeDirection, QChar lonEnhancement)
{
    // e.g. "00102.04W"
    if (nmeaLongitude.size() < 8) {
        qDebug() << "lon invalid input";
        return qQNaN(); // Invalid input
    }

    bool ok = false;

    // Parse degrees (first 3 characters)
    double longitudeDegrees = nmeaLongitude.left(3).toDouble(&ok);
    if (!ok) {
        qDebug() << nmeaLongitude << "lon toDouble failed 1" << nmeaLongitude.left(2);
        return qQNaN();
    }

    // Parse minutes (remaining characters after the first 3)
    double longitudeMinutes = nmeaLongitude.mid(3).toDouble(&ok);
    if (!ok) {
        qDebug() << nmeaLongitude << "lon toDouble failed 2" << nmeaLongitude.left(2);
        return qQNaN();
    }

    // Combine degrees and minutes
    double longitude = longitudeDegrees + (longitudeMinutes / 60.0);

    // Apply precision enhancement
    if(lonEnhancement != 0) {
        longitude += static_cast<double>(lonEnhancement.digitValue()) * 0.001 / 60;
    }

    // Adjust for direction (West is negative)
    if (longitudeDirection == 'W') {
        longitude = -longitude;
    }

    return longitude;
}

void TrafficDataSource_OgnParser::parseTrafficReport(OgnMessage& ognMessage, const QStringView header, const QStringView body)
{
    // e.g. header = "FLRDDE626>APRS,qAS,EGHL:"
    // e.g. body = "/074548h5111.32N/00102.04W'086/007/A=000607 id0ADDE626 -019fpm +0.0rot 5.5dB 3e -4.3kHz" (traffic report)
    // e.g. body = "/001140h4741.90N/01104.20E^/A=034868 !W91! id254D21C2 +128fpm FL350.00 A3:AXY547M Sq2244" (traffic report)
    // e.g. body = "/222245h4803.92N/00800.93E_292/005g010t030h00b65526 5.2dB" (weather report, but starting with '/' like a traffic report)
    #if OGN_DEBUG
    qDebug() << "Traffic Report:" << header << ":" << body;
    #endif

    // Check if the body starts with a '/'
    // This is a requirement for the Traffic Report format.
    if(body.at(0) != u'/') {
        #if OGN_DEBUG
        qDebug() << "Invalid body format in Traffic Report: " << header << ":" << body;
        #endif
        ognMessage.type = OgnMessageType::UNKNOWN;
        return;
    }

    // Parse the Header
    int index = header.indexOf(u'>');
    if (index == -1) {
        #if OGN_DEBUG
        qDebug() << "Invalid header format in Traffic Report: " << header << ":" << body;
        #endif
        ognMessage.type = OgnMessageType::UNKNOWN;
        return;
    }
    ognMessage.type = OgnMessageType::TRAFFIC_REPORT;
    ognMessage.sourceId = header.left(index);

    // Parse the body
    int blankIndex = body.indexOf(u' ');
    QStringView aprsPart;
    QStringView ognPart;
    if (blankIndex == -1) {
        aprsPart = body;
    } else {
        aprsPart = QStringView(body.constData(), blankIndex); // APRS Part before the first blank
        ognPart = QStringView(body.constData() + blankIndex + 1, body.size() - blankIndex - 1); // OGN Part after the first blank
    }

    // Parse aprsPart
    if (!aprsPart.startsWith(u"/") || aprsPart.size() < 30) {
        ognMessage.type = OgnMessageType::UNKNOWN;
        return;
    }

    // Parse timestamp
    ognMessage.timestamp = aprsPart.mid(1, 6);

    // Parse coordinates
    {
        // latitude
        QStringView latString = aprsPart.mid(8, 7); // "4741.90"
        QChar latDirection = aprsPart.at(15);      // "N" or "S"
        // longitude
        QStringView lonString = aprsPart.mid(17, 8); // "01104.20"
        QChar lonDirection = aprsPart.at(25);       // "E" or "W"
        // optional precision enhancement, e.g. "!W91"
        QChar latEnhancement = {};
        QChar lonEnhancement = {};
        int precisionIndex = body.indexOf(u"!W");
        if (precisionIndex != -1 && body.size() > precisionIndex + 4) {
            latEnhancement = body.at(precisionIndex + 2);
            lonEnhancement = body.at(precisionIndex + 3);
        }
        // decode
        double latitude = decodeLatitude(latString, latDirection, latEnhancement);
        double longitude = decodeLongitude(lonString, lonDirection, lonEnhancement);
        ognMessage.coordinate.setLatitude(latitude);
        ognMessage.coordinate.setLongitude(longitude);
    }


    // Parse symbol
    QChar symbolTable = aprsPart.at(16);
    QChar symbolCode = aprsPart.at(26);
    ognMessage.symbol = AprsSymbolMap->value(QString(symbolTable) + symbolCode, OgnSymbol::UNKNOWN);

    // If the weather report is detected (e.g. an underscore appears after the longitude)
    if(ognMessage.symbol == OgnSymbol::WEATHERSTATION) {
        ognMessage.type = OgnMessageType::WEATHER;
        int underscoreIndex = 26;
        // Decode wind direction: next 3 digits after the underscore
        QStringView windDirStr = aprsPart.mid(underscoreIndex + 1, 3);
        ognMessage.wind_direction = windDirStr.toUInt();
        // Find the slash following the wind direction to decode wind speed
        int slashAfterUnderscore = aprsPart.indexOf(u"/", underscoreIndex);
        if (slashAfterUnderscore != -1) {
            QStringView windSpeedStr = aprsPart.mid(slashAfterUnderscore + 1, 3);
            ognMessage.wind_speed = windSpeedStr.toUInt();
        }
        // Decode wind gust speed: look for 'g'
        int gIndex = aprsPart.indexOf(u"g", underscoreIndex);
        if (gIndex != -1) {
            QStringView gustStr = aprsPart.mid(gIndex + 1, 3);
            ognMessage.wind_gust_speed = gustStr.toUInt();
        }
        // Decode temperature: look for 't'
        int tIndex = aprsPart.indexOf(u"t", underscoreIndex);
        if (tIndex != -1) {
            QStringView tempStr = aprsPart.mid(tIndex + 1, 3);
            ognMessage.temperature = tempStr.toDouble();
        }
        // Decode humidity: look for 'h'
        int hIndex = aprsPart.indexOf(u"h", underscoreIndex);
        if (hIndex != -1) {
            QStringView humStr = aprsPart.mid(hIndex + 1, 2);
            ognMessage.humidity = humStr.toUInt();
        }
        // Decode pressure: look for 'b'
        int bIndex = aprsPart.indexOf(u"b", underscoreIndex);
        if (bIndex != -1) {
            QStringView presStr = aprsPart.mid(bIndex + 1);
            double pressure = presStr.toDouble(); // tenths of hectopascal.
            ognMessage.pressure = pressure / 10.0;
        }
    } else {
        // Parse course, speed
        if (aprsPart.size() >= 34 && aprsPart.at(30) == u'/') {
            ognMessage.course = Units::Angle::fromDEG(aprsPart.mid(27, 3).toInt());
            ognMessage.speed = Units::Speed::fromKN(aprsPart.mid(31, 3).toDouble());
        }
        // Parse altitude
        int altitudeIndex = aprsPart.indexOf(QStringView(u"/A="));
        if (altitudeIndex != -1) {
            int altStart = altitudeIndex + 3;
            QStringView altitudeStr = aprsPart.mid(altStart, 6);
            double altitudeFeet = altitudeStr.toDouble();
            ognMessage.coordinate.setAltitude(Units::Distance::fromFT(altitudeFeet).toM());
        }
    }

    // Parse ognPart
    if (!ognPart.isEmpty() && !ognPart.trimmed().isEmpty()) {
        auto it = ognPart.cbegin();
        while (it != ognPart.cend()) {
            auto end = std::find(it, ognPart.cend(), u' ');
            QStringView item(it, end);

            if (item.startsWith(u"id")) {
                ognMessage.aircraftID = item.mid(2);
            } else if (item.startsWith(u"t")) {
                ognMessage.temperature = item.mid(1).toDouble();
            } else if (item.startsWith(u"h")) {
                ognMessage.humidity = item.mid(1).toUInt();
            } else if (item.startsWith(u"b")) {
                ognMessage.pressure = item.mid(1).toDouble() / 10.0; // Convert to hPa
            } else if (item.endsWith(u"fpm")) {
                ognMessage.verticalSpeed = Units::Speed::fromFPM(item.mid(0, item.indexOf(u'f')).toDouble()).toMPS();
            } else if (item.endsWith(u"rot")) {
                ognMessage.rotationRate = item;
            } else if (item.endsWith(u"dB")) {
                ognMessage.signalStrength = item;
            } else if (item.endsWith(u"e")) {
                ognMessage.errorCount = item;
            } else if (item.endsWith(u"kHz")) {
                ognMessage.frequencyOffset = item;
            } else if (item.startsWith(u"FL")) {
                ognMessage.flightlevel = item;
            } else if (item.startsWith(u"A") && item[2] == u':') {
                ognMessage.flightnumber = item.mid(item.indexOf(u':') + 1);
            } else if (item.startsWith(u"Sq")) {
                ognMessage.squawk = item.mid(2);
            } else if (item.startsWith(u"gps:")) {
                ognMessage.gpsInfo = item.mid(4);
            }

            it = (end != ognPart.cend()) ? end + 1 : ognPart.cend();
        }
    }

    // Parse aircraft type, address type, and address
    if (!ognMessage.aircraftID.isEmpty()) {
        bool ok = false;
        uint32_t hexcode = ognMessage.aircraftID.toUInt(&ok, 16);
        if (!ok) {
            #if OGN_DEBUG
            qDebug() << "Failed to parse aircraft ID as hex:" << ognMessage.aircraftID;
            #endif
        } else {
            ognMessage.stealthMode = hexcode & 0x80000000;
            ognMessage.noTrackingFlag = hexcode & 0x40000000;
            uint32_t aircraftCategory = ((hexcode >> 26) & 0xF);
            Traffic::AircraftType aircraftTypeEnumValue = AircraftCategoryMap->value(aircraftCategory, Traffic::AircraftType::unknown);
            ognMessage.aircraftType = aircraftTypeEnumValue;
            uint32_t addressTypeValue = (hexcode >> 24) & 0x3;
            ognMessage.addressType = static_cast<OgnAddressType>(addressTypeValue);
            ognMessage.address = QStringView(ognMessage.aircraftID.cbegin()+2, 6);
        }
    }

    #if OGN_DEBUG
    qDebug() << "Parsed Traffic Report: " << ognMessage.coordinate.latitude() << " " << ognMessage.coordinate.longitude()
             << " course:" << ognMessage.course.toDEG() << "° speed:" << ognMessage.speed.toKN() << "kts"
             << " altitude:" << Units::Distance::fromM(ognMessage.coordinate.altitude()).toFeet() << " wind:" << ognMessage.wind_direction << "/" << ognMessage.wind_speed
             << " temperature:" << ognMessage.temperature << "°C pressure:" << ognMessage.pressure << "hPa";
    #endif
}

void TrafficDataSource_OgnParser::parseCommentMessage(OgnMessage& ognMessage)
{
    ognMessage.type = OgnMessageType::COMMENT;
}

void TrafficDataSource_OgnParser::parseStatusMessage(OgnMessage &ognMessage,
                                                     const QStringView /*header*/,
                                                     const QStringView /*body*/)
{
    ognMessage.type = OgnMessageType::STATUS;
}

QString TrafficDataSource_OgnParser::formatPositionReport(const QStringView callSign,
                                                          const QGeoCoordinate &coordinate,
                                                          double course,
                                                          double speed,
                                                          double altitude,
                                                          Traffic::AircraftType aircraftType)
{
    // e.g. "ENR12345>APRS,TCPIP*: /074548h5111.32N/00102.04W'086/007/A=000607"

    // Static cache for the last lookup
    static Traffic::AircraftType lastAircraftType = Traffic::AircraftType::unknown;
    static QString lastSymbol = "/z"; // Default to unknown symbol

    // Check if the cached value matches the current aircraft type
    if (lastAircraftType != aircraftType)
    {
        // Perform reverse lookup
        lastSymbol.clear();
        for (auto it = AircraftTypeMap->cbegin(); it != AircraftTypeMap->cend(); ++it)
        {
            if (it.value() == aircraftType)
            {
                lastSymbol = it.key();
                break;
            }
        }

        // Default to unknown symbol if no match is found
        if (lastSymbol.isEmpty())
        {
            lastSymbol = "\\^";
        }

        // Update the cache
        lastAircraftType = aircraftType;
    }

    return QString("%1>APRS,TCPIP*: /%2h%3%4%5%6%7/%8/A=%9\n")
        .arg(callSign.toString(),
             QDateTime::currentDateTimeUtc().toString("hhmmss"),
             formatLatitude(coordinate.latitude()))   // Latitude
        .arg(lastSymbol[0])                           // Symbol table
        .arg(formatLongitude(coordinate.longitude())) // Longitude
        .arg(lastSymbol[1])                           // Symbol code
        .arg(QString::number(course, 'f', 0).rightJustified(3, '0'),
             QString::number(speed, 'f', 0).rightJustified(3, '0'),
             QString::number(Units::Distance::fromM(altitude).toFeet(), 'f', 0)
                 .rightJustified(6, '0')); // Altitude in feet
}
QString TrafficDataSource_OgnParser::formatLatitude(double latitude)
{
    // e.g. "5111.32N"
    QString const direction = latitude >= 0 ? "N" : "S";
    latitude = qAbs(latitude);
    int const degrees = static_cast<int>(latitude);
    double const minutes = (latitude - degrees) * 60.0;
    return QString("%1%2%3")
        .arg(degrees, 2, 10, QChar('0'))
        .arg(QString::number(minutes, 'f', 2).rightJustified(5, '0'), direction);
}

QString TrafficDataSource_OgnParser::formatLongitude(double longitude)
{
    // e.g. "00102.04W"
    QString const direction = longitude >= 0 ? "E" : "W";
    longitude = qAbs(longitude);
    int const degrees = static_cast<int>(longitude);
    double const minutes = (longitude - degrees) * 60.0;
    return QString("%1%2%3")
        .arg(degrees, 3, 10, QChar('0'))
        .arg(QString::number(minutes, 'f', 2).rightJustified(5, '0'), direction);
}

} // namespace Traffic::Ogn
