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
    void testParseAprsisMessage_invalidMessage();
    void testParseAprsisMessage_commentMessage();
    void testParseAprsisMessage_receiverStatusMessage();
    void testParseAprsisMessage_Docu();
    void testParseAprsisMessage_ReceivedDataFile();
};

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
    QSKIP("This test is disabled because there are still some issues");
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
    QCOMPARE(message.latitude, +51.1886666667);
    QCOMPARE(message.longitude, -1.034);
    QCOMPARE(message.course, "086");
    QCOMPARE(message.speed, "007");
    QCOMPARE(message.altitude, 185.0136);
    QCOMPARE(message.aircraftID, "0ADDE626");
    QCOMPARE(message.verticalSpeed, "-019fpm");
    QCOMPARE(message.rotationRate, "+0.0rot");
    QCOMPARE(message.signalStrength, "5.5dB");
    QCOMPARE(message.errorCount, "3e");
    QCOMPARE(message.frequencyOffset, "-4.3kHz");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::TowPlane);
    QCOMPARE(message.addressType, "FLARM");
    QCOMPARE(message.address, 0xDDE626);
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_validTrafficReport2() {
    QSKIP("This test is disabled because there are still some issues");
    QString sentence = "ICA4D21C2>OGADSB,qAS,HLST:/001140h4741.90N/01104.20E^124/460/A=034868 !W91! id254D21C2 +128fpm FL350.00 A3:AXY547M Sq2244";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::TRAFFIC_REPORT);
    QCOMPARE(message.latitude, +47.6983333333);
    QCOMPARE(message.longitude, +11.07);
    QCOMPARE(message.course, "124");
    QCOMPARE(message.speed, "460");
    QCOMPARE(message.altitude, 10627.7664);
    QCOMPARE(message.aircraftID, "254D21C2");
    QCOMPARE(message.verticalSpeed, "+128fpm");
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::Jet);
    QCOMPARE(message.addressType, "ICAO");
    QCOMPARE(message.address, 0x4D21C2);
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_invalidMessage() {
    QString sentence = "INVALID MESSAGE FORMAT";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::UNKNOWN);
    QCOMPARE(message.latitude, 0.0);
    QCOMPARE(message.longitude, 0.0);
    QCOMPARE(message.course, "");
    QCOMPARE(message.speed, "");
    QCOMPARE(message.altitude, 0.0);
    QCOMPARE(message.aircraftID, "");
    QCOMPARE(message.verticalSpeed, "");
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::unknown);
    QCOMPARE(message.addressType, "");
    QCOMPARE(message.address, 0);
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_commentMessage() {
    QString sentence = "# This is a comment";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::COMMENT);
    QCOMPARE(message.latitude, 0.0);
    QCOMPARE(message.longitude, 0.0);
    QCOMPARE(message.course, "");
    QCOMPARE(message.speed, "");
    QCOMPARE(message.altitude, 0.0);
    QCOMPARE(message.aircraftID, "");
    QCOMPARE(message.verticalSpeed, "");
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::unknown);
    QCOMPARE(message.addressType, "");
    QCOMPARE(message.address, 0);
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

void TrafficDataSource_OgnTest::testParseAprsisMessage_receiverStatusMessage() {
    QString sentence = "FLRDDE626>APRS,qAS,EGHL:>Receiver Status Message";
    OgnMessage message = TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

    QCOMPARE(message.sentence, sentence);
    QCOMPARE(message.type, OgnMessageType::STATUS);
    QCOMPARE(message.latitude, 0.0);
    QCOMPARE(message.longitude, 0.0);
    QCOMPARE(message.course, "");
    QCOMPARE(message.speed, "");
    QCOMPARE(message.altitude, 0.0);
    QCOMPARE(message.aircraftID, "");
    QCOMPARE(message.verticalSpeed, "");
    QCOMPARE(message.rotationRate, "");
    QCOMPARE(message.signalStrength, "");
    QCOMPARE(message.errorCount, "");
    QCOMPARE(message.frequencyOffset, "");
    QCOMPARE(message.aircraftType, Traffic::AircraftType::unknown);
    QCOMPARE(message.addressType, "");
    QCOMPARE(message.address, 0);
    QCOMPARE(message.stealthMode, false);
    QCOMPARE(message.noTrackingFlag, false);
}

QTEST_MAIN(TrafficDataSource_OgnTest)
#include "TrafficDataSource_OgnTest.moc"
