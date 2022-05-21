/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include <QApplication>
#include <QDebug>
#include <QEventLoop>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>
#include <QTranslator>
#include <chrono>

#include "DemoRunner.h"
#include "GlobalObject.h"
#include "Settings.h"
#include "geomaps/GeoMapProvider.h"
#include "navigation/Navigator.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_Simulate.h"
#include "traffic/TrafficFactor_WithPosition.h"
#include "weather/WeatherDataProvider.h"

using namespace std::chrono_literals;


DemoRunner::DemoRunner(QObject *parent) : QObject(parent) {

}


void delay(std::chrono::milliseconds ms)
{
    QEventLoop loop; // define a new event loop
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}


auto findQQuickItem(const QString &objectName, QQmlApplicationEngine* engine) -> QObject*
{
    foreach (auto rootItem, engine->rootObjects()) {
        if (rootItem->objectName() == objectName) {
            return rootItem;
        }
        auto *objectPtr = rootItem->findChild<QObject*>(objectName);
        if (objectPtr != nullptr) {
            return objectPtr;
        }
    }
    return nullptr;
}


void DemoRunner::run()
{
    Q_ASSERT(m_engine != nullptr);

    // Obtain pointers to QML items
    auto* applicationWindow = qobject_cast<QQuickWindow*>(findQQuickItem(QStringLiteral("applicationWindow"), m_engine));
    Q_ASSERT(applicationWindow != nullptr);
    auto* flightMap = findQQuickItem(QStringLiteral("flightMap"), m_engine);
    Q_ASSERT(flightMap != nullptr);
    auto *waypointDescription = findQQuickItem(QStringLiteral("waypointDescription"), m_engine);
    Q_ASSERT(waypointDescription != nullptr);

    // Set up traffic simulator
    GlobalObject::settings()->setPositioningByTrafficDataReceiver(true);
    auto* trafficSimulator = new Traffic::TrafficDataSource_Simulate();
    GlobalObject::trafficDataProvider()->addDataSource( trafficSimulator );
    trafficSimulator->connectToTrafficReceiver();
    delay(5s);

    // Set up aircraft
    Navigation::Aircraft acft;
    acft.setName(QStringLiteral("D-EULL"));
    acft.setHorizontalDistanceUnit(Navigation::Aircraft::NauticalMile);
    acft.setVerticalDistanceUnit(Navigation::Aircraft::Feet);
    acft.setFuelConsumptionUnit(Navigation::Aircraft::LiterPerHour);
    acft.setCruiseSpeed(Units::Speed::fromKN(90));
    acft.setDescentSpeed(Units::Speed::fromKN(120));
    acft.setMinimumSpeed(Units::Speed::fromKN(65));
    acft.setFuelConsumption(Units::VolumeFlow::fromLPH(18));
    GlobalObject::navigator()->setAircraft(acft);

    // Set up wind
    Weather::Wind wind;
    wind.setDirectionFrom( Units::Angle::fromDEG(210) );
    wind.setSpeed( Units::Speed::fromKN(10) );
    GlobalObject::navigator()->setWind(wind);

    // Set up route
    GlobalObject::navigator()->flightRoute()->clear();

    // Settings
    GlobalObject::settings()->setAirspaceAltitudeLimit({});
    GlobalObject::settings()->setHideGlidingSectors(true);

    //
    // GENERATE SCREENSHOTS FOR GOOGLE PLAY
    //

    {
        QStringList languages = {"de-DE", "en-US", "fr-FR", "it-IT", "pl-PL"};
        QStringList devices = {"phone", "sevenInch", "tenInch"};

        foreach(auto device, devices) {
            foreach(auto language, languages) {
                if (device == QLatin1String("phone")) {
                    applicationWindow->setProperty("width", 1080/2.5);
                    applicationWindow->setProperty("height", 1920/2.5);
                }
                if (device == QLatin1String("sevenInch")) {
                    applicationWindow->setProperty("width", 1920/2);
                    applicationWindow->setProperty("height", 1200/2);
                }
                if (device == QLatin1String("tenInch")) {
                    applicationWindow->setProperty("width", 1920/2);
                    applicationWindow->setProperty("height", 1200/2);
                }


                qWarning() << "Generating screenshots for Google Play" << language;
                int count = 1;

                // Generate directory
                QDir dir;
                dir.mkpath(QStringLiteral("fastlane/metadata/android/%1/images/%2Screenshots").arg(language, device));

                // Set language
                setLanguage(language);

                // Enroute near EDSB
                {
                    qWarning() << "… En route near EDSB";
                    trafficSimulator->setCoordinate( {48.8442094, 8.45, Units::Distance::fromFT(7512).toM()} );
                    trafficSimulator->setBarometricHeight( Units::Distance::fromFT(7480) );
                    trafficSimulator->setTT( Units::Angle::fromDEG(30) );
                    trafficSimulator->setGS( Units::Speed::fromKN(91) );

                    GlobalObject::navigator()->flightRoute()->clear();
                    GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("EDTL")) );
                    GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("KRH")) );
                    GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("EDTY")) );

                    flightMap->setProperty("zoomLevel", 11);
                    GlobalObject::settings()->setMapBearingPolicy(Settings::TTUp);
                    delay(4s);
                    applicationWindow->grabWindow().save(QStringLiteral("fastlane/metadata/android/%1/images/%2Screenshots/%3_%1.png").arg(language, device).arg(count++));
                    GlobalObject::navigator()->flightRoute()->clear();
                }

                // Approaching EDDR
                {
                    qWarning() << "… Approaching EDDR";
                    trafficSimulator->setCoordinate( {49.30, 7.0028, Units::Distance::fromFT(5500).toM()} );
                    trafficSimulator->setBarometricHeight( Units::Distance::fromFT(5500) );
                    trafficSimulator->setTT( Units::Angle::fromDEG(158) );
                    trafficSimulator->setGS( Units::Speed::fromKN(91) );
                    flightMap->setProperty("zoomLevel", 12);
                    GlobalObject::settings()->setMapBearingPolicy(Settings::TTUp);
                    delay(4s);
                    applicationWindow->grabWindow().save(QStringLiteral("fastlane/metadata/android/%1/images/%2Screenshots/%3_%1.png").arg(language, device).arg(count++));
                }

                // Approaching EDTF w/ traffic
                {
                    qWarning() << "… EDTF Traffic";

                    QGeoCoordinate ownPosition(48.00144, 7.76231, 604);
                    trafficSimulator->setCoordinate( ownPosition );
                    trafficSimulator->setBarometricHeight( Units::Distance::fromM(600) );
                    trafficSimulator->setTT( Units::Angle::fromDEG(36) );
                    trafficSimulator->setGS( Units::Speed::fromKN(92) );
                    flightMap->setProperty("zoomLevel", 13);
                    flightMap->setProperty("followGPS", true);
                    GlobalObject::settings()->setMapBearingPolicy(Settings::TTUp);
                    QGeoCoordinate trafficPosition(48.0103, 7.8052, 540);
                    QGeoPositionInfo trafficInfo;
                    trafficInfo.setCoordinate(trafficPosition);
                    trafficInfo.setAttribute(QGeoPositionInfo::Direction, 160);
                    trafficInfo.setAttribute(QGeoPositionInfo::GroundSpeed, Units::Speed::fromKN(70).toMPS() );
                    trafficInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, -2);
                    trafficInfo.setTimestamp( QDateTime::currentDateTimeUtc() );
                    auto* trafficFactor1 = new Traffic::TrafficFactor_WithPosition(this);
                    trafficFactor1->setAlarmLevel(0);
                    trafficFactor1->setID(QStringLiteral("newID"));
                    trafficFactor1->setType(Traffic::TrafficFactor_Abstract::Aircraft);
                    trafficFactor1->setPositionInfo(trafficInfo);
                    trafficFactor1->setHDist( Units::Distance::fromM(1000) );
                    trafficFactor1->setVDist( Units::Distance::fromM(17) );
                    trafficSimulator->addTraffic(trafficFactor1);

                    auto* trafficFactor2 = new Traffic::TrafficFactor_DistanceOnly(this);
                    trafficFactor2->setAlarmLevel(1);
                    trafficFactor2->setID(QStringLiteral("newID"));
                    trafficFactor2->setHDist( Units::Distance::fromM(1000) );
                    trafficFactor2->setType( Traffic::TrafficFactor_Abstract::Aircraft );
                    trafficFactor2->setCallSign({});
                    trafficFactor2->setCoordinate(ownPosition);
                    trafficSimulator->setTrafficFactor_DistanceOnly(trafficFactor2);

                    delay(4s);
                    applicationWindow->grabWindow().save(QStringLiteral("fastlane/metadata/android/%1/images/%2Screenshots/%3_%1.png").arg(language, device).arg(count++));
                    trafficFactor1->setHDist( {} );
                    trafficSimulator->removeTraffic();
                    trafficSimulator->setTrafficFactor_DistanceOnly( nullptr );
                    delay(20s);
                }

                // Erfurt Airport Info
                {
                    qWarning() << "… EDDE Info Page";
                    auto waypoint = GlobalObject::geoMapProvider()->findByID(QStringLiteral("EDDE"));
                    waypointDescription->setProperty("waypoint", QVariant::fromValue(waypoint));
                    QMetaObject::invokeMethod(waypointDescription, "open", Qt::QueuedConnection);
                    delay(4s);
                    applicationWindow->grabWindow().save(QStringLiteral("fastlane/metadata/android/%1/images/%2Screenshots/%3_%1.png").arg(language, device).arg(count++));
                    QMetaObject::invokeMethod(waypointDescription, "close", Qt::QueuedConnection);
                }

                // Weather Dialog
                {
                    qWarning() << "… LFSB Weather Dialog";
                    emit requestOpenWeatherPage();
                    auto *weatherReport = findQQuickItem(QStringLiteral("weatherReport"), m_engine);
                    Q_ASSERT(weatherReport != nullptr);
                    auto *station = GlobalObject::weatherDataProvider()->findWeatherStation(QStringLiteral("LFSB"));
                    Q_ASSERT(station != nullptr);
                    weatherReport->setProperty("weatherStation", QVariant::fromValue(station));
                    QMetaObject::invokeMethod(weatherReport, "open", Qt::QueuedConnection);
                    delay(4s);
                    applicationWindow->grabWindow().save(QStringLiteral("fastlane/metadata/android/%1/images/%2Screenshots/%3_%1.png").arg(language, device).arg(count++));
                    emit requestClosePages();
                }

                // Nearby waypoints
                {
                    qWarning() << "… Nearby Waypoints Page";
                    emit requestOpenNearbyPage();
                    delay(4s);
                    applicationWindow->grabWindow().save(QStringLiteral("fastlane/metadata/android/%1/images/%2Screenshots/%3_%1.png").arg(language, device).arg(count++));
                    emit requestClosePages();
                }
            }
        }
    } // Google Play


    //
    // GENERATE SCREENSHOTS FOR MANUAL
    //
    qWarning() << "Generating screenshots for manual";

    // Set language
    setLanguage(QStringLiteral("en"));

    // Resize window
    applicationWindow->setProperty("width", 400);
    applicationWindow->setProperty("height", 600);

    // Clear flight route
    GlobalObject::navigator()->flightRoute()->clear();

    // Route page
    {
        qWarning() << "… Route Page";
        GlobalObject::navigator()->flightRoute()->clear();
        emit requestOpenRoutePage();
        delay(2s);
        applicationWindow->grabWindow().save(QStringLiteral("02-02-01-RouteEmpty.png"));

        // Set data for a reasonable route
        GlobalObject::navigator()->flightRoute()->clear();
        GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("EDTF")) );
        GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("KRH")) );
        GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("EDFW")) );
        GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("EDQD")) );

        delay(2s);
        applicationWindow->grabWindow().save(QStringLiteral("02-02-02-RouteNonEmpty.png"));

        emit requestOpenFlightRouteAddWPDialog();
        delay(2s);
        applicationWindow->grabWindow().save(QStringLiteral("02-02-03-AddWP.png"));
        emit requestClosePages();
        GlobalObject::navigator()->flightRoute()->clear();
    }

    // Enroute near EDSB
    {
        qWarning() << "… En route near EDSB";
        trafficSimulator->setCoordinate( {48.8442094, 8.45, Units::Distance::fromFT(7512).toM()} );
        trafficSimulator->setBarometricHeight( Units::Distance::fromFT(7480) );
        trafficSimulator->setTT( Units::Angle::fromDEG(30) );
        trafficSimulator->setGS( Units::Speed::fromKN(91) );

        GlobalObject::navigator()->flightRoute()->clear();
        GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("EDTL")) );
        GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("KRH")) );
        GlobalObject::navigator()->flightRoute()->append( GlobalObject::geoMapProvider()->findByID(QStringLiteral("EDTY")) );

        flightMap->setProperty("zoomLevel", 11);
        GlobalObject::settings()->setMapBearingPolicy(Settings::TTUp);
        delay(4s);
        applicationWindow->grabWindow().save(QStringLiteral("02-02-04-EnRoute.png"));
        GlobalObject::navigator()->flightRoute()->clear();
    }

    // Aircraft page
    {
        qWarning() << "… Aircraft Page";
        emit requestOpenAircraftPage();
        delay(2s);
        applicationWindow->grabWindow().save(QStringLiteral("01-03-04-Aircraft.png"));
        emit requestClosePages();
    }

    // Nearby waypoints
    {
        qWarning() << "… Nearby Waypoints Page";
        emit requestOpenNearbyPage();
        delay(2s);
        applicationWindow->grabWindow().save(QStringLiteral("02-02-01-Nearby.png"));
        emit requestClosePages();
    }

    // Weather
    {
        qWarning() << "… Weather Page";
        emit requestOpenWeatherPage();
        delay(2s);
        applicationWindow->grabWindow().save(QStringLiteral("02-03-01-Weather.png"));
    }

    // Weather Dialog
    {
        qWarning() << "… Weather Dialog";
        auto *weatherReport = findQQuickItem(QStringLiteral("weatherReport"), m_engine);
        Q_ASSERT(weatherReport != nullptr);
        auto *station = GlobalObject::weatherDataProvider()->findWeatherStation(QStringLiteral("LSZB"));
        Q_ASSERT(station != nullptr);
        weatherReport->setProperty("weatherStation", QVariant::fromValue(station));
        QMetaObject::invokeMethod(weatherReport, "open", Qt::QueuedConnection);
        delay(4s);
        applicationWindow->grabWindow().save(QStringLiteral("02-03-02-WeatherDialog.png"));
        emit requestClosePages();
    }

    // EDTF Taxiway
    {
        qWarning() << "… EDTF Taxiway";
        trafficSimulator->setCoordinate( {48.02197, 7.83451, 240} );
        trafficSimulator->setBarometricHeight( Units::Distance::fromFT(800) );
        trafficSimulator->setTT( Units::Angle::fromDEG(160) );
        trafficSimulator->setGS( Units::Speed::fromKN(5) );
        flightMap->setProperty("zoomLevel", 13);
        flightMap->setProperty("followGPS", true);
        GlobalObject::settings()->setMapBearingPolicy(Settings::NUp);
        delay(4s);
        applicationWindow->grabWindow().save(QStringLiteral("01-03-01-ground.png"));
    }

    // Approaching EDDR
    {
        qWarning() << "… Approaching EDDR";
        trafficSimulator->setCoordinate( {49.35, 7.0028, Units::Distance::fromFT(5500).toM()} );
        trafficSimulator->setBarometricHeight( Units::Distance::fromFT(5500) );
        trafficSimulator->setTT( Units::Angle::fromDEG(170) );
        trafficSimulator->setGS( Units::Speed::fromKN(90) );
        flightMap->setProperty("zoomLevel", 11);
        GlobalObject::settings()->setMapBearingPolicy(Settings::TTUp);
        delay(4s);
        applicationWindow->grabWindow().save(QStringLiteral("01-03-02-flight.png"));
    }

    // Egelsbach Airport Info
    {
        qWarning() << "… EDFE Info Page";
        auto waypoint = GlobalObject::geoMapProvider()->findByID(QStringLiteral("EDFE"));
        Q_ASSERT(waypoint.isValid());
        waypointDescription->setProperty("waypoint", QVariant::fromValue(waypoint));
        QMetaObject::invokeMethod(waypointDescription, "open", Qt::QueuedConnection);
        GlobalObject::settings()->setMapBearingPolicy(Settings::NUp);
        delay(4s);
        applicationWindow->grabWindow().save(QStringLiteral("01-03-03-EDFEinfo.png"));
        QMetaObject::invokeMethod(waypointDescription, "close", Qt::QueuedConnection);
    }

    // Approaching EDTF w/ traffic
    {
        qWarning() << "… EDTF Traffic";
        QGeoCoordinate ownPosition(48.00144, 7.76231, 604);
        trafficSimulator->setCoordinate( ownPosition );
        trafficSimulator->setBarometricHeight( Units::Distance::fromM(600) );
        trafficSimulator->setTT( Units::Angle::fromDEG(41) );
        trafficSimulator->setGS( Units::Speed::fromKN(92) );
        flightMap->setProperty("zoomLevel", 13);
        flightMap->setProperty("followGPS", true);
        GlobalObject::settings()->setMapBearingPolicy(Settings::TTUp);
        QGeoCoordinate trafficPosition(48.0103, 7.7952, 540);
        QGeoPositionInfo trafficInfo;
        trafficInfo.setCoordinate(trafficPosition);
        trafficInfo.setAttribute(QGeoPositionInfo::Direction, 160);
        trafficInfo.setAttribute(QGeoPositionInfo::GroundSpeed, Units::Speed::fromKN(70).toMPS() );
        trafficInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, -2);
        trafficInfo.setTimestamp( QDateTime::currentDateTimeUtc() );
        auto* trafficFactor1 = new Traffic::TrafficFactor_WithPosition(this);
        trafficFactor1->setAlarmLevel(0);
        trafficFactor1->setID(QStringLiteral("newID"));
        trafficFactor1->setType(Traffic::TrafficFactor_Abstract::Aircraft);
        trafficFactor1->setPositionInfo(trafficInfo);
        trafficFactor1->setHDist( Units::Distance::fromM(1000) );
        trafficFactor1->setVDist( Units::Distance::fromM(17) );
        trafficSimulator->addTraffic(trafficFactor1);

        auto* trafficFactor2 = new Traffic::TrafficFactor_DistanceOnly(this);
        trafficFactor2->setAlarmLevel(1);
        trafficFactor2->setID(QStringLiteral("newID"));
        trafficFactor2->setHDist( Units::Distance::fromM(1000) );
        trafficFactor2->setType( Traffic::TrafficFactor_Abstract::Aircraft );
        trafficFactor2->setCallSign({});
        trafficFactor2->setCoordinate(ownPosition);
        trafficSimulator->setTrafficFactor_DistanceOnly(trafficFactor2);

        delay(4s);
        applicationWindow->grabWindow().save(QStringLiteral("02-01-01-traffic.png"));
        trafficFactor1->setHDist( {} );
        trafficSimulator->removeTraffic();
        trafficSimulator->setTrafficFactor_DistanceOnly( nullptr );
        delay(40s);
    }

    // Done. Terminate the program.
    QApplication::exit();
}


void DemoRunner::setLanguage(const QString &language){

    Q_ASSERT(m_engine != nullptr);

    // Delete existing translators
    foreach(auto translator, qApp->findChildren<QTranslator*>()) {
        if (QCoreApplication::removeTranslator(translator)) {
            delete translator;
        }
    }

    auto* enrouteTranslator = new QTranslator(qApp);
    if ( enrouteTranslator->load(QStringLiteral(":enroute_%1.qm").arg(language.left(2))) ) {
        QCoreApplication::installTranslator(enrouteTranslator);
    }
    m_engine->retranslate();

}
