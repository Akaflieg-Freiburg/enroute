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

#include <QCoreApplication>
#include <QQmlEngine>
#include <chrono>

#include "GlobalObject.h"
#include "platform/PlatformAdaptor_Abstract.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_Tcp.h"
#include "traffic/TrafficDataSource_Udp.h"

using namespace std::chrono_literals;


// Member functions

Traffic::TrafficDataProvider::TrafficDataProvider(QObject *parent) : Positioning::PositionInfoSource_Abstract(parent)
{

    // Create traffic objects
    const int numTrafficObjects = 20;
    m_trafficObjects.reserve(numTrafficObjects);
    for(int i = 0; i<numTrafficObjects; i++)
    {
        auto *trafficObject = new Traffic::TrafficFactor_WithPosition(this);
        QQmlEngine::setObjectOwnership(trafficObject, QQmlEngine::CppOwnership);
        m_trafficObjects.append( trafficObject );
    }
    m_trafficObjectWithoutPosition = new Traffic::TrafficFactor_DistanceOnly(this);
    QQmlEngine::setObjectOwnership(m_trafficObjectWithoutPosition, QQmlEngine::CppOwnership);

    setSourceName(tr("Traffic data receiver"));

    // Setup FLARM warning
    m_WarningTimer.setInterval( Positioning::PositionInfo::lifetime );
    m_WarningTimer.setSingleShot(true);
    connect(&m_WarningTimer, &QTimer::timeout, this, &Traffic::TrafficDataProvider::resetWarning);

    // Setup ForeFlight Broadcases
    foreFlightBroadcastTimer.setInterval(5s);
    connect(&foreFlightBroadcastTimer, &QTimer::timeout, this, &Traffic::TrafficDataProvider::foreFlightBroadcast);
    foreFlightBroadcastTimer.start();

    // Real data sources in order of preference, preferred sources first
    addDataSource( new Traffic::TrafficDataSource_Tcp(QStringLiteral("10.10.10.10"), 2000, this));
    addDataSource( new Traffic::TrafficDataSource_Tcp(QStringLiteral("192.168.1.1"), 2000, this));
    addDataSource( new Traffic::TrafficDataSource_Tcp(QStringLiteral("192.168.4.1"), 2000, this));
    addDataSource( new Traffic::TrafficDataSource_Tcp(QStringLiteral("192.168.10.1"), 2000, this) );
    addDataSource( new Traffic::TrafficDataSource_Tcp(QStringLiteral("192.168.42.1"), 2000, this) );
    addDataSource( new Traffic::TrafficDataSource_Udp(4000, this) );
    addDataSource( new Traffic::TrafficDataSource_Udp(49002, this));

    // Bindings for status string
    connect(this, &Traffic::TrafficDataProvider::positionInfoChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(this, &Traffic::TrafficDataProvider::pressureAltitudeChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(this, &Traffic::TrafficDataProvider::receivingHeartbeatChanged, this, &Traffic::TrafficDataProvider::updateStatusString);

    // Connect timer. Try to (re)connect after 2s, and then again every five minutes.
    QTimer::singleShot(2s, this, &Traffic::TrafficDataProvider::connectToTrafficReceiver);
    connect(&reconnectionTimer, &QTimer::timeout, this, &Traffic::TrafficDataProvider::connectToTrafficReceiver);
    reconnectionTimer.setInterval(5min);
    reconnectionTimer.setSingleShot(false);
    reconnectionTimer.start();

    // Try to (re)connect whenever the network situation changes
    QTimer::singleShot(0, this, &Traffic::TrafficDataProvider::deferredInitialization);

    // Clean up
    connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &Traffic::TrafficDataProvider::clearDataSources);

    // Wire up Bluetooth-related members
    m_bluetoothPermission.setCommunicationModes(QBluetoothPermission::Access);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(&m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &Traffic::TrafficDataProvider::onBTDeviceDiscovered);
}


void Traffic::TrafficDataProvider::clearDataSources()
{
    foreach(auto dataSource, m_dataSources)
    {
        if (dataSource.isNull())
        {
            continue;
        }
        dataSource->disconnect();
        delete dataSource;
    }
    m_dataSources.clear();
}


void Traffic::TrafficDataProvider::addDataSource(Traffic::TrafficDataSource_Abstract* source)
{
    Q_ASSERT( source != nullptr );

    source->setParent(this);
    m_dataSources << source;
    connect(source, &Traffic::TrafficDataSource_Abstract::connectivityStatusChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(source, &Traffic::TrafficDataSource_Abstract::errorStringChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(source, &Traffic::TrafficDataSource_Abstract::passwordRequest, this, &Traffic::TrafficDataProvider::passwordRequest);
    connect(source, &Traffic::TrafficDataSource_Abstract::passwordStorageRequest, this, &Traffic::TrafficDataProvider::passwordStorageRequest);
    connect(source, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(source, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataProvider::onSourceHeartbeatChanged);
    connect(source, &Traffic::TrafficDataSource_Abstract::trafficReceiverRuntimeErrorChanged, this, &Traffic::TrafficDataProvider::onTrafficReceiverRuntimeError);
    connect(source, &Traffic::TrafficDataSource_Abstract::trafficReceiverSelfTestErrorChanged, this, &Traffic::TrafficDataProvider::onTrafficReceiverSelfTestError);

    updateStatusString();
}


void Traffic::TrafficDataProvider::connectToTrafficReceiver()
{
    foreach(auto dataSource, m_dataSources)
    {
        if (dataSource.isNull())
        {
            continue;
        }
        dataSource->connectToTrafficReceiver();
    }

    switch (qApp->checkPermission(m_bluetoothPermission)) {
    case Qt::PermissionStatus::Undetermined:
        qApp->requestPermission(m_bluetoothPermission, this, [this]() { connectToTrafficReceiver(); });
        break;
    case Qt::PermissionStatus::Denied:
        break;
    case Qt::PermissionStatus::Granted:
        m_discoveryAgent.start();
        break;
    }
    updateStatusString();
}


void Traffic::TrafficDataProvider::deferredInitialization() const
{
    // Try to (re)connect whenever the network situation changes
    connect(GlobalObject::platformAdaptor(), &Platform::PlatformAdaptor_Abstract::wifiConnected, this, &Traffic::TrafficDataProvider::connectToTrafficReceiver);
}


void Traffic::TrafficDataProvider::disconnectFromTrafficReceiver()
{
    foreach(auto dataSource, m_dataSources)
    {
        if (dataSource.isNull())
        {
            continue;
        }
        dataSource->disconnectFromTrafficReceiver();
    }
}


void Traffic::TrafficDataProvider::foreFlightBroadcast()
{
    foreFlightBroadcastSocket.writeDatagram(foreFlightBroadcastDatagram);
}


void Traffic::TrafficDataProvider::onSourceHeartbeatChanged()
{
    // If we have a current source, if the current source has a heartbeat and if the current source is a TCP source, then we simply stick with it.
    if ((qobject_cast<Traffic::TrafficDataSource_Tcp*>(m_currentSource) != nullptr)
            && m_currentSource->receivingHeartbeat() )
    {
        setReceivingHeartbeat(true);
        return;
    }

    // Among the m_dataSources, find the first (=most preferred) source that is receiving heartbeat messages.
    Traffic::TrafficDataSource_Abstract *heartbeatDataSource = nullptr;
    foreach(auto source, m_dataSources)
    {
        if (source.isNull())
        {
            continue;
        }

        if (source->receivingHeartbeat())
        {
            heartbeatDataSource = source;
            break;
        }
    }

    // If the source has changed, then connect/disconnect old and new source, update m_currentSource
    if (heartbeatDataSource != m_currentSource) {


        // Disconnect old m_currentSource
        if (!m_currentSource.isNull())
        {
            disconnect(m_currentSource, &Traffic::TrafficDataSource_Abstract::pressureAltitudeUpdated, this, &Traffic::TrafficDataProvider::setPressureAltitude);
            disconnect(m_currentSource, &Traffic::TrafficDataSource_Abstract::factorWithoutPosition, this, &Traffic::TrafficDataProvider::onTrafficFactorWithoutPosition);
            disconnect(m_currentSource, &Traffic::TrafficDataSource_Abstract::factorWithPosition, this, &Traffic::TrafficDataProvider::onTrafficFactorWithPosition);
            disconnect(m_currentSource, &Traffic::TrafficDataSource_Abstract::positionUpdated, this, &Traffic::TrafficDataProvider::setPositionInfo);
            disconnect(m_currentSource, &Traffic::TrafficDataSource_Abstract::warning, this, &Traffic::TrafficDataProvider::setWarning);
        }

        // Update m_currentsource
        m_currentSource = heartbeatDataSource;

        if (!m_currentSource.isNull())
        {
            // If there is a new m_currentSource, then setup Qt connections and
            // disconnect all sources of lower priority from the traffic receivers.
            connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::pressureAltitudeUpdated, this, &Traffic::TrafficDataProvider::setPressureAltitude);
            connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::factorWithoutPosition, this, &Traffic::TrafficDataProvider::onTrafficFactorWithoutPosition);
            connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::factorWithPosition, this, &Traffic::TrafficDataProvider::onTrafficFactorWithPosition);
            connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::positionUpdated, this, &Traffic::TrafficDataProvider::setPositionInfo);
            connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::warning, this, &Traffic::TrafficDataProvider::setWarning);

            // Disconnect from traffic receiver
            bool doDisconnect = false;
            foreach(auto source, m_dataSources)
            {
                if ( source.isNull() )
                {
                    continue;
                }
                if (source == m_currentSource)
                {
                    doDisconnect = true;
                    continue;
                }
                if (doDisconnect)
                {
                    source->disconnectFromTrafficReceiver();
                }
            }

        }
        else
        {
            // If there is no m_currentSource, then try in 1sek to (re)connect to any
            // traffic receiver out there.
            QTimer::singleShot(1s, this, &Traffic::TrafficDataProvider::connectToTrafficReceiver);
        }


    }

    // Update heartbeat status
    if (m_currentSource.isNull())
    {
        setReceivingHeartbeat(false);
    }
    else
    {
        setReceivingHeartbeat(m_currentSource->receivingHeartbeat());
    }
}


void Traffic::TrafficDataProvider::onTrafficFactorWithoutPosition(const Traffic::TrafficFactor_DistanceOnly &factor)
{

    if (factor.ID() == m_trafficObjectWithoutPosition->ID())
    {
        m_trafficObjectWithoutPosition->setAnimate(true);
        m_trafficObjectWithoutPosition->copyFrom(factor);
        m_trafficObjectWithoutPosition->startLiveTime();
    }

    if (factor.hasHigherPriorityThan(*m_trafficObjectWithoutPosition))
    {
        m_trafficObjectWithoutPosition->setAnimate(false);
        m_trafficObjectWithoutPosition->copyFrom(factor);
        m_trafficObjectWithoutPosition->startLiveTime();
    }

}


void Traffic::TrafficDataProvider::onTrafficFactorWithPosition(const Traffic::TrafficFactor_WithPosition &factor)
{

    // Check if traffic is too far away to be shown
    bool farAway = false;
    if (factor.vDist().isFinite() && (factor.vDist() > maxVerticalDistance))
    {
        farAway = true;
    }
    if (factor.hDist().isFinite() && (factor.hDist() > maxHorizontalDistance))
    {
        farAway = true;
    }


    // Check if the traffic is one of the known factors.
    foreach(auto target, m_trafficObjects)
    {
        if (factor.ID() == target->ID())
        {
            // If traffic is too far away, delete the entry. Otherwise, replace the entry by the factor.
            if (farAway)
            {
                target->setAnimate(false);
                target->copyFrom(TrafficFactor_WithPosition());
            }
            else
            {
                target->setAnimate(true);
                target->copyFrom(factor);
                target->startLiveTime();
            }
            return;
        }
    }

    // If traffic is too far away, ignore the factor.
    if (farAway) {
        return;
    }

    auto *lowestPriObject = m_trafficObjects.at(0);
    foreach(auto target, m_trafficObjects)
    {
        if (lowestPriObject->hasHigherPriorityThan(*target))
        {
            lowestPriObject = target;
        }
    }
    if (factor.hasHigherPriorityThan(*lowestPriObject))
    {
        lowestPriObject->setAnimate(false);
        lowestPriObject->copyFrom(factor);
        lowestPriObject->startLiveTime();
    }

}


void Traffic::TrafficDataProvider::onTrafficReceiverRuntimeError()
{
    QString result;
    foreach(auto dataSource, m_dataSources)
    {
        if (dataSource.isNull())
        {
            continue;
        }
        result = dataSource->trafficReceiverRuntimeError();
        if (!result.isEmpty())
        {
            break;
        }
    }

    if (m_trafficReceiverRuntimeError == result)
    {
        return;
    }
    m_trafficReceiverRuntimeError = result;
    emit trafficReceiverRuntimeErrorChanged();
}


void Traffic::TrafficDataProvider::onTrafficReceiverSelfTestError()
{
    QString result;
    foreach(auto dataSource, m_dataSources)
    {
        if (dataSource.isNull())
        {
            continue;
        }
        result = dataSource->trafficReceiverSelfTestError();
        if (!result.isEmpty())
        {
            break;
        }
    }

    if (m_trafficReceiverSelfTestError == result)
    {
        return;
    }
    m_trafficReceiverSelfTestError = result;
    emit trafficReceiverSelfTestErrorChanged();
}


void Traffic::TrafficDataProvider::resetWarning()
{
    setWarning( Traffic::Warning() );
}


void Traffic::TrafficDataProvider::setPassword(const QString& SSID, const QString &password)
{
    foreach(auto dataSource, m_dataSources)
    {
        if (dataSource.isNull())
        {
            continue;
        }
        dataSource->setPassword(SSID, password);
    }

}


void Traffic::TrafficDataProvider::setReceivingHeartbeat(bool newReceivingHeartbeat)
{
    if (m_receivingHeartbeat == newReceivingHeartbeat)
    {
        return;
    }
    m_receivingHeartbeat = newReceivingHeartbeat;
    emit receivingHeartbeatChanged(m_receivingHeartbeat);
}


void Traffic::TrafficDataProvider::setWarning(const Traffic::Warning& warning)
{
    if (warning.alarmLevel() > -1)
    {
        m_WarningTimer.start();
    }

    if (m_Warning == warning)
    {
        return;
    }

    m_Warning = warning;
    emit warningChanged(m_Warning);
}


void Traffic::TrafficDataProvider::updateStatusString()
{
    if (receivingHeartbeat())
    {
        QString result;
        if (!m_currentSource.isNull())
        {
            result += QStringLiteral("<p>%1</p><ul style='margin-left:-25px;'>").arg(m_currentSource->sourceName());
        }
        result += QStringLiteral("<li>%1</li>").arg(tr("Receiving heartbeat."));
        if (positionInfo().isValid())
        {
            result += QStringLiteral("<li>%1</li>").arg(tr("Receiving position info."));
        }
        if (pressureAltitude().isFinite())
        {
            result += QStringLiteral("<li>%1</li>").arg(tr("Receiving barometric altitude info."));
        }
        result += u"</ul>"_qs;
        setStatusString(result);
        return;
    }

    QString result = "<p>" + tr("Not receiving heartbeat.") + "<p>";

    result += BTStatus();
    result += u"<ul style='margin-left:-25px;'>"_qs;
    foreach(auto source, m_dataSources)
    {
        if (source.isNull())
        {
            continue;
        }

        result += u"<li>"_qs;
        result += source->sourceName() + ": " + source->connectivityStatus();
        if (!source->errorString().isEmpty())
        {
            result += " " + source->errorString();
        }
        result += u"</li>"_qs;
    }
    result += u"</ul>"_qs;

    setStatusString(result);
}
