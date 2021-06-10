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
#include <QTimer>
#include <chrono>

#include "DemoRunner.h"
#include "Global.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_Simulate.h"

using namespace std::chrono_literals;


DemoRunner::DemoRunner(QObject *parent) : QObject(parent) {
    qWarning() << "DemoRunner Initialisation";

    auto *engine = qobject_cast<QQmlApplicationEngine*>(parent);
    Q_ASSERT(engine != nullptr);

    // Save settings
    // Obtain a pointer to the flightMap
    QQuickItem *flightMap = nullptr;
    foreach (auto rootItem, engine->rootObjects()) {
        flightMap = rootItem->findChild<QQuickItem*>("flightMap");
        if (flightMap != nullptr) {
            break;
        }
    }
    Q_ASSERT(flightMap != nullptr);


    QTimer::singleShot(4s, this, &DemoRunner::run);
}


void delay(std::chrono::milliseconds ms)
{
    QEventLoop loop; // define a new event loop
    QTimer::singleShot(ms, &loop, &QEventLoop::quit);
    loop.exec();
}


void DemoRunner::run()
{
    // Set up traffic simulator
    auto* trafficSimulator = new Traffic::TrafficDataSource_Simulate();
    Global::trafficDataProvider()->addDataSource( trafficSimulator );
    trafficSimulator->connectToTrafficReceiver();

    qWarning() << "Running Demo";

    emit resizeMainWindow(400, 600);

    // EDTF Taxiway
    trafficSimulator->setCoordinate( {48.02197, 7.83451, 240} );
    trafficSimulator->setBarometricHeight( AviationUnits::Distance::fromFT(800) );
    trafficSimulator->setTT( AviationUnits::Angle::fromDEG(160) );
    trafficSimulator->setGS( AviationUnits::Speed::fromKN(5) );
    emit zoom(13); // Set maximum zoom level
    delay(2s);
    emit saveImage("Ground.png");

/*
 *     // EDTF Downwind Runway 34
    trafficSimulator->setCoordinate( {47.99992, 7.80979, 550} );
    trafficSimulator->setBarometricHeight( AviationUnits::Distance::fromFT(1800) );
    trafficSimulator->setTT( AviationUnits::Angle::fromDEG(120) );
    trafficSimulator->setGS( AviationUnits::Speed::fromKN(75) );
    delay(2s);
    emit saveImage("demo.png");
    */
}


