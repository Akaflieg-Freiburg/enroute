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
#include <chrono>

#include "DemoRunner.h"
#include "GlobalObject.h"
#include "Settings.h"
#include "geomaps/GeoMapProvider.h"
#include "navigation/Navigator.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_Simulate.h"
#include "traffic/TrafficFactor_WithPosition.h"

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
    auto* applicationWindow =  qobject_cast<QQuickWindow*>(findQQuickItem("applicationWindow", m_engine));
    Q_ASSERT(applicationWindow != nullptr);
    auto* flightMap = findQQuickItem("flightMap", m_engine);
    Q_ASSERT(flightMap != nullptr);
    auto *waypointDescription = findQQuickItem("waypointDescription", m_engine);
    Q_ASSERT(waypointDescription != nullptr);

    // Set up traffic simulator
    auto* trafficSimulator = new Traffic::TrafficDataSource_Simulate();
    GlobalObject::trafficDataProvider()->addDataSource( trafficSimulator );
    trafficSimulator->connectToTrafficReceiver();
    delay(10s);

    qWarning() << "Demo Mode" << "Running Demo";

    // Resize window
    qWarning() << "Demo Mode" << "Resize window";
    applicationWindow->setProperty("width", 400);
    applicationWindow->setProperty("height", 600);

    // Set language
    GlobalObject::settings()->installTranslators("en");
    m_engine->retranslate();

    // Clear flight route
    GlobalObject::navigator()->flightRoute()->clear();

    // Nearby waypoints
    {
        qWarning() << "Demo Mode" << "Nearby Waypoints Page";
        emit requestOpenNearbyPage();
        delay(4s);
        applicationWindow->grabWindow().save("02-02-01-Nearby.png");
        emit requestClosePages();
    }

    // EDTF Taxiway
    {
        qWarning() << "Demo Mode" << "EDTF Taxiway";
        trafficSimulator->setCoordinate( {48.02197, 7.83451, 240} );
        trafficSimulator->setBarometricHeight( Units::Distance::fromFT(800) );
        trafficSimulator->setTT( Units::Angle::fromDEG(160) );
        trafficSimulator->setGS( Units::Speed::fromKN(5) );
        flightMap->setProperty("zoomLevel", 13);
        flightMap->setProperty("followGPS", true);
        GlobalObject::settings()->setMapBearingPolicy(Settings::NUp);
        delay(4s);
        applicationWindow->grabWindow().save("01-03-01-ground.png");
    }

    // Approaching EDDR
    {
        qWarning() << "Demo Mode" << "Approaching EDDR";
        trafficSimulator->setCoordinate( {49.35, 7.0028, Units::Distance::fromFT(5500).toM()} );
        trafficSimulator->setBarometricHeight( Units::Distance::fromFT(5500) );
        trafficSimulator->setTT( Units::Angle::fromDEG(170) );
        trafficSimulator->setGS( Units::Speed::fromKN(90) );
        flightMap->setProperty("zoomLevel", 11);
        GlobalObject::settings()->setMapBearingPolicy(Settings::TTUp);
        delay(4s);
        applicationWindow->grabWindow().save("01-03-02-flight.png");
    }

    // Egelsbach Airport Info
    {
        qWarning() << "Demo Mode" << "EDFE Info Page";
        auto waypoint = GlobalObject::geoMapProvider()->findByID("EDFE");
        Q_ASSERT(waypoint.isValid());
        waypointDescription->setProperty("waypoint", QVariant::fromValue(waypoint));
        QMetaObject::invokeMethod(waypointDescription, "open", Qt::QueuedConnection);
        GlobalObject::settings()->setMapBearingPolicy(Settings::NUp);
        delay(4s);
        applicationWindow->grabWindow().save("01-03-03-EDFEinfo.png");
        QMetaObject::invokeMethod(waypointDescription, "close", Qt::QueuedConnection);
    }

    // Approaching EDTF w/ traffic
    {
        qWarning() << "Demo Mode" << "EDTF Traffic";
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
        trafficFactor1->setID("newID");
        trafficFactor1->setType(Traffic::TrafficFactor_Abstract::Aircraft);
        trafficFactor1->setPositionInfo(trafficInfo);
        trafficFactor1->setHDist( Units::Distance::fromM(1000) );
        trafficFactor1->setVDist( Units::Distance::fromM(17) );
        trafficSimulator->addTraffic(trafficFactor1);

        auto* trafficFactor2 = new Traffic::TrafficFactor_DistanceOnly(this);
        trafficFactor2->setAlarmLevel(1);
        trafficFactor2->setID("newID");
        trafficFactor2->setHDist( Units::Distance::fromM(1000) );
        trafficFactor2->setType( Traffic::TrafficFactor_Abstract::Aircraft );
        trafficFactor2->setCallSign({});
        trafficFactor2->setCoordinate(ownPosition);
        trafficSimulator->setTrafficFactor_DistanceOnly(trafficFactor2);

        delay(4s);
        applicationWindow->grabWindow().save("02-01-01-traffic.png");
        trafficFactor1->setHDist( {} );
        trafficSimulator->removeTraffic();
        trafficSimulator->setTrafficFactor_DistanceOnly( nullptr );
        delay(40s);

    }

    // Done. Terminate the program.
    QApplication::exit();
}
