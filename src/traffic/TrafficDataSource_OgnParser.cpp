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
    {"/'",  Traffic::AircraftType::Drone}            // UAV (Drone)
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
    if (blankIndex == -1) {
        qDebug() << "Invalid body format in Traffic Report: " << header << ":" << body;
        ognMessage.type = OgnMessageType::UNKNOWN;
        return ognMessage;
    }
    QStringView aprsPart(body.constData(), blankIndex); // APRS Part before the first blank
    QStringView ognPart(body.constData() + blankIndex + 1, body.size() - blankIndex - 1); // OGN Part after the first blank

    // Parse aprsPart
    // Example: "/074548h5111.32N/00102.04W'086/007/A=000607"
    QRegularExpression regex(R"(^[/\\](\d{6})h(\d{4}\.\d{2})([NS])([/\\])(\d{5}\.\d{2})([EW])(.)(\d{3})[/\\](\d{3})[/\\]A=(\d{6})$)");
    QRegularExpressionMatch match = regex.match(aprsPart.toString());

    if (!match.hasMatch()) {
        qDebug() << "Invalid APRS format:" << aprsPart;
        ognMessage.type = OgnMessageType::UNKNOWN;
        return ognMessage;
    }

    ognMessage.timestamp = match.capturedView(1); // timestamp
    QChar symboltable = match.capturedView(4).at(0); // Symbol table
    QChar symbolselection = match.capturedView(7).at(0); // Symbol
    ognMessage.course = match.capturedView(8);    // Course
    ognMessage.speed = match.capturedView(9);     // Speed
    ognMessage.altitude = Units::Distance::fromFT(match.capturedView(10).toDouble()).toM();  // Altitude in m
    ognMessage.coordinate.setAltitude(ognMessage.altitude);

    // Convert latitude from NMEA to decimal
    QStringView nmeaLatitude = match.capturedView(2);
    QChar latitudeDirection = match.capturedView(3).at(0);
    double latitudeDegrees = nmeaLatitude.left(2).toDouble();
    double latitudeMinutes = nmeaLatitude.mid(2).toDouble();
    ognMessage.latitude = latitudeDegrees + (latitudeMinutes / 60.0);
    if (latitudeDirection == 'S') {
        ognMessage.latitude = -ognMessage.latitude;
    }
    ognMessage.coordinate.setLatitude(ognMessage.latitude);

    // Convert longitude from NMEA to decimal
    QStringView nmeaLongitude = match.capturedView(5);
    QChar longitudeDirection = match.capturedView(6).at(0);
    double longitudeDegrees = nmeaLongitude.left(3).toDouble();
    double longitudeMinutes = nmeaLongitude.mid(3).toDouble();
    ognMessage.longitude = longitudeDegrees + (longitudeMinutes / 60.0);
    if (longitudeDirection == 'W') {
        ognMessage.longitude = -ognMessage.longitude;
    }
    ognMessage.coordinate.setLongitude(ognMessage.longitude);

    // Parse ognPart for additional fields
    QRegularExpression ognRegex(R"(id(\w+)\s+([-+]?\d+fpm)\s+([-+]?\d+\.\d+rot)\s+(\d+\.\d+dB)\s+(\d+e)\s+([-+]?\d+\.\dkHz))");
    QRegularExpressionMatch ognMatch = ognRegex.match(ognPart.toString());
    if (ognMatch.hasMatch()) {
        ognMessage.aircraftID = ognMatch.capturedView(1); // Aircraft ID (with "id" prefix removed)
        ognMessage.verticalSpeed = ognMatch.capturedView(2);   // Vertical speed
        ognMessage.rotationRate = ognMatch.capturedView(3);    // Rotation rate
        ognMessage.signalStrength = ognMatch.capturedView(4);  // Signal strength
        ognMessage.errorCount = ognMatch.capturedView(5);      // Error count
        ognMessage.frequencyOffset = ognMatch.capturedView(6); // Frequency offset

        // Parse aircraft type, address type, and address
        // Example: "id0ADDE626"
        // Needs to be parsed as a hex number into a 32 bit unsigned int. 
        bool ok = false;
        uint32_t hexcode = ognMessage.aircraftID.toString().toUInt(&ok, 16);
        if (!ok) {
            qDebug() << "Failed to parse aircraft ID as hex:" << ognMessage.aircraftID;
        } else {
            ognMessage.stealthMode = hexcode & 0x80000000;

            ognMessage.noTrackingFlag = hexcode & 0x40000000;

            uint32_t aircraftCategory = ((hexcode >> 26) & 0xF);
            Traffic::AircraftType aircraftTypeEnumValue = AircraftCategoryMap.value(aircraftCategory, Traffic::AircraftType::unknown);
            //const QMetaEnum aircraftTypeMetaEnum = QMetaEnum::fromType<Traffic::AircraftType::EAircraftType>();
            ognMessage.aircraftType = aircraftTypeEnumValue;

            uint32_t addressTypeValue = (hexcode >> 24) & 0x3;
            const QMetaEnum addressTypeMetaEnum = QMetaEnum::fromType<OgnAddressType>();
            ognMessage.addressType = addressTypeMetaEnum.valueToKey(addressTypeValue);

            ognMessage.address = hexcode & 0x00FFFFFF;
        }
    }

    qDebug() << "Parsed Traffic Report: " << ognMessage.latitude << " " << ognMessage.longitude << " course:" << ognMessage.course
             << "° speed:" << ognMessage.speed << " altitude:" << ognMessage.altitude 
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

} // namespace Traffic
