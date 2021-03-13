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
#include "traffic/FileTrafficDataSource.h"
#include "traffic/TrafficDataManager.h"

using namespace std::chrono_literals;

// Static instance of this class. Do not analyze, because of many unwanted warnings.
#ifndef __clang_analyzer__
Q_GLOBAL_STATIC(Traffic::TrafficDataProvider, TrafficDataManagerStatic);
#endif


// Member functions

Traffic::TrafficDataProvider::TrafficDataProvider(QObject *parent) : QObject(parent) {

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


    // Setup Data Sources

    // Uncomment one of the lines below to start this class in simulation mode.
    //    simulatorFileName = "/home/kebekus/Software/standards/FLARM/helluva_lot_aircraft.txt";
    //    simulatorFileName = "/home/kebekus/Software/standards/FLARM/expiry-hard.txt";
    //    simulatorFileName = "/home/kebekus/Software/standards/FLARM/expiry-soft.txt";
    //    simulatorFileName = "/home/kebekus/Software/standards/FLARM/many_opponents.txt";
    //    simulatorFileName= "/home/kebekus/Software/standards/FLARM/obstacles_from_gurtnellen_to_lake_constance.txt";
    //    simulatorFileName = "/home/kebekus/Software/standards/FLARM/single_opponent.txt";
    //    simulatorFileName = "/home/kebekus/Software/standards/FLARM/single_opponent_mode_s.txt";


    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/single_opponent.txt", this);
    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/single_opponent_mode_s.txt", this);
    _dataSources << new Traffic::FileTrafficDataSource("inex.txt", this);

    // Wire up data sources
    foreach(auto dataSource, _dataSources) {
        if (dataSource.isNull()) {
            continue;
        }

        connect(dataSource, &Traffic::AbstractTrafficDataSource::connectivityStatusChanged, this, &Traffic::TrafficDataProvider::statusStringChanged);
        connect(dataSource, &Traffic::AbstractTrafficDataSource::hasHeartbeatChanged, this, &Traffic::TrafficDataProvider::statusStringChanged);
        connect(dataSource, &Traffic::AbstractTrafficDataSource::hasHeartbeatChanged, this, &Traffic::TrafficDataProvider::onSourceHeartbeatChanged);
        connect(dataSource, &Traffic::AbstractTrafficDataSource::hasHeartbeatChanged, this, &Traffic::TrafficDataProvider::receivingChanged);
        connect(dataSource, &Traffic::AbstractTrafficDataSource::errorStringChanged, this, &Traffic::TrafficDataProvider::statusStringChanged);
        connect(dataSource, &Traffic::AbstractTrafficDataSource::factorWithoutPosition, this, &Traffic::TrafficDataProvider::onFactorWithoutPosition);
        connect(dataSource, &Traffic::AbstractTrafficDataSource::factorWithPosition, this, &Traffic::TrafficDataProvider::onFactorWithPosition);

    }
}


void Traffic::TrafficDataProvider::connectToTrafficReceiver()
{

    foreach(auto dataSource, _dataSources) {
        if (dataSource.isNull()) {
            continue;
        }
        dataSource->connectToTrafficReceiver();
    }

}


void Traffic::TrafficDataProvider::disconnectFromTrafficReceiver()
{
    foreach(auto dataSource, _dataSources) {
        if (dataSource.isNull()) {
            continue;
        }
        dataSource->disconnectFromTrafficReceiver();
    }
}


auto Traffic::TrafficDataProvider::globalInstance() -> Traffic::TrafficDataProvider *
{
#ifndef __clang_analyzer__
    return TrafficDataManagerStatic;
#else
    return nullptr;
#endif
}


void Traffic::TrafficDataProvider::onFactorWithPosition(Traffic::Factor *factor)
{
    if (factor == nullptr) {
        return;
    }

    foreach(auto target, _trafficObjects)
        if (factor->ID() == target->ID()) {
            target->copyFrom(*factor);
            return;
        }

    auto *lowestPriObject = _trafficObjects.at(0);
    foreach(auto target, _trafficObjects)
        if (lowestPriObject->hasHigherPriorityThan(*target)) {
            lowestPriObject = target;
        }
    if (factor->hasHigherPriorityThan(*lowestPriObject)) {
        lowestPriObject->copyFrom(*factor);
    }

}


void Traffic::TrafficDataProvider::onFactorWithoutPosition(Traffic::Factor *factor)
{
    if (factor == nullptr) {
        return;
    }

    if ((factor->ID() == _trafficObjectWithoutPosition->ID()) || factor->hasHigherPriorityThan(*_trafficObjectWithoutPosition)) {
        _trafficObjectWithoutPosition->copyFrom(*factor);
    }

}


void Traffic::TrafficDataProvider::onSourceHeartbeatChanged()
{

    Traffic::AbstractTrafficDataSource *heartbeatDataSource = nullptr;
    foreach(auto source, _dataSources) {
        if (source.isNull()) {
            continue;
        }

        if (source->hasHeartbeat()) {
            heartbeatDataSource = source;
            break;
        }
    }

    if (heartbeatDataSource == nullptr) {
        return;
    }

    foreach(auto source, _dataSources) {
        if (source.isNull()) {
            continue;
        }

        if (source != heartbeatDataSource) {
            source->disconnectFromTrafficReceiver();
        }

    }

}


auto Traffic::TrafficDataProvider::receiving() const -> bool
{

    foreach(auto source, _dataSources) {
        if (source.isNull()) {
            continue;
        }

        if (source->hasHeartbeat()) {
            return true;
        }
    }

    return false;

}


auto Traffic::TrafficDataProvider::statusString() const -> QString
{

    QString result = "<ul>";
    foreach(auto source, _dataSources) {
        if (source.isNull()) {
            continue;
        }

        result += "<li>";
        result += source->sourceName() + ": ";
        if (source->hasHeartbeat()) {
            result += tr("Receiving traffic data.");
        } else {
            switch(source->connectivityStatus()) {
            case Traffic::AbstractTrafficDataSource::Disconnected:
                result += tr("Not connected.");
                break;
            case Traffic::AbstractTrafficDataSource::Connecting:
                result += tr("Connecting.");
                break;
            case Traffic::AbstractTrafficDataSource::Connected:
                result += tr("Connected, but not receiving traffic data.");
                break;
            }
        }
        if (!source->errorString().isEmpty()) {
            result += " " + source->errorString();
        }
        result += "</li>";
    }
    result += "</ul>";
    return result;

}

