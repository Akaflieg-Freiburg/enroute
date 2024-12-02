/***************************************************************************
 *   Copyright (C) 2021-2024 by Stefan Kebekus                             *
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


#include "traffic/TrafficDataSource_Simulate.h"


// Member functions

Traffic::TrafficDataSource_Simulate::TrafficDataSource_Simulate(bool isCanonical, QObject* parent) :
    TrafficDataSource_Abstract(isCanonical, parent)
{
    simulatorTimer.setInterval(1s);
    simulatorTimer.setSingleShot(false);
    connect(&simulatorTimer, &QTimer::timeout, this, &Traffic::TrafficDataSource_Simulate::sendSimulatorData);

    // Initially, set properties
    TrafficDataSource_Simulate::disconnectFromTrafficReceiver();
}


void Traffic::TrafficDataSource_Simulate::connectToTrafficReceiver()
{
    // Do not do anything if the traffic receiver is connected and is receiving.
    if (receivingHeartbeat()) {
        return;
    }

    setConnectivityStatus( tr("Connected.") );
    simulatorTimer.start();
}


void Traffic::TrafficDataSource_Simulate::disconnectFromTrafficReceiver()
{
    setConnectivityStatus( tr("Not connected.") );
    setReceivingHeartbeat(false);
    simulatorTimer.stop();
}


void Traffic::TrafficDataSource_Simulate::sendSimulatorData()
{

    geoInfo.setTimestamp( QDateTime::currentDateTimeUtc() );
    if (geoInfo.isValid()) {
        emit positionUpdated( Positioning::PositionInfo(geoInfo) );
        setReceivingHeartbeat(true);
    } else {
        setReceivingHeartbeat(false);
    }

    foreach(TrafficFactor_WithPosition* trafficFactor, trafficFactors) {
        if (trafficFactor == nullptr) {
            continue;
        }

        trafficFactor->startLiveTime();
        if (trafficFactor->valid()) {
            emit factorWithPosition(*trafficFactor);
        }
    }

    if (!trafficFactor_DistanceOnly.isNull()) {
        emit factorWithoutPosition(*trafficFactor_DistanceOnly);
    }

    setPressureAltitude(barometricHeight);
}
