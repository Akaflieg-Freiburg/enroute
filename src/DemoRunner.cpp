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
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_Simulate.h"

using namespace std::chrono_literals;


DemoRunner::DemoRunner(QObject *parent) : QObject(parent) {
    qWarning() << "DemoRunner Initialisation";

    QTimer::singleShot(4s, this, &DemoRunner::run);
}


void delay(std::chrono::milliseconds ms)
{
    QEventLoop loop; // define a new event loop
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}

QObject* findQQuickItem(const QString &objectName, QQmlApplicationEngine* engine)
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

    // Save settings
    // Obtain a pointer to the flightMap
    auto* applicationWindow =  qobject_cast<QQuickWindow*>(findQQuickItem("applicationWindow", engine));
    Q_ASSERT(applicationWindow != nullptr);
    QObject* flightMap = findQQuickItem("flightMap", engine);
    Q_ASSERT(flightMap != nullptr);

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

    // EDTF Taxiway   
    qWarning() << "Demo Mode" << "EDTF Taxiway";
    trafficSimulator->setCoordinate( {48.02197, 7.83451, 240} );
    trafficSimulator->setBarometricHeight( AviationUnits::Distance::fromFT(800) );
    trafficSimulator->setTT( AviationUnits::Angle::fromDEG(160) );
    trafficSimulator->setGS( AviationUnits::Speed::fromKN(5) );
    flightMap->setProperty("zoomLevel", 13);
    Global::settings()->setMapBearingPolicy(Settings::NUp);
    delay(4s);
    applicationWindow->grabWindow().save("Ground.png");

    // Approaching EDDR
    qWarning() << "Demo Mode" << "Approaching EDDR";
    trafficSimulator->setCoordinate( {49.35, 7.0028, AviationUnits::Distance::fromFT(5500).toM()} );
    trafficSimulator->setBarometricHeight( AviationUnits::Distance::fromFT(5500) );
    trafficSimulator->setTT( AviationUnits::Angle::fromDEG(170) );
    trafficSimulator->setGS( AviationUnits::Speed::fromKN(90) );
    flightMap->setProperty("zoomLevel", 11);
    Global::settings()->setMapBearingPolicy(Settings::TTUp);
    delay(4s);
    applicationWindow->grabWindow().save("Flight.png");

}


