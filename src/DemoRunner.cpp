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
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QTimer>
#include <chrono>

#include "DemoRunner.h"
#include "Global.h"
#include "Settings.h"
#include "geomaps/GeoMapProvider.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_Simulate.h"
#include "traffic/TrafficFactor.h"

using namespace std::chrono_literals;


DemoRunner::DemoRunner(QObject *parent) : QObject(parent) {
    qWarning() << "DemoRunner Initialisation";

    QTimer::singleShot(10s, this, &DemoRunner::run);
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
    auto *engine = qobject_cast<QQmlApplicationEngine*>(parent());
    Q_ASSERT(engine != nullptr);

    // Obtain pointers to QML items
    auto* applicationWindow =  qobject_cast<QQuickWindow*>(findQQuickItem("applicationWindow", engine));
    Q_ASSERT(applicationWindow != nullptr);
    auto* flightMap = findQQuickItem("flightMap", engine);
    Q_ASSERT(flightMap != nullptr);
    auto *waypointDescription = findQQuickItem("waypointDescription", engine);
    Q_ASSERT(waypointDescription != nullptr);

    // Set up traffic simulator
    auto* trafficSimulator = new Traffic::TrafficDataSource_Simulate();
    Global::trafficDataProvider()->addDataSource( trafficSimulator );
    trafficSimulator->connectToTrafficReceiver();

    qWarning() << "Demo Mode" << "Running Demo";

    // Resize window
    qWarning() << "Demo Mode" << "Resize window";
    applicationWindow->setProperty("width", 400);
    applicationWindow->setProperty("height", 600);

    // Set language
    Global::settings()->installTranslators("en");
    engine->retranslate();

    /*
    // EDTF Taxiway   
    qWarning() << "Demo Mode" << "EDTF Taxiway";
    trafficSimulator->setCoordinate( {48.02197, 7.83451, 240} );
    trafficSimulator->setBarometricHeight( AviationUnits::Distance::fromFT(800) );
    trafficSimulator->setTT( AviationUnits::Angle::fromDEG(160) );
    trafficSimulator->setGS( AviationUnits::Speed::fromKN(5) );
    flightMap->setProperty("zoomLevel", 13);
    flightMap->setProperty("followGPS", true);
    Global::settings()->setMapBearingPolicy(Settings::NUp);
    delay(4s);
    applicationWindow->grabWindow().save("01-03-01-ground.png");

    // Approaching EDDR
    qWarning() << "Demo Mode" << "Approaching EDDR";
    trafficSimulator->setCoordinate( {49.35, 7.0028, AviationUnits::Distance::fromFT(5500).toM()} );
    trafficSimulator->setBarometricHeight( AviationUnits::Distance::fromFT(5500) );
    trafficSimulator->setTT( AviationUnits::Angle::fromDEG(170) );
    trafficSimulator->setGS( AviationUnits::Speed::fromKN(90) );
    flightMap->setProperty("zoomLevel", 11);
    Global::settings()->setMapBearingPolicy(Settings::TTUp);
    delay(4s);
    applicationWindow->grabWindow().save("01-03-02-flight.png");

    // Stuttgart Airport Info
    qWarning() << "Demo Mode" << "EDFE Info Page";
    auto waypoint = Global::geoMapProvider()->findByID("EDFE");
    Q_ASSERT(waypoint.isValid());
    waypointDescription->setProperty("waypoint", QVariant::fromValue(waypoint));
    QMetaObject::invokeMethod(waypointDescription, "open", Qt::QueuedConnection);
    Global::settings()->setMapBearingPolicy(Settings::NUp);
    delay(4s);
    applicationWindow->grabWindow().save("01-03-03-EDFEinfo.png");
*/

    // Approaching EDTF w/ traffic
    qWarning() << "Demo Mode" << "EDTF Traffic";
    trafficSimulator->setCoordinate( {48.02197, 7.83451, 240} );
    trafficSimulator->setBarometricHeight( AviationUnits::Distance::fromFT(800) );
    trafficSimulator->setTT( AviationUnits::Angle::fromDEG(160) );
    trafficSimulator->setGS( AviationUnits::Speed::fromKN(5) );
    flightMap->setProperty("zoomLevel", 13);
    flightMap->setProperty("followGPS", true);
    Global::settings()->setMapBearingPolicy(Settings::NUp);
    delay(4s);
    applicationWindow->grabWindow().save("01-03-01-ground.png");

    Traffic::TrafficFactor trafficFactor;
    QGeoPositionInfo trafficInfo;
    trafficFactor.setData(1, //int newAlarmLevel,
                          "newId", //const QString & newID,
                          AviationUnits::Distance::fromM(100), // newHDist,
                          AviationUnits::Distance::fromM(100), // newVDist,
                          Traffic::TrafficFactor::Aircraft, // newType,
                          {},
                          {} //const QString & newCallSign
                          );

    // Done. Terminate the program.
    //QApplication::exit();
}


