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
#include "positioning/PositionProvider.h"
#include "traffic/FileTrafficDataSource.h"
#include "traffic/TcpTrafficDataSource.h"
#include "traffic/TrafficDataProvider.h"

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


    // Position Info Timer
    positionInfoTimer.setInterval(5s);
    positionInfoTimer.setSingleShot(true);
    connect(&positionInfoTimer, &QTimer::timeout, this, &Traffic::TrafficDataProvider::onPositionInfoTimeout);


    // Setup Data Sources

    // Uncomment one of the lines below to start this class in simulation mode.
//    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/expiry-hard.txt", this);
//    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/expiry-soft.txt", this);
//    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/helluva_lot_aircraft.txt", this);
//    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/many_opponents.txt", this);
//    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/obstacles_from_gurtnellen_to_lake_constance.txt", this);
//    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/single_opponent.txt", this);
    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/single_opponent_mode_s.txt", this);
//    _dataSources << new Traffic::FileTrafficDataSource("/home/kebekus/Software/standards/FLARM/single_opponent.txt", this);
    _dataSources << new Traffic::TcpTrafficDataSource("192.168.1.1", 2000, this);
    _dataSources << new Traffic::TcpTrafficDataSource("192.168.10.1", 2000, this);

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
        connect(dataSource, &Traffic::AbstractTrafficDataSource::positionUpdated, this, &Traffic::TrafficDataProvider::onPositionInfoUpdate);

        connect(this, &Traffic::TrafficDataProvider::positionInfoChanged, this, &Traffic::TrafficDataProvider::statusStringChanged);

    }

    // Connect timer. Try to (re)connect after 2s, and then again every five minutes.
    QTimer::singleShot(2s, this, &Traffic::TrafficDataProvider::connectToTrafficReceiver);
    connect(&reconnectionTimer, &QTimer::timeout, this, &Traffic::TrafficDataProvider::connectToTrafficReceiver);
    reconnectionTimer.setInterval(5min);
    reconnectionTimer.setSingleShot(false);
    reconnectionTimer.start();

    // Try to (re)connect whenever the network situation changes
    auto* mobileAdaptor = MobileAdaptor::globalInstance();
    if (mobileAdaptor != nullptr) {
        connect(mobileAdaptor, &MobileAdaptor::wifiConnected, this, &Traffic::TrafficDataProvider::connectToTrafficReceiver);
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


void Traffic::TrafficDataProvider::onFactorWithPosition(const Traffic::Factor &factor)
{
    foreach(auto target, _trafficObjects)
        if (factor.ID() == target->ID()) {
            target->copyFrom(factor);
            return;
        }

    auto *lowestPriObject = _trafficObjects.at(0);
    foreach(auto target, _trafficObjects)
        if (lowestPriObject->hasHigherPriorityThan(*target)) {
            lowestPriObject = target;
        }
    if (factor.hasHigherPriorityThan(*lowestPriObject)) {
        lowestPriObject->copyFrom(factor);
    }

}


void Traffic::TrafficDataProvider::onFactorWithoutPosition(const Traffic::Factor &factor)
{
    if ((factor.ID() == _trafficObjectWithoutPosition->ID()) || factor.hasHigherPriorityThan(*_trafficObjectWithoutPosition)) {
        _trafficObjectWithoutPosition->copyFrom(factor);
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

    foreach(auto source, _dataSources) {
        if (source.isNull()) {
            continue;
        }
        if (source->hasHeartbeat()) {
            QString result = QString("<p>%1</p><ul style='margin-left:-25px;'>").arg(source->sourceName());
            result += QString("<li>%1</li>").arg(tr("Receiving traffic data."));
            if (_positionInfo.isValid()) {
                result += QString("<li>%1</li>").arg(tr("Receiving position info."));
            }
            result += "</ul>";
            return result;
        }
    }

#warning Might receive baro alt
#warning might receive position info
    QString result = "<p>" + tr("Not receiving traffic data.") + "<p><ul style='margin-left:-25px;'>";
    foreach(auto source, _dataSources) {
        if (source.isNull()) {
            continue;
        }

        result += "<li>";
        result += source->sourceName() + ": " + source->connectivityStatus();
        if (!source->errorString().isEmpty()) {
            result += " " + source->errorString();
        }
        result += "</li>";
    }
    result += "</ul>";
    return result;

}


void Traffic::TrafficDataProvider::onPositionInfoUpdate(const QGeoPositionInfo& newGeoPositionInfo)
{
    bool positionInfoDidChange = true;
    if (!_positionInfo.isValid() && !newGeoPositionInfo.isValid()) {
        positionInfoDidChange = false;
}
    if (_positionInfo == newGeoPositionInfo) {
        positionInfoDidChange = false;
}

    if (_positionInfo.isValid()) {
        positionInfoTimer.start();
}

    if (positionInfoDidChange) {
        _positionInfo = newGeoPositionInfo;
        emit positionInfoChanged();
    }
}


void Traffic::TrafficDataProvider::onPositionInfoTimeout()
{
    onPositionInfoUpdate(QGeoPositionInfo());
}

