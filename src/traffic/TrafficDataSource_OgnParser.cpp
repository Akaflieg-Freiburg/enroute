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
#include "Units.h"
#include <QRegularExpression>
#include <QDebug>
#include <QMetaEnum>

using namespace Qt::Literals::StringLiterals;

namespace Traffic::Ogn {

const QRegularExpression TrafficDataSource_OgnParser::s_regex1(R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\I])(\d{5}\.\d{2})([EW])(.)(\d{3})[/\\](\d{3})[/\\]A=(\d{6})$)");
const QRegularExpression TrafficDataSource_OgnParser::s_regex2(R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\I])(\d{5}\.\d{2})([EW])(.)[/\\]A=(\d{6})$)");
const QRegularExpression TrafficDataSource_OgnParser::s_regex_weatherreport(R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\I])(\d{5}\.\d{2})([EW])(_)(\d{3})[/](\d{3})g(\d{3})t(\d{3})h(\d{2})b(\d{5})$)");

// see http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
static const QMap<QString, Traffic::AircraftType> AircraftTypeMap = {
    {"/z",  Traffic::AircraftType::unknown},         // Unknown
    {"/'",  Traffic::AircraftType::Glider},          // Glider
    {"/X",  Traffic::AircraftType::Copter},          // Helicopter
    {"/g",  Traffic::AircraftType::Paraglider},      // Parachute, Hang Glider, Paraglider
    {"\\^", Traffic::AircraftType::Aircraft},        // Drop Plane, Powered Aircraft
    {"/^",  Traffic::AircraftType::Jet},             // Jet Aircraft
    {"/O",  Traffic::AircraftType::Balloon},         // Balloon, Airship
    {"\\n", Traffic::AircraftType::StaticObstacle},  // Static Object
};
static const QMap<QString, OgnSymbol> AprsSymbolMap = {
    {"/z",  OgnSymbol::UNKNOWN},        // Unknown
    {"/'",  OgnSymbol::GLIDER},         // Glider
    {"/X",  OgnSymbol::HELICOPTER},     // Helicopter
    {"/g",  OgnSymbol::PARACHUTE},      // Parachute, Hang Glider, Paraglider
    {"\\^", OgnSymbol::AIRCRAFT},       // Drop Plane, Powered Aircraft
    {"/^",  OgnSymbol::JET},            // Jet Aircraft
    {"/O",  OgnSymbol::BALLOON},        // Balloon, Airship
    {"\\n", OgnSymbol::STATIC_OBJECT},  // Static Object
    {"/_",  OgnSymbol::WEATHERSTATION},  // WeatherStation
};

// see http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
static const QMap<uint32_t, Traffic::AircraftType> AircraftCategoryMap = {
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
};

void TrafficDataSource_OgnParser::parseAprsisMessage(OgnMessage& ognMessage)
{
    const QString& sentence = ognMessage.sentence;

    if (sentence.startsWith(u"#"_s)) {
        // Comment message  
        parseCommentMessage(ognMessage);
        return;
    } else {
        // Split the sentence into header and body at the first colon
        int colonIndex = sentence.indexOf(u':');
        if (colonIndex == -1) {
            #if OGN_DEBUG
            qDebug() << "Invalid message format:" << sentence;
            #endif
            ognMessage.type = OgnMessageType::UNKNOWN;
            return;
        }
        // This function runs all the time, so it is performance critical. 
        // I try to avoid heap allocations and use a lot of QStringView.
        QStringView header(sentence.constData(), colonIndex);
        QStringView body(sentence.constData() + colonIndex + 1, sentence.size() - colonIndex - 1);

        // Check if header and body are valid
        if (header.size() < 5 || body.size() < 5) {
            #if OGN_DEBUG
            qDebug() << "Invalid message header or body:" << sentence;
            #endif
            ognMessage.type = OgnMessageType::UNKNOWN;
            return;
        }

        // Determine the type of message based on the first character in the body
        if (body.startsWith(u"/"_s)) {
            // "/" indicates a Traffic Report
            parseTrafficReport(ognMessage, header, body);
        } else if (body.startsWith(u">"_s)) {
            // ">" indicates a Receiver Status
            parseStatusMessage(ognMessage, header, body);
        } else {
            ognMessage.type = OgnMessageType::UNKNOWN;
            #if OGN_DEBUG
            qDebug() << "Unknown message type:" << sentence;
            #endif
            return;
        }
    }
}

double TrafficDataSource_OgnParser::decodeLatitude(const QStringView nmeaLatitude, QChar latitudeDirection)
{
    // e.g. "5111.32N"
    double latitudeDegrees = nmeaLatitude.left(2).toDouble();
    double latitudeMinutes = nmeaLatitude.mid(2).toDouble();
    double latitude = latitudeDegrees + (latitudeMinutes / 60.0);
    if (latitudeDirection == 'S') {
        latitude = -latitude;
    }
    return latitude;
}

double TrafficDataSource_OgnParser::decodeLongitude(const QStringView nmeaLongitude, QChar longitudeDirection)
{
    // e.g. "00102.04W"
    double longitudeDegrees = nmeaLongitude.left(3).toDouble();
    double longitudeMinutes = nmeaLongitude.mid(3).toDouble();
    double longitude = longitudeDegrees + (longitudeMinutes / 60.0);
    if (longitudeDirection == 'W') {
        longitude = -longitude;
    }
    return longitude;
}

void TrafficDataSource_OgnParser::parseTrafficReport(OgnMessage& ognMessage, const QStringView header, const QStringView body)
{
    // e.g. header = "FLRDDE626>APRS,qAS,EGHL:"
    // e.g. body = "/074548h5111.32N/00102.04W'086/007/A=000607 id0ADDE626 -019fpm +0.0rot 5.5dB 3e -4.3kHz"
    ognMessage.type = OgnMessageType::TRAFFIC_REPORT;

    #ifdef OGN_DEBUG
    qDebug() << "Traffic Report:" << header << ":" << body;
    #endif

    // Parse the Header
    int index = header.indexOf(u'>');
    if (index == -1) {
        #if OGN_DEBUG
        qDebug() << "Invalid header format in Traffic Report: " << header << ":" << body;
        #endif
        ognMessage.type = OgnMessageType::UNKNOWN;
        return;
    }
    ognMessage.sourceId = header.left(index);

    // Parse the body
    int blankIndex = body.indexOf(u' ');
    QStringView aprsPart;
    QStringView ognPart;
    if (blankIndex == -1) {
        aprsPart = body;
    }
    else {
        aprsPart = QStringView(body.constData(), blankIndex); // APRS Part before the first blank
        ognPart = QStringView(body.constData() + blankIndex + 1, body.size() - blankIndex - 1); // OGN Part after the first blank
    }

    // Parse aprsPart
    // Example: "/074548h5111.32N/00102.04W'086/007/A=000607"
    QStringView latstring1 = {};
    QChar latstring2 = {};
    QStringView lonstring1 = {};
    QChar lonstring2 = {};
    QRegularExpressionMatch match = s_regex1.match(aprsPart.toString());
    if (match.hasMatch()) {
        ognMessage.timestamp = match.capturedView(1); // timestamp
        QChar symboltable = match.capturedView(4).at(0); // Symbol table
        QChar symbolselection = match.capturedView(7).at(0); // Symbol
        ognMessage.symbol = AprsSymbolMap.value(QString(symboltable)+symbolselection);
        ognMessage.course = match.capturedView(8);    // Course
        ognMessage.speed = match.capturedView(9);     // Speed
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
    else {
        QRegularExpressionMatch match = s_regex2.match(aprsPart.toString());
        if (match.hasMatch()) {
            ognMessage.timestamp = match.capturedView(1); // timestamp
            QChar symboltable = match.capturedView(4).at(0); // Symbol table
            QChar symbolselection = match.capturedView(7).at(0); // Symbol
            ognMessage.symbol = AprsSymbolMap.value(QString(symboltable)+symbolselection);
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
        else{
            QRegularExpressionMatch match = s_regex_weatherreport.match(aprsPart.toString());
            if (match.hasMatch()) {
                ognMessage.type = OgnMessageType::WEATHER;
                ognMessage.timestamp = match.capturedView(1); // timestamp
                QChar symboltable = match.capturedView(4).at(0); // Symbol table
                QChar symbolselection = match.capturedView(7).at(0); // Symbol
                ognMessage.symbol = AprsSymbolMap.value(QString(symboltable)+symbolselection);
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
            else {
                #if OGN_DEBUG
                qDebug() << "Invalid APRS format:" << aprsPart;
                #endif
                ognMessage.type = OgnMessageType::UNKNOWN;
                return;
            }
        }
    }

    // Parse ognPart by iterating through substrings separated by blanks using an iterator
    if (!ognPart.isEmpty() && !ognPart.trimmed().isEmpty()) {
        auto it = ognPart.cbegin();
        while (it != ognPart.cend()) {
            // Find the next blank or the end of the string
            auto end = std::find(it, ognPart.cend(), u' ');

            // Create a QStringView for the current item
            QStringView item(it, end);

            if (item.startsWith(u"id")) {
                ognMessage.aircraftID = item.mid(2); // Remove "id" prefix
            } else if (item.startsWith(u"!W") && item.endsWith(u"!") && item.length() >= 5) {
                if(!latstring1.isEmpty() && !lonstring1.isEmpty()) {
                    QString lastring1 = QString(latstring1) + item[2];
                    QString lostring1 = QString(lonstring1) + item[3]; 
                    ognMessage.coordinate.setLatitude(decodeLatitude(lastring1, latstring2));
                    ognMessage.coordinate.setLongitude(decodeLongitude(lostring1, lonstring2));
                }
            } else if (item.startsWith(u"gps")) {
                ognMessage.gpsInfo = item;
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
            } else if (item.startsWith(u"Sq")) {
                ognMessage.squawk = item;
            } else if (item.startsWith(u"FL")) {
                ognMessage.flightlevel = item;
            } else if (item.startsWith(u"A") && item[2] == u':') {
                ognMessage.flightnumber = item.mid(item.indexOf(u':') + 1); // Extract flight number after "A3:" or "A5:"
            } else {
                #if OGN_DEBUG
                qDebug() << "Unrecognized item in ognPart:" << item;
                #endif
            }

            // Move the iterator to the next item
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
            Traffic::AircraftType aircraftTypeEnumValue = AircraftCategoryMap.value(aircraftCategory, Traffic::AircraftType::unknown);
            ognMessage.aircraftType = aircraftTypeEnumValue;

            uint32_t addressTypeValue = (hexcode >> 24) & 0x3;
            ognMessage.addressType = static_cast<OgnAddressType>(addressTypeValue);

            ognMessage.address = QStringView(ognMessage.aircraftID.cbegin()+2, 6);
        }
    }

    #ifdef OGN_DEBUG
    qDebug() << "Parsed Traffic Report: " << ognMessage.coordinate.latitude() << " " << ognMessage.coordinate.longitude() << " course:" << ognMessage.course
             << "° speed:" << ognMessage.speed << " altitude:" << ognMessage.coordinate.altitude() 
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
    return;
}

void TrafficDataSource_OgnParser::parseStatusMessage(
    OgnMessage& ognMessage, const QStringView header, const QStringView body)
{
    ognMessage.type = OgnMessageType::STATUS;
    return;
}

QString TrafficDataSource_OgnParser::formatPositionReport(
    const QStringView callSign, const QGeoCoordinate& coordinate, double course, double speed, double altitude, Traffic::AircraftType aircraftType)
{
    // e.g. "ENR12345>APRS,TCPIP*: /074548h5111.32N/00102.04W'086/007/A=000607"

    // Static cache for the last lookup
    static Traffic::AircraftType lastAircraftType = Traffic::AircraftType::unknown;
    static QString lastSymbol = "/z"; // Default to unknown symbol

    // Check if the cached value matches the current aircraft type
    if (lastAircraftType != aircraftType) {
        // Perform reverse lookup
        lastSymbol.clear();
        for (auto it = AircraftTypeMap.cbegin(); it != AircraftTypeMap.cend(); ++it) {
            if (it.value() == aircraftType) {
                lastSymbol = it.key();
                break;
            }
        }

        // Default to unknown symbol if no match is found
        if (lastSymbol.isEmpty()) {
            lastSymbol = "\\^";
        }

        // Update the cache
        lastAircraftType = aircraftType;
    }

    return QString("%1>APRS,TCPIP*: /%2h%3%4%5%6%7/%8/A=%9\n")
        .arg(callSign.toString()) // Callsign
        .arg(QDateTime::currentDateTimeUtc().toString("hhmmss")) // Timestamp
        .arg(formatLatitude(coordinate.latitude())) // Latitude
        .arg(lastSymbol[0]) // Symbol table
        .arg(formatLongitude(coordinate.longitude())) // Longitude
        .arg(lastSymbol[1]) // Symbol code
        .arg(QString::number(course, 'f', 0).rightJustified(3, '0')) // Course
        .arg(QString::number(speed, 'f', 0).rightJustified(3, '0')) // Speed
        .arg(QString::number(Units::Distance::fromM(altitude).toFeet(), 'f', 0).rightJustified(6, '0')); // Altitude in feet
}

QString TrafficDataSource_OgnParser::formatLatitude(double latitude)
{
    // e.g. "5111.32N"
    QString direction = latitude >= 0 ? "N" : "S";
    latitude = qAbs(latitude);
    int degrees = static_cast<int>(latitude);
    double minutes = (latitude - degrees) * 60.0;
    return QString("%1%2%3").arg(degrees, 2, 10, QChar('0')).arg(QString::number(minutes, 'f', 2).rightJustified(5, '0')).arg(direction);
}

QString TrafficDataSource_OgnParser::formatLongitude(double longitude)
{
    // e.g. "00102.04W"
    QString direction = longitude >= 0 ? "E" : "W";
    longitude = qAbs(longitude);
    int degrees = static_cast<int>(longitude);
    double minutes = (longitude - degrees) * 60.0;
    return QString("%1%2%3").arg(degrees, 3, 10, QChar('0')).arg(QString::number(minutes, 'f', 2).rightJustified(5, '0')).arg(direction);
}

QString TrafficDataSource_OgnParser::formatFilterCommand(const QGeoCoordinate& receiveLocation, const unsigned int receiveRadius)
{
    // e.g. "# filter r/-48.0000/7.8512/99 t/o"
    return QString("# filter %1\n").arg(formatFilter(receiveLocation, receiveRadius));
}

// see https://www.aprs-is.net/javAPRSFilter.aspx
QString TrafficDataSource_OgnParser::formatFilter(const QGeoCoordinate& receiveLocation, const unsigned int receiveRadius)
{
    // e.g. "r/-48.0000/7.8512/99 t/o"
    return QString("r/%1/%2/%3 t/o")
        .arg(receiveLocation.latitude(), 1, 'f', 4)
        .arg(receiveLocation.longitude(), 1, 'f', 4)
        .arg(receiveRadius);
}

QString TrafficDataSource_OgnParser::formatLoginString(
    const QStringView callSign, const QGeoCoordinate& receiveLocation, const unsigned int receiveRadius, const QStringView appName, const QStringView appVersion)
{
    // e.g. "user ENR12345 pass 1234 vers 1.0.0 1.0 filter r/-48.0000/7.8512/99 t/o"

    // Prepare the filter for what data we want to receive
    QString filter = formatFilter(receiveLocation, receiveRadius);

    // Calculate the password based on the call sign
    QString passcode = calculatePassword(callSign);

    auto loginString = QString("user %1 pass %2 vers %3 %4 filter %5\n")
        .arg(callSign.toString())    // Callsign
        .arg(passcode)              // Calculated passcode
        .arg(appName.toString())    // Application name
        .arg(appVersion.toString()) // Application version
        .arg(filter);               // Filter string

    return loginString;
}

QString TrafficDataSource_OgnParser::calculatePassword(const QStringView callSign)
{
    // APRS-IS passcode calculation: Sum of ASCII values of the first 6 characters of the call sign
    // e.g. "1234"
    int sum = 0;
    for (int i = 0; i < callSign.length() && i < 6; ++i) {
        sum += callSign.at(i).unicode();
    }
    return QString::number(sum % 10000); // Modulo 10000 to get a 4-digit passcode
}

} // namespace Traffic
