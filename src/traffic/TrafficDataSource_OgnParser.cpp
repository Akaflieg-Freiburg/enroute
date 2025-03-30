#include "TrafficDataSource_OgnParser.h"
#include "TrafficFactorAircraftType.h"
#include "Units.h"
#include <QRegularExpression>
#include <QDebug>
#include <QMetaEnum>

using namespace Qt::Literals::StringLiterals;

namespace Traffic::Ogn {

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
    {"/'",  Traffic::AircraftType::Drone},           // UAV (Drone)
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
    {"/'",  OgnSymbol::UAV},            // UAV (Drone)
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

OgnMessage TrafficDataSource_OgnParser::parseAprsisMessage(const QString& sentence)
{
    OgnMessage ognMessage;
    ognMessage.sentence = sentence;

    if (sentence.startsWith(u"#"_s)) {
        // Comment message
        ognMessage = parseCommentMessage(sentence);
    } else {
        // Split the sentence into header and body at the first colon
        int colonIndex = sentence.indexOf(u':');
        if (colonIndex == -1) {
            qDebug() << "Invalid message format:" << sentence;
            ognMessage.type = OgnMessageType::UNKNOWN;
            return ognMessage;
        }
        QStringView header(sentence.constData(), colonIndex);
        QStringView body(sentence.constData() + colonIndex + 1, sentence.size() - colonIndex - 1);

        // Check if header and body are valid
        if (header.size() < 5 || body.size() < 5) {
            qDebug() << "Invalid message header or body:" << sentence;
            ognMessage.type = OgnMessageType::UNKNOWN;
            return ognMessage;
        }

        // Determine the type of message based on the first character in the body
        if (body.startsWith(u"/"_s)) {
            // "/" indicates a Traffic Report
            ognMessage = parseTrafficReport(header, body);
        } else if (body.startsWith(u">"_s)) {
            // ">" indicates a Receiver Status
            ognMessage = parseStatusMessage(header, body);
        } else {
            ognMessage.type = OgnMessageType::UNKNOWN;
            qDebug() << "Unknown message type:" << sentence;
        }
    }
    return ognMessage;
}

double TrafficDataSource_OgnParser::decodeLatitude(const QStringView& nmeaLatitude, QChar latitudeDirection)
{
    double latitudeDegrees = nmeaLatitude.left(2).toDouble();
    double latitudeMinutes = nmeaLatitude.mid(2).toDouble();
    double latitude = latitudeDegrees + (latitudeMinutes / 60.0);
    if (latitudeDirection == 'S') {
        latitude = -latitude;
    }
    return latitude;
}

double TrafficDataSource_OgnParser::decodeLongitude(const QStringView& nmeaLongitude, QChar longitudeDirection)
{
    double longitudeDegrees = nmeaLongitude.left(3).toDouble();
    double longitudeMinutes = nmeaLongitude.mid(3).toDouble();
    double longitude = longitudeDegrees + (longitudeMinutes / 60.0);
    if (longitudeDirection == 'W') {
        longitude = -longitude;
    }
    return longitude;
}

OgnMessage TrafficDataSource_OgnParser::parseTrafficReport(const QStringView& header, const QStringView& body)
{
    OgnMessage ognMessage;
    ognMessage.sentence = QString(header) + ":" + QString(body);
    ognMessage.type = OgnMessageType::TRAFFIC_REPORT;

    qDebug() << "Traffic Report:" << header << ":" << body;

    // Parse the Header
    int index = header.indexOf(u'>');
    if (index == -1) {
        qDebug() << "Invalid header format in Traffic Report: " << header << ":" << body;
        ognMessage.type = OgnMessageType::UNKNOWN;
        return ognMessage;
    }
    ognMessage.sourceId = header.left(index);

    // Parse the body
    int blankIndex = body.indexOf(u' ');
    QStringView aprsPart;
    QStringView ognPart;
    if (blankIndex == -1) {
        aprsPart = body;
    }
    else{
        aprsPart = QStringView(body.constData(), blankIndex); // APRS Part before the first blank
        ognPart = QStringView(body.constData() + blankIndex + 1, body.size() - blankIndex - 1); // OGN Part after the first blank
    }

    // Parse aprsPart
    // Example: "/074548h5111.32N/00102.04W'086/007/A=000607"
    QRegularExpression regex(R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\I])(\d{5}\.\d{2})([EW])(.)(\d{3})[/\\](\d{3})[/\\]A=(\d{6})$)");
    QRegularExpressionMatch match = regex.match(aprsPart.toString());
    if (match.hasMatch()) {
        ognMessage.timestamp = match.capturedView(1); // timestamp
        QChar symboltable = match.capturedView(4).at(0); // Symbol table
        QChar symbolselection = match.capturedView(7).at(0); // Symbol
        ognMessage.course = match.capturedView(8);    // Course
        ognMessage.speed = match.capturedView(9);     // Speed
        auto altitude = Units::Distance::fromFT(match.capturedView(10).toDouble()).toM();  // Altitude in m
        ognMessage.coordinate.setAltitude(altitude);
        auto latitude = decodeLatitude(match.capturedView(2), match.capturedView(3).at(0));
        ognMessage.coordinate.setLatitude(latitude);
        auto longitude = decodeLongitude(match.capturedView(5), match.capturedView(6).at(0));
        ognMessage.coordinate.setLongitude(longitude);
    }
    else {
        QRegularExpression regex(R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\I])(\d{5}\.\d{2})([EW])(.)[/\\]A=(\d{6})$)");
        QRegularExpressionMatch match = regex.match(aprsPart.toString());
        if (match.hasMatch()) {
            ognMessage.timestamp = match.capturedView(1); // timestamp
            QChar symboltable = match.capturedView(4).at(0); // Symbol table
            QChar symbolselection = match.capturedView(7).at(0); // Symbol
            auto altitude = Units::Distance::fromFT(match.capturedView(8).toDouble()).toM();  // Altitude in m
            ognMessage.coordinate.setAltitude(altitude);
            auto latitude = decodeLatitude(match.capturedView(2), match.capturedView(3).at(0));
            ognMessage.coordinate.setLatitude(latitude);
            auto longitude = decodeLongitude(match.capturedView(5), match.capturedView(6).at(0));
            ognMessage.coordinate.setLongitude(longitude);
        }
        else{
            QRegularExpression regex(R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\I])(\d{5}\.\d{2})([EW])(_)(\d{3})[/](\d{3})g(\d{3})t(\d{3})h(\d{2})b(\d{5})$)");
            QRegularExpressionMatch match = regex.match(aprsPart.toString());
            if (match.hasMatch()) {
                ognMessage.type = OgnMessageType::WEATHER;
                ognMessage.timestamp = match.capturedView(1); // timestamp
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
                qDebug() << "Invalid APRS format:" << aprsPart;
                ognMessage.type = OgnMessageType::UNKNOWN;
                return ognMessage;
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
            } else if (item.endsWith(u"fpm")) {
                ognMessage.verticalSpeed = item;
            } else if (item.endsWith(u"rot")) {
                ognMessage.rotationRate = item;
            } else if (item.endsWith(u"dB")) {
                ognMessage.signalStrength = item;
            } else if (item.endsWith(u"e")) {
                ognMessage.errorCount = item;
            } else if (item.endsWith(u"kHz")) {
                ognMessage.frequencyOffset = item;
            } else {
                qDebug() << "Unrecognized item in ognPart:" << item;
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
            qDebug() << "Failed to parse aircraft ID as hex:" << ognMessage.aircraftID;
        } else {
            ognMessage.stealthMode = hexcode & 0x80000000;
            ognMessage.noTrackingFlag = hexcode & 0x40000000;

            uint32_t aircraftCategory = ((hexcode >> 26) & 0xF);
            Traffic::AircraftType aircraftTypeEnumValue = AircraftCategoryMap.value(aircraftCategory, Traffic::AircraftType::unknown);
            ognMessage.aircraftType = aircraftTypeEnumValue;

            uint32_t addressTypeValue = (hexcode >> 24) & 0x3;
            ognMessage.addressType = static_cast<OgnAddressType>(addressTypeValue);

            ognMessage.address = hexcode & 0x00FFFFFF;
        }
    }

    qDebug() << "Parsed Traffic Report: " << ognMessage.coordinate.latitude() << " " << ognMessage.coordinate.longitude() << " course:" << ognMessage.course
             << "° speed:" << ognMessage.speed << " altitude:" << ognMessage.coordinate.altitude() 
             << " verticalspeed:" << ognMessage.verticalSpeed << " rotationrate:" << ognMessage.rotationRate
             << " signalstrength:" << ognMessage.signalStrength << " errorCount:" << ognMessage.errorCount
             << " frequencyOffset:" << ognMessage.frequencyOffset << " aircraftType:" << ognMessage.aircraftType 
             << " addressType:" << ognMessage.addressType << " address:" << QString::number(ognMessage.address,16) 
             << " stealthMode:" << ognMessage.stealthMode << " NoTrackingFlag:" << ognMessage.noTrackingFlag
             << " sourceId:" << ognMessage.sourceId;

    return ognMessage;
}

OgnMessage TrafficDataSource_OgnParser::parseCommentMessage(const QString& message)
{
    OgnMessage ognMessage;
    ognMessage.sentence = message;
    ognMessage.type = OgnMessageType::COMMENT;
    return ognMessage;
}

OgnMessage TrafficDataSource_OgnParser::parseStatusMessage(const QStringView& header, const QStringView& body)
{
    OgnMessage ognMessage;
    ognMessage.sentence = QString(header) + ":" + QString(body);
    ognMessage.type = OgnMessageType::STATUS;
    return ognMessage;
}

QString TrafficDataSource_OgnParser::formatPositionReport(const QString& callSign, const QGeoCoordinate& coordinate, double course, double speed, double altitude)
{
    return QString("%1>APRS,TCPIP*: /%2h%3/%4'/%5/%6/A=%7\n")
        .arg(callSign) // Callsign
        .arg(QDateTime::currentDateTimeUtc().toString("hhmmss")) // Timestamp
        .arg(formatLatitude(coordinate.latitude())) // Latitude
        .arg(formatLongitude(coordinate.longitude())) // Longitude
        .arg(QString::number(course, 'f', 0).rightJustified(3, '0')) // Course
        .arg(QString::number(speed, 'f', 0).rightJustified(3, '0')) // Speed
        .arg(QString::number(Units::Distance::fromM(altitude).toFeet(), 'f', 0).rightJustified(6, '0')); // Altitude in feet
}

QString TrafficDataSource_OgnParser::formatLatitude(double latitude)
{
    QString direction = latitude >= 0 ? "N" : "S";
    latitude = qAbs(latitude);
    int degrees = static_cast<int>(latitude);
    double minutes = (latitude - degrees) * 60.0;
    return QString("%1%2%3").arg(degrees, 2, 10, QChar('0')).arg(QString::number(minutes, 'f', 2).rightJustified(5, '0')).arg(direction);
}

QString TrafficDataSource_OgnParser::formatLongitude(double longitude)
{
    QString direction = longitude >= 0 ? "E" : "W";
    longitude = qAbs(longitude);
    int degrees = static_cast<int>(longitude);
    double minutes = (longitude - degrees) * 60.0;
    return QString("%1%2%3").arg(degrees, 3, 10, QChar('0')).arg(QString::number(minutes, 'f', 2).rightJustified(5, '0')).arg(direction);
}

} // namespace Traffic
