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
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_File.h"
#include "traffic/TrafficDataSource_Tcp.h"
#include "traffic/TrafficDataSource_Udp.h"

using namespace std::chrono_literals;

// Static instance of this class. Do not analyze, because of many unwanted warnings.
#ifndef __clang_analyzer__
Q_GLOBAL_STATIC(Traffic::TrafficDataProvider, TrafficDataManagerStatic);
#endif


// Member functions

Traffic::TrafficDataProvider::TrafficDataProvider(QObject *parent) : Positioning::PositionInfoSource_Abstract(parent) {

    // Create traffic objects
    int numTrafficObjects = 20;
    m_trafficObjects.reserve(numTrafficObjects);
    for(int i = 0; i<numTrafficObjects; i++) {
        auto *trafficObject = new Traffic::TrafficFactor(this);
        QQmlEngine::setObjectOwnership(trafficObject, QQmlEngine::CppOwnership);
        m_trafficObjects.append( trafficObject );
    }
    m_trafficObjectWithoutPosition = new Traffic::TrafficFactor(this);
    QQmlEngine::setObjectOwnership(m_trafficObjectWithoutPosition, QQmlEngine::CppOwnership);

    setSourceName(tr("Traffic data receiver"));

    // Setup FLARM warning
    m_WarningTimer.setInterval( Positioning::PositionInfo::lifetime );
    m_WarningTimer.setSingleShot(true);
    connect(&m_WarningTimer, &QTimer::timeout, this, &Traffic::TrafficDataProvider::resetWarning);



    // Setup Data Sources

    // Uncomment one of the lines below to start this class in simulation mode.
    // m_dataSources << new Traffic::TrafficDataSource_File("/home/kebekus/Software/standards/FLARM/expiry-hard.txt", this);
    // m_dataSources << new Traffic::TrafficDataSource_File("/home/kebekus/Software/standards/FLARM/expiry-soft.txt", this);
    // m_dataSources << new Traffic::TrafficDataSource_File("/home/kebekus/Software/standards/FLARM/helluva_lot_aircraft.txt", this);
    // m_dataSources << new Traffic::TrafficDataSource_File("/home/kebekus/Software/standards/FLARM/many_opponents.txt", this);
    // m_dataSources << new Traffic::TrafficDataSource_File("/home/kebekus/Software/standards/FLARM/obstacles_from_gurtnellen_to_lake_constance.txt", this);
    // m_dataSources << new Traffic::TrafficDataSource_File("/home/kebekus/Software/standards/FLARM/single_opponent.txt", this);
    // m_dataSources << new Traffic::TrafficDataSource_File("/home/kebekus/Software/standards/FLARM/single_opponent_mode_s.txt", this);
    // m_dataSources << new Traffic::TrafficDataSource_File("/home/kebekus/Software/standards/FLARM/single_opponent.txt", this);

    // Real data sources
    m_dataSources << new Traffic::TrafficDataSource_Tcp("192.168.1.1", 2000, this);
    m_dataSources << new Traffic::TrafficDataSource_Tcp("192.168.10.1", 2000, this);
    m_dataSources << new Traffic::TrafficDataSource_Udp(4000, this);

    // Wire up data sources
    foreach(auto dataSource, m_dataSources) {
        if (dataSource.isNull()) {
            continue;
        }

        connect(dataSource, &Traffic::TrafficDataSource_Abstract::pressureAltitudeUpdated, this, &Traffic::TrafficDataProvider::setPressureAltitude);
        connect(dataSource, &Traffic::TrafficDataSource_Abstract::connectivityStatusChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
        connect(dataSource, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
        connect(dataSource, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataProvider::onSourceHeartbeatChanged);
        connect(dataSource, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataProvider::receivingHeartbeatChanged);
        connect(dataSource, &Traffic::TrafficDataSource_Abstract::errorStringChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
        connect(dataSource, &Traffic::TrafficDataSource_Abstract::factorWithoutPosition, this, &Traffic::TrafficDataProvider::onTrafficFactorWithoutPosition);
        connect(dataSource, &Traffic::TrafficDataSource_Abstract::factorWithPosition, this, &Traffic::TrafficDataProvider::onTrafficFactorWithPosition);
        connect(dataSource, &Traffic::TrafficDataSource_Abstract::positionUpdated, this, &Traffic::TrafficDataProvider::setPositionInfo);
        connect(dataSource, &Traffic::TrafficDataSource_Abstract::warning, this, &Traffic::TrafficDataProvider::setWarning);
    }

    // Bindings for status string
    connect(this, &Traffic::TrafficDataProvider::positionInfoChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(this, &Traffic::TrafficDataProvider::pressureAltitudeChanged, this, &Traffic::TrafficDataProvider::updateStatusString);

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
    foreach(auto dataSource, m_dataSources) {
        if (dataSource.isNull()) {
            continue;
        }
        dataSource->connectToTrafficReceiver();
    }
}


void Traffic::TrafficDataProvider::disconnectFromTrafficReceiver()
{
    foreach(auto dataSource, m_dataSources) {
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


void Traffic::TrafficDataProvider::onSourceHeartbeatChanged()
{
    Traffic::TrafficDataSource_Abstract *heartbeatDataSource = nullptr;
    foreach(auto source, m_dataSources) {
        if (source.isNull()) {
            continue;
        }

        if (source->receivingHeartbeat()) {
            heartbeatDataSource = source;
            break;
        }
    }

    if (heartbeatDataSource == nullptr) {
        return;
    }

    foreach(auto source, m_dataSources) {
        if (source.isNull()) {
            continue;
        }

        if (source != heartbeatDataSource) {
            source->disconnectFromTrafficReceiver();
        }

    }
}


void Traffic::TrafficDataProvider::onTrafficFactorWithoutPosition(const Traffic::TrafficFactor &factor)
{
    if ((factor.ID() == m_trafficObjectWithoutPosition->ID()) || factor.hasHigherPriorityThan(*m_trafficObjectWithoutPosition)) {
        m_trafficObjectWithoutPosition->copyFrom(factor);
    }
}


void Traffic::TrafficDataProvider::onTrafficFactorWithPosition(const Traffic::TrafficFactor &factor)
{
    foreach(auto target, m_trafficObjects) {
        if (factor.ID() == target->ID()) {
            target->copyFrom(factor);
            return;
        }
    }

    auto *lowestPriObject = m_trafficObjects.at(0);
    foreach(auto target, m_trafficObjects) {
        if (lowestPriObject->hasHigherPriorityThan(*target)) {
            lowestPriObject = target;
        }
    }
    if (factor.hasHigherPriorityThan(*lowestPriObject)) {
        lowestPriObject->copyFrom(factor);
    }
}


auto Traffic::TrafficDataProvider::receivingHeartbeat() const -> bool
{
    foreach(auto source, m_dataSources) {
        if (source.isNull()) {
            continue;
        }

        if (source->receivingHeartbeat()) {
            return true;
        }
    }

    return false;
}


void Traffic::TrafficDataProvider::resetWarning()
{
    setWarning( Traffic::Warning() );
}


void Traffic::TrafficDataProvider::setWarning(const Traffic::Warning& warning)
{
    if (warning.alarmLevel() > -1) {
        m_WarningTimer.start();
    }

    if (m_Warning == warning) {
        return;
    }

    m_Warning = warning;
    emit warningChanged(m_Warning);
}


void Traffic::TrafficDataProvider::updateStatusString()
{
    foreach(auto source, m_dataSources) {
        if (source.isNull()) {
            continue;
        }
        if (source->receivingHeartbeat()) {
            QString result = QString("<p>%1</p><ul style='margin-left:-25px;'>").arg(source->sourceName());
            result += QString("<li>%1</li>").arg(tr("Receiving traffic data."));
            if (positionInfo().isValid()) {
                result += QString("<li>%1</li>").arg(tr("Receiving position info."));
            }
            if (pressureAltitude().isFinite()) {
                result += QString("<li>%1</li>").arg(tr("Receiving barometric altitude info."));
            }
            result += "</ul>";
            setStatusString(result);
            return;
        }
    }

    QString result = "<p>" + tr("Not receiving traffic data.") + "<p><ul style='margin-left:-25px;'>";
    foreach(auto source, m_dataSources) {
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

    setStatusString(result);
}
