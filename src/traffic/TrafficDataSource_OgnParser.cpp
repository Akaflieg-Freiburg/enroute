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
Q_GLOBAL_STATIC(QRegularExpression, s_regex1, R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\I])(\d{5}\.\d{2})([EW])(.)(\d{3})[/\\](\d{3})[/\\]A=(\d{6})$)"_L1);
Q_GLOBAL_STATIC(QRegularExpression, s_regex2, R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\I])(\d{5}\.\d{2})([EW])(.)[/\\]A=(\d{6})$)"_L1);
Q_GLOBAL_STATIC(QRegularExpression, s_regex_weatherreport, R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\I])(\d{5}\.\d{2})([EW])(_)(\d{3})[/](\d{3})g(\d{3})t(\d{3})h(\d{2})b(\d{5})$)"_L1);

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

double TrafficDataSource_OgnParser::decodeLatitude(const QStringView nmeaLatitude, QChar latitudeDirection)
{
    // e.g. "5111.32N"
    bool ok = false;
    double const latitudeDegrees = nmeaLatitude.left(2).toDouble(&ok);
    if (!ok)
    {
        return qQNaN();
    }
    double const latitudeMinutes = nmeaLatitude.mid(2).toDouble(&ok);
    if (!ok)
    {
        return qQNaN();
    }
    double latitude = latitudeDegrees + (latitudeMinutes / 60.0);
    if (latitudeDirection == 'S')
    {
        latitude = -latitude;
    }
    if (latitude < -90 || latitude > 90)
    {
        return qQNaN();
    }
    return latitude;
}

double TrafficDataSource_OgnParser::decodeLongitude(const QStringView nmeaLongitude, QChar longitudeDirection)
{
    // e.g. "00102.04W"
    bool ok = false;
    double const longitudeDegrees = nmeaLongitude.left(3).toDouble(&ok);
    if (!ok)
    {
        return qQNaN();
    }
    double const longitudeMinutes = nmeaLongitude.mid(3).toDouble(&ok);
    if (!ok)
    {
        return qQNaN();
    }
    double longitude = longitudeDegrees + (longitudeMinutes / 60.0);
    if (longitudeDirection == 'W')
    {
        longitude = -longitude;
    }
    if (longitude < -180 || longitude > 180)
    {
        return qQNaN();
    }
    return longitude;
}

// ---------------------------

void TrafficDataSource_OgnParser::parseTrafficReport(OgnMessage& ognMessage, const QStringView header, const QStringView body)
{
    // e.g. header = "FLRDDE626>APRS,qAS,EGHL:"
    // e.g. body = "/074548h5111.32N/00102.04W'086/007/A=000607 id0ADDE626 -019fpm +0.0rot 5.5dB 3e -4.3kHz"
    ognMessage.type = OgnMessageType::TRAFFIC_REPORT;

#ifdef OGN_DEBUG
    qDebug() << "Traffic Report:" << header << ":" << body;
#endif

    // Parse the Header
    auto const index = header.indexOf(u'>');
    if (index == -1)
    {
#if OGN_DEBUG
        qDebug() << "Invalid header format in Traffic Report: " << header << ":" << body;
#endif
        ognMessage.type = OgnMessageType::UNKNOWN;
        return;
    }
    ognMessage.sourceId = header.left(index);

    // Parse the body
    auto const blankIndex = body.indexOf(u' ');
    QStringView aprsPart;
    QStringView ognPart;
    if (blankIndex == -1)
    {
        aprsPart = body;
    }
    else
    {
        aprsPart = QStringView(body.constData(), blankIndex); // APRS Part before the first blank
        ognPart = QStringView(body.constData() + blankIndex + 1, body.size() - blankIndex - 1); // OGN Part after the first blank
    }

    // Parse aprsPart
    // Example: "/074548h5111.32N/00102.04W'086/007/A=000607"
    QStringView latstring1 = {};
    QChar latstring2 = {};
    QStringView lonstring1 = {};
    QChar lonstring2 = {};
    QRegularExpressionMatch const match = s_regex1->match(aprsPart.toString());
    if (match.hasMatch())
    {
        ognMessage.timestamp = match.capturedView(1); // timestamp
        QChar const symboltable = match.capturedView(4).at(0);     // Symbol table
        QChar const symbolselection = match.capturedView(7).at(0); // Symbol
        ognMessage.symbol = AprsSymbolMap->value(QString(symboltable)+symbolselection);
        ognMessage.course = Units::Angle::fromDEG(match.capturedView(8).toDouble());    // Course
        ognMessage.speed = Units::Speed::fromKN(match.capturedView(9).toDouble());     // Speed
        auto altitude = Units::Distance::fromFT(match.capturedView(10).toDouble()).toM();  // Altitude in m
        ognMessage.coordinate.setAltitude(altitude);
        latstring1 = match.capturedView(2);
        latstring2 = match.capturedView(3).at(0);
        auto latitude = decodeLatitude(latstring1, latstring2);
        ognMessage.coordinate.setLatitude(latitude);
        lonstring1 = match.capturedView(5);
        lonstring2 = match.capturedView(6).at(0);
        auto longitude = decodeLongitude(lonstring1, lonstring2);
        ognMessage.coordinate.setLongitude(longitude);
    }
    else
    {
        QRegularExpressionMatch const match = s_regex2->match(aprsPart.toString());
        if (match.hasMatch())
        {
            ognMessage.timestamp = match.capturedView(1); // timestamp
            QChar const symboltable = match.capturedView(4).at(0);     // Symbol table
            QChar const symbolselection = match.capturedView(7).at(0); // Symbol
            ognMessage.symbol = AprsSymbolMap->value(QString(symboltable)+symbolselection);
            auto altitude = Units::Distance::fromFT(match.capturedView(8).toDouble()).toM();  // Altitude in m
            ognMessage.coordinate.setAltitude(altitude);
            latstring1 = match.capturedView(2);
            latstring2 = match.capturedView(3).at(0);
            auto latitude = decodeLatitude(latstring1, latstring2);
            ognMessage.coordinate.setLatitude(latitude);
            lonstring1 = match.capturedView(5);
            lonstring2 = match.capturedView(6).at(0);
            auto longitude = decodeLongitude(lonstring1, lonstring2);
            ognMessage.coordinate.setLongitude(longitude);
        }
        else
        {
            QRegularExpressionMatch const match = s_regex_weatherreport->match(aprsPart.toString());
            if (match.hasMatch())
            {
                ognMessage.type = OgnMessageType::WEATHER;
                ognMessage.timestamp = match.capturedView(1); // timestamp
                QChar const symboltable = match.capturedView(4).at(0);     // Symbol table
                QChar const symbolselection = match.capturedView(7).at(0); // Symbol
                ognMessage.symbol = AprsSymbolMap->value(QString(symboltable)+symbolselection);
                auto latitude = decodeLatitude(match.capturedView(2), match.capturedView(3).at(0));
                ognMessage.coordinate.setLatitude(latitude);
                auto longitude = decodeLongitude(match.capturedView(5), match.capturedView(6).at(0));
                ognMessage.coordinate.setLongitude(longitude);
                ognMessage.wind_direction = match.capturedView(8).toInt();
                ognMessage.wind_speed = match.capturedView(9).toInt();
                ognMessage.wind_gust_speed = match.capturedView(10).toInt();
                ognMessage.temperature = match.capturedView(11).toInt();
                ognMessage.humidity = match.capturedView(12).toInt();
                ognMessage.pressure = match.capturedView(13).toDouble() / 10.0;
            }
            else
            {
#if OGN_DEBUG
                qDebug() << "Invalid APRS format:" << aprsPart;
#endif
                ognMessage.type = OgnMessageType::UNKNOWN;
                return;
            }
        }
    }

    // Parse ognPart by iterating through substrings separated by blanks using an iterator
    if (!ognPart.isEmpty() && !ognPart.trimmed().isEmpty())
    {
        const auto *it = ognPart.cbegin();
        while (it != ognPart.cend())
        {
            // Find the next blank or the end of the string
            const auto *end = std::find(it, ognPart.cend(), u' ');

            // Create a QStringView for the current item
            QStringView const item(it, end);

            if (item.startsWith(u"id"))
            {
                ognMessage.aircraftID = item.mid(2); // Remove "id" prefix
            }
            else if (item.startsWith(u"!W") && item.endsWith(u"!") && item.length() >= 5)
            {
                if(!latstring1.isEmpty() && !lonstring1.isEmpty())
                {
                    QString const lastring1 = QString(latstring1) + item[2];
                    QString const lostring1 = QString(lonstring1) + item[3];
                    ognMessage.coordinate.setLatitude(decodeLatitude(lastring1, latstring2));
                    ognMessage.coordinate.setLongitude(decodeLongitude(lostring1, lonstring2));
                }
            }
            else if (item.startsWith(u"gps"))
            {
                ognMessage.gpsInfo = item;
            }
            else if (item.endsWith(u"fpm"))
            {
                ognMessage.verticalSpeed = Units::Speed::fromFPM(item.mid(0, item.indexOf(u'f')).toDouble()).toMPS();
            }
            else if (item.endsWith(u"rot"))
            {
                ognMessage.rotationRate = item;
            }
            else if (item.endsWith(u"dB"))
            {
                ognMessage.signalStrength = item;
            }
            else if (item.endsWith(u"e"))
            {
                ognMessage.errorCount = item;
            }
            else if (item.endsWith(u"kHz"))
            {
                ognMessage.frequencyOffset = item;
            }
            else if (item.startsWith(u"Sq"))
            {
                ognMessage.squawk = item;
            }
            else if (item.startsWith(u"FL"))
            {
                ognMessage.flightlevel = item;
            }
            else if (item.startsWith(u"A") && item[2] == u':')
            {
                ognMessage.flightnumber = item.mid(item.indexOf(u':') + 1); // Extract flight number after "A3:" or "A5:"
            }
            else
            {
                #if OGN_DEBUG
                qDebug() << "Unrecognized item in ognPart:" << item;
                #endif
            }

            // Move the iterator to the next item
            it = (end != ognPart.cend()) ? end + 1 : ognPart.cend();
        }
    }

    // Parse aircraft type, address type, and address
    if (!ognMessage.aircraftID.isEmpty())
    {
        bool ok = false;
        uint32_t const hexcode = ognMessage.aircraftID.toUInt(&ok, 16);
        if (!ok)
        {
#if OGN_DEBUG
            qDebug() << "Failed to parse aircraft ID as hex:" << ognMessage.aircraftID;
#endif
        }
        else
        {
            ognMessage.stealthMode = ((hexcode & 0x80000000) != 0U);
            ognMessage.noTrackingFlag = ((hexcode & 0x40000000) != 0U);

            uint32_t const aircraftCategory = ((hexcode >> 26) & 0xF);
            Traffic::AircraftType const aircraftTypeEnumValue
                = AircraftCategoryMap->value(aircraftCategory, Traffic::AircraftType::unknown);
            ognMessage.aircraftType = aircraftTypeEnumValue;

            uint32_t const addressTypeValue = (hexcode >> 24) & 0x3;
            ognMessage.addressType = static_cast<OgnAddressType>(addressTypeValue);

            ognMessage.address = QStringView(ognMessage.aircraftID.cbegin()+2, 6);
        }
    }

#ifdef OGN_DEBUG
    qDebug() << "Parsed Traffic Report: " << ognMessage.coordinate.latitude() << " " << ognMessage.coordinate.longitude() << " course:" << ognMessage.course.toDEG()
             << "° speed:" << ognMessage.speed.toKN() << "kn" << " altitude:" << ognMessage.coordinate.altitude()
             << " verticalspeed:" << ognMessage.verticalSpeed << " rotationrate:" << ognMessage.rotationRate
             << " signalstrength:" << ognMessage.signalStrength << " errorCount:" << ognMessage.errorCount
             << " frequencyOffset:" << ognMessage.frequencyOffset << " aircraftType:" << ognMessage.aircraftType
             << " addressType:" << ognMessage.addressType << " address:" << ognMessage.address
             << " stealthMode:" << ognMessage.stealthMode << " NoTrackingFlag:" << ognMessage.noTrackingFlag
             << " sourceId:" << ognMessage.sourceId;
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
