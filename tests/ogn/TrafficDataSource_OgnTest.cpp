#include <QtTest/QtTest>
#include <QFile>
#include <QDir>
#include <QDebug>
#include "traffic/TrafficDataSource_OgnParser.h"
#include "traffic/TrafficFactorAircraftType.h"

using namespace Traffic::Ogn;

class TrafficDataSource_OgnTest : public QObject {
    Q_OBJECT

private slots:
    void testParseAprsisMessage_validTrafficReport1();
    void testParseAprsisMessage_validTrafficReport2();
    void testParseAprsisMessage_validTrafficReport3();
    void testParseAprsisMessage_invalidMessage();
    void testParseAprsisMessage_commentMessage();
    void testParseAprsisMessage_receiverStatusMessage();
    void testParseAprsisMessage_Docu();
    void testParseAprsisMessage_ReceivedDataFile();
    void testParseAprsisMessage_weatherReport();
    void testFormatLoginString();
    void testFormatFilterCommand();
    void testFormatPositionReport();
};

void TrafficDataSource_OgnTest::testFormatLoginString() {
    const QStringView callsign = u"ENR12345"_s;
    const QGeoCoordinate receiveLocation = {-48.0, 7.85123456, 250};
    const unsigned int receiveRadius = 99;
    const QStringView swname = u"Akaflieg-Freiburg Enroute"_s;
    const QStringView swversion = u"1.99"_s;

    const QString loginString = TrafficDataSource_OgnParser::formatLoginString(
        callsign, receiveLocation, receiveRadius, swname, swversion);

    QCOMPARE(loginString, "user ENR12345 pass 379 vers Akaflieg-Freiburg Enroute 1.99 filter r/-48.0000/7.8512/99 t/o\n");
}

void TrafficDataSource_OgnTest::testFormatFilterCommand() {
    const QGeoCoordinate receiveLocation = {-48.0, 7.85123456, 250};
    const unsigned int receiveRadius = 99;
    QString command = Traffic::Ogn::TrafficDataSource_OgnParser::formatFilterCommand(receiveLocation, receiveRadius);
    QCOMPARE(command, "# filter r/-48.0000/7.8512/99 t/o\n");
}

void TrafficDataSource_OgnTest::testFormatPositionReport() {
    QStringView callSign = u"ENR12345"_s;
    QGeoCoordinate coordinate = {51.1886666667, -1.034, 185};
    double course = 86.0;
    double speed = 7.0;
    double altitude = 185.0136;

    //qDebug() << "Unknown";
    Traffic::AircraftType aircraftType = Traffic::AircraftType::unknown;
    QString currentTimeStamp = QDateTime::currentDateTimeUtc().toString("hhmmss");
    QString loginString = TrafficDataSource_OgnParser::formatPositionReport(
        callSign, coordinate, course, speed, altitude, aircraftType);
    QCOMPARE(loginString, "ENR12345>APRS,TCPIP*: /"+currentTimeStamp+"h5111.32N/00102.04Wz086/007/A=000607\n");

    //qDebug() << "TowPlane";
    aircraftType = Traffic::AircraftType::TowPlane;
    currentTimeStamp = QDateTime::currentDateTimeUtc().toString("hhmmss");
    loginString = TrafficDataSource_OgnParser::formatPositionReport(
        callSign, coordinate, course, speed, altitude, aircraftType);
    QCOMPARE(loginString, "ENR12345>APRS,TCPIP*: /"+currentTimeStamp+"h5111.32N\\00102.04W^086/007/A=000607\n");

    //qDebug() << "Glider";
    aircraftType = Traffic::AircraftType::Glider;
    currentTimeStamp = QDateTime::currentDateTimeUtc().toString("hhmmss");
    loginString = TrafficDataSource_OgnParser::formatPositionReport(
        callSign, coordinate, course, speed, altitude, aircraftType);
    QCOMPARE(loginString, "ENR12345>APRS,TCPIP*: /"+currentTimeStamp+"h5111.32N/00102.04W'086/007/A=000607\n");

    //qDebug() << "Glider again"; // again to test the caching mechanism
    aircraftType = Traffic::AircraftType::Glider;
    currentTimeStamp = QDateTime::currentDateTimeUtc().toString("hhmmss");
    loginString = TrafficDataSource_OgnParser::formatPositionReport(
        callSign, coordinate, course, speed, altitude, aircraftType);
    QCOMPARE(loginString, "ENR12345>APRS,TCPIP*: /"+currentTimeStamp+"h5111.32N/00102.04W'086/007/A=000607\n");

    //qDebug() << "Aircraft";
    aircraftType = Traffic::AircraftType::Aircraft;
    currentTimeStamp = QDateTime::currentDateTimeUtc().toString("hhmmss");
    loginString = TrafficDataSource_OgnParser::formatPositionReport(
        callSign, coordinate, course, speed, altitude, aircraftType);
    QCOMPARE(loginString, "ENR12345>APRS,TCPIP*: /"+currentTimeStamp+"h5111.32N\\00102.04W^086/007/A=000607\n");

    //qDebug() << "Paraglider";
    aircraftType = Traffic::AircraftType::Paraglider;
    currentTimeStamp = QDateTime::currentDateTimeUtc().toString("hhmmss");
    loginString = TrafficDataSource_OgnParser::formatPositionReport(
        callSign, coordinate, course, speed, altitude, aircraftType);
    QCOMPARE(loginString, "ENR12345>APRS,TCPIP*: /"+currentTimeStamp+"h5111.32N/00102.04Wg086/007/A=000607\n");

    //qDebug() << "Copter";
    aircraftType = Traffic::AircraftType::Copter;
    currentTimeStamp = QDateTime::currentDateTimeUtc().toString("hhmmss");
    loginString = TrafficDataSource_OgnParser::formatPositionReport(
        callSign, coordinate, course, speed, altitude, aircraftType);
    QCOMPARE(loginString, "ENR12345>APRS,TCPIP*: /"+currentTimeStamp+"h5111.32N/00102.04WX086/007/A=000607\n");
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_ReceivedDataFile() {
    QSKIP("This test is disabled because there are still some issues");
    QFile file("../tests/ogn/received_data.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file for reading:" << file.errorString() << " CWD:" << QDir::currentPath();
        QFAIL("File could not be opened");
        return;
    }
    QTextStream in(&file);
    QString sentence;
    while (!in.atEnd()) {
        sentence = in.readLine();
        OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);
        QCOMPARE(message.sentence, sentence);
        QCOMPARE_NE(message.type, OgnMessageType::UNKNOWN);
    }
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_Docu() {
    QString sentences = 
        "FLRDDE626>APRS,qAS,EGHL:/074548h5111.32N/00102.04W'086/007/A=000607 id0ADDE626 -019fpm +0.0rot 5.5dB 3e -4.3kHz \n"
        "LFNW>APRS,TCPIP*,qAC,GLIDERN5:/183804h4254.53NI00203.90E&/A=001000\n"
        "LFNW>APRS,TCPIP*,qAC,GLIDERN5:>183804h v0.2.6.ARM CPU:0.7 RAM:505.3/889.7MB NTP:0.4ms/+7.7ppm +0.0C 0/0Acfts[1h] RF:+69-4.0ppm/+1.77dB/+3.5dB@10km[184484]/+11.2dB@10km[1/1]\n"
        "FLRDDE626>APRS,qAS,EGHL:/074548h5111.32N/00102.04W'086/007/A=000607 id0ADDE626 -019fpm +0.0rot 5.5dB 3e -4.3kHz\n"
        "FLRDDE626>APRS,qAS,EGHL:/074557h5111.32N/00102.01W'086/006/A=000607 id0ADDE626 +020fpm +0.3rot 5.8dB 4e -4.3kHz\n"
        "FLRDDE626>APRS,qAS,EGHL:/074559h5111.32N/00102.00W'090/006/A=000607 id0ADDE626 +020fpm -0.7rot 8.8dB 0e -4.3kHz\n"
        "FLRDDE626>APRS,qAS,EGHL:/074605h5111.32N/00101.98W'090/006/A=000607 id0ADDE626 +020fpm +0.0rot 5.5dB 1e -4.2kHz\n"
        "# aprsc 2.0.14-g28c5a6a 29 Jun 2014 07:46:15 GMT GLIDERN1 37.187.40.234:14580\n";
    QStringList sentencesList = sentences.split('\n', Qt::SkipEmptyParts);
    for (const QString& sentence : sentencesList) {
        OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);
        QCOMPARE(message.sentence, sentence);
        QCOMPARE_NE(message.type, OgnMessageType::UNKNOWN);
    }
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_validTrafficReport1() {
    QString sentence = "FLRDDE626>APRS,qAS,EGHL:/074548h5111.32N/00102.04W'086/007/A=000607 id0ADDE626 -019fpm +0.0rot 5.5dB 3e -4.3kHz";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::TRAFFIC_REPORT);
    QCOMPARE(message.coordinate.latitude(), +51.1886666667);
    QCOMPARE(message.coordinate.longitude(), -1.034);
    QCOMPARE(message.coordinate.altitude(), 185.0136);
    QCOMPARE(message.symbol, OgnSymbol::GLIDER);
    QCOMPARE(message.course, "086");
    QCOMPARE(message.speed, "007");
    QCOMPARE(message.aircraftID, "0ADDE626");
    QCOMPARE(message.verticalSpeed, -0.09652);
    QCOMPARE(message.rotationRate, "+0.0rot");
    QCOMPARE(message.signalStrength, "5.5dB");
    QCOMPARE(message.errorCount, "3e");
    QCOMPARE(message.frequencyOffset, "-4.3kHz");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::TowPlane);
    QCOMPARE(message.addressType, OgnAddressType::FLARM);
    QCOMPARE(message.address, "DDE626");
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_validTrafficReport2() {
    QString sentence = "ICA4D21C2>OGADSB,qAS,HLST:/001140h4741.90N/01104.20E^124/460/A=034868 !W91! id254D21C2 +128fpm FL350.00 A3:AXY547M Sq2244";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::TRAFFIC_REPORT);
    QCOMPARE(message.coordinate.latitude(), +47.6984833333);
    QCOMPARE_GE(message.coordinate.longitude(), +11.0700166667-0.0001);
    QCOMPARE_LE(message.coordinate.longitude(), +11.0700166667+0.0001);
    QCOMPARE(message.coordinate.altitude(), 10627.7664);
    QCOMPARE(message.course, "124");
    QCOMPARE(message.speed, "460");
    QCOMPARE(message.aircraftID, "254D21C2");
    QCOMPARE(message.symbol, OgnSymbol::JET);
    QCOMPARE(message.verticalSpeed, +0.65024);
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::Jet);
    QCOMPARE(message.addressType, OgnAddressType::ICAO);
    QCOMPARE(message.address, "4D21C2");
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_validTrafficReport3() {
    QString sentence = "ICA4D21C2>OGADSB,qAS,HLST:/001140h4741.90N/01104.20E^/A=034868 !W91! id254D21C2 +128fpm FL350.00 A3:AXY547M Sq2244";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::TRAFFIC_REPORT);
    QCOMPARE(message.coordinate.latitude(), +47.6984833333);
    QCOMPARE_GE(message.coordinate.longitude(), +11.0700166667-0.0001);
    QCOMPARE_LE(message.coordinate.longitude(), +11.0700166667+0.0001);
    QCOMPARE(message.coordinate.altitude(), 10627.7664);
    QCOMPARE(message.course, "");
    QCOMPARE(message.speed, "");
    QCOMPARE(message.aircraftID, "254D21C2");
    QCOMPARE(message.symbol, OgnSymbol::JET);
    QCOMPARE(message.verticalSpeed, +0.65024);
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::Jet);
    QCOMPARE(message.addressType, OgnAddressType::ICAO);
    QCOMPARE(message.address, "4D21C2");
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_invalidMessage() {
    QString sentence = "INVALID MESSAGE FORMAT";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::UNKNOWN);
    QVERIFY(qIsNaN(message.coordinate.latitude()));
    QVERIFY(qIsNaN(message.coordinate.longitude()));
    QVERIFY(qIsNaN(message.coordinate.altitude()));
    QCOMPARE(message.course, "");
    QCOMPARE(message.speed, "");
    QCOMPARE(message.aircraftID, "");
    QCOMPARE(message.symbol, OgnSymbol::UNKNOWN);
    QCOMPARE(message.verticalSpeed, 0.0);
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::unknown);
    QCOMPARE(message.addressType, OgnAddressType::UNKNOWN);
    QCOMPARE(message.address, "");
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_commentMessage() {
    QString sentence = "# This is a comment";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::COMMENT);
    QVERIFY(qIsNaN(message.coordinate.latitude()));
    QVERIFY(qIsNaN(message.coordinate.longitude()));
    QVERIFY(qIsNaN(message.coordinate.altitude()));
    QCOMPARE(message.course, "");
    QCOMPARE(message.speed, "");
    QCOMPARE(message.aircraftID, "");
    QCOMPARE(message.symbol, OgnSymbol::UNKNOWN);
    QCOMPARE(message.verticalSpeed, 0.0);
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::unknown);
    QCOMPARE(message.addressType, OgnAddressType::UNKNOWN);
    QCOMPARE(message.address, "");
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_receiverStatusMessage() {
    QString sentence = "FLRDDE626>APRS,qAS,EGHL:>Receiver Status Message";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::STATUS);
    QVERIFY(qIsNaN(message.coordinate.latitude()));
    QVERIFY(qIsNaN(message.coordinate.longitude()));
    QVERIFY(qIsNaN(message.coordinate.altitude()));
    QCOMPARE(message.course, "");
    QCOMPARE(message.speed, "");
    QCOMPARE(message.aircraftID, "");
    QCOMPARE(message.symbol, OgnSymbol::UNKNOWN);
    QCOMPARE(message.verticalSpeed, 0.0);
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::unknown);
    QCOMPARE(message.addressType, OgnAddressType::UNKNOWN);
    QCOMPARE(message.address, "");
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_weatherReport() {
    QString sentence = "FNT08075C>OGNFNT,qAS,Hoernle2:/222245h4803.92N/00800.93E_292/005g010t030h00b65526 5.2dB";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::WEATHER);
    QCOMPARE(message.coordinate.latitude(), 48.0653333333);
    QCOMPARE(message.coordinate.longitude(), 8.0155);
    QCOMPARE(message.wind_direction, 292);
    QCOMPARE(message.wind_speed, 5);
    QCOMPARE(message.wind_gust_speed, 10);
    QCOMPARE(message.temperature, 30);
    QCOMPARE(message.humidity, 0);
    QCOMPARE(message.pressure, 6552.6);

    QCOMPARE(message.course, "");
    QCOMPARE(message.speed, "");
    QVERIFY(qIsNaN(message.coordinate.altitude()));
    QCOMPARE(message.aircraftID, "");
    QCOMPARE(message.symbol, OgnSymbol::WEATHERSTATION);
    QCOMPARE(message.verticalSpeed, 0.0);
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "5.2dB");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::unknown);
    QCOMPARE(message.addressType, OgnAddressType::UNKNOWN);
    QCOMPARE(message.address, "");
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

QTEST_MAIN(TrafficDataSource_OgnTest)
#include "TrafficDataSource_OgnTest.moc"
