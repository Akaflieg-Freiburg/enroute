/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <QQmlEngine>
#include <chrono>

#include "MobileAdaptor.h"
#include "Navigation_SatNav.h"
#include "traffic/TrafficDataManager.h"

using namespace std::chrono_literals;


// Member functions

Traffic::TrafficDataManager::TrafficDataManager(QObject *parent) : QObject(parent) {

    // Create traffic objects
    int numTrafficObjects = 20;
    _trafficObjects.reserve(numTrafficObjects);
    for(int i = 0; i<numTrafficObjects; i++) {
        auto *trafficObject = new Traffic::Factor(this);
        QQmlEngine::setObjectOwnership(trafficObject, QQmlEngine::CppOwnership);
        _trafficObjects.append( trafficObject );
    }
    _trafficObjectWithoutPosition = new Traffic::Factor(this);
    QQmlEngine::setObjectOwnership(_trafficObjectWithoutPosition, QQmlEngine::CppOwnership);

    // Setup timers for property updates
    receivingHeartbeatTimer.setSingleShot(true);
    receivingHeartbeatTimer.setInterval(5s);
    receivingPositionDataTimer.setSingleShot(true);
    receivingPositionDataTimer.setInterval(5s);

    //
    // Property bindings
    //

    // Bind property "receivingBarometricAltData"
    connect(&receivingBarometricAltDataTimer, &QTimer::timeout, this, &Traffic::TrafficDataManager::updateReceivingBarometricAltData);

    // Bind property "receivingPositionData"
    connect(&receivingPositionDataTimer, &QTimer::timeout, this, &Traffic::TrafficDataManager::updateReceivingPositionData);
}


void Traffic::TrafficDataManager::processFLARMMessage(const QString & /*unused*/)
{
}


void Traffic::TrafficDataManager::updateReceivingBarometricAltData()
{
    auto newReceivingBarometricAltData = receivingBarometricAltDataTimer.isActive();

    // Update property and emit signal if necessary
    if (_receivingBarometricAltData == newReceivingBarometricAltData) {
        return;
    }
    _receivingBarometricAltData = newReceivingBarometricAltData;
    emit receivingBarometricAltDataChanged(_receivingBarometricAltData);
}


void Traffic::TrafficDataManager::updateReceivingPositionData()
{
    auto newReceivingPositionData = receivingPositionDataTimer.isActive();

    // Update property and emit signal if necessary
    if (_receivingPositionData == newReceivingPositionData) {
        return;
    }
    _receivingPositionData = newReceivingPositionData;
    emit receivingPositionDataChanged(_receivingPositionData);
}
