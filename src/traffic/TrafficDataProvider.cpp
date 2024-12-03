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
#include <QFile>

#include "platform/PlatformAdaptor_Abstract.h"
#include "traffic/TrafficDataProvider.h"

#if __has_include(<QSerialPort>)
#include "traffic/TrafficDataSource_SerialPort.h"
#endif
#include "traffic/TrafficDataSource_Tcp.h"
#include "traffic/TrafficDataSource_Udp.h"

using namespace Qt::Literals::StringLiterals;


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
    addDataSource( new Traffic::TrafficDataSource_Tcp(true, QStringLiteral("10.10.10.10"), 2000, this));
    addDataSource( new Traffic::TrafficDataSource_Tcp(true, QStringLiteral("192.168.1.1"), 2000, this));
    addDataSource( new Traffic::TrafficDataSource_Tcp(true, QStringLiteral("192.168.4.1"), 2000, this));
    addDataSource( new Traffic::TrafficDataSource_Tcp(true, QStringLiteral("192.168.10.1"), 2000, this) );
    addDataSource( new Traffic::TrafficDataSource_Tcp(true, QStringLiteral("192.168.42.1"), 2000, this) );
    addDataSource( new Traffic::TrafficDataSource_Udp(true, 4000, this) );
    addDataSource( new Traffic::TrafficDataSource_Udp(true, 49002, this));

    // Setup Bindings
    m_pressureAltitude.setBinding([this]() {return computePressureAltitude();});

    // Bindings for saving
    loadConnectionInfos();
    connect(this, &Traffic::TrafficDataProvider::dataSourcesChanged, this, &Traffic::TrafficDataProvider::saveConnectionInfos);

    // Bindings for status string
    connect(this, &Traffic::TrafficDataProvider::positionInfoChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
#warning
    //    connect(this, &Traffic::TrafficDataProvider::pressureAltitudeChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
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
}


//
// Methods
//

void Traffic::TrafficDataProvider::addDataSource(Traffic::TrafficDataSource_Abstract* source)
{
    Q_ASSERT( source != nullptr );

    source->setParent(this);
    QQmlEngine::setObjectOwnership(source, QQmlEngine::CppOwnership);

    auto tmp = m_dataSources.value();
    tmp  << source;
    m_dataSources = tmp;

    connect(source, &Traffic::TrafficDataSource_Abstract::connectivityStatusChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(source, &Traffic::TrafficDataSource_Abstract::errorStringChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(source, &Traffic::TrafficDataSource_Abstract::passwordRequest, this, &Traffic::TrafficDataProvider::passwordRequest);
    connect(source, &Traffic::TrafficDataSource_Abstract::passwordStorageRequest, this, &Traffic::TrafficDataProvider::passwordStorageRequest);
    connect(source, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataProvider::updateStatusString);
    connect(source, &Traffic::TrafficDataSource_Abstract::receivingHeartbeatChanged, this, &Traffic::TrafficDataProvider::onSourceHeartbeatChanged);
    connect(source, &Traffic::TrafficDataSource_Abstract::trafficReceiverRuntimeErrorChanged, this, &Traffic::TrafficDataProvider::onTrafficReceiverRuntimeError);
    connect(source, &Traffic::TrafficDataSource_Abstract::trafficReceiverSelfTestErrorChanged, this, &Traffic::TrafficDataProvider::onTrafficReceiverSelfTestError);

    emit dataSourcesChanged();
    updateStatusString();
}

QString Traffic::TrafficDataProvider::addDataSource(const Traffic::ConnectionInfo &connectionInfo)
{
    switch (connectionInfo.type()) {
    case Traffic::ConnectionInfo::Invalid:
        return tr("Invalid connection.");
    case Traffic::ConnectionInfo::BluetoothClassic:
    case Traffic::ConnectionInfo::BluetoothLowEnergy:
        return addDataSource_BluetoothClassic(connectionInfo);
    case Traffic::ConnectionInfo::TCP:
        return tr("Unable to add TCP connection. This is not implemented at the moment.");
    case Traffic::ConnectionInfo::UDP:
        return addDataSource_UDP(connectionInfo.port());
    case Traffic::ConnectionInfo::Serial:
        return addDataSource_SerialPort(connectionInfo.name());
    case Traffic::ConnectionInfo::FLARMFile:
        return tr("Unable to add FLARM simulator file connection. This is not implemented at the moment.");
    }
    return {};
}

QString Traffic::TrafficDataProvider::addDataSource_UDP(quint16 port)
{
    // Ignore new device if data source already exists.
    foreach(auto _dataSource, m_dataSources.value())
    {
        auto* dataSourceUDP = qobject_cast<TrafficDataSource_Udp*>(_dataSource);
        if (dataSourceUDP != nullptr)
        {
            if (port == dataSourceUDP->port())
            {
                return tr("A connection to this device already exists.");
            }
        }
    }

    auto* source = new TrafficDataSource_Udp(false, port, this);
    source->connectToTrafficReceiver();
    addDataSource(source);
    return {};
}

QString Traffic::TrafficDataProvider::addDataSource_SerialPort(const QString& portName)
{
#if __has_include(<QSerialPort>)
    // Ignore new device if data source already exists.
    foreach(auto _dataSource, m_dataSources.value())
    {
        auto* dataSourceSerialPort = qobject_cast<TrafficDataSource_SerialPort*>(_dataSource);
        if (dataSourceSerialPort != nullptr)
        {
            if (portName == dataSourceSerialPort->sourceName())
            {
                return tr("A connection to this device already exists.");
            }
        }
    }

    auto* source = new TrafficDataSource_SerialPort(false, portName, this);
    source->connectToTrafficReceiver();
    addDataSource(source);
    return {};
#else
    return tr("Serial ports are not supported on this platform.");
#endif
}

QString Traffic::TrafficDataProvider::addDataSource_TCP(const QString& host, quint16 port)
{
    // Ignore new device if data source already exists.
    foreach(auto _dataSource, m_dataSources.value())
    {
        auto* dataSourceTCP = qobject_cast<TrafficDataSource_Tcp*>(_dataSource);
        if (dataSourceTCP != nullptr)
        {
            if ((port == dataSourceTCP->port()) && (host == dataSourceTCP->host()))
            {
                return tr("A connection to this device already exists.");
            }
        }
    }

    auto* source = new TrafficDataSource_Tcp(false, host, port, this);
    source->connectToTrafficReceiver();
    addDataSource(source);
    return {};
}

void Traffic::TrafficDataProvider::clearDataSources()
{
    if (m_dataSources.value().isEmpty())
    {
        return;
    }
    foreach(auto dataSource, m_dataSources.value())
    {
        if (dataSource.isNull())
        {
            continue;
        }
        dataSource->disconnect();
        delete dataSource;
    }

    auto tmp = m_dataSources.value();
    tmp.clear();
    m_dataSources = tmp;
}

void Traffic::TrafficDataProvider::connectToTrafficReceiver()
{
    foreach(auto dataSource, m_dataSources.value())
    {
        if (dataSource.isNull())
        {
            continue;
        }
        dataSource->connectToTrafficReceiver();
    }

    updateStatusString();
}

QList<Traffic::TrafficDataSource_Abstract*> Traffic::TrafficDataProvider::dataSources() const
{
    QList<Traffic::TrafficDataSource_Abstract*> result;
    foreach(auto dataSource, m_dataSources.value())
    {
        if (dataSource == nullptr)
        {
            continue;
        }
        result.append(dataSource);
    }

    std::sort(result.begin(),
              result.end(),
              [](const Traffic::TrafficDataSource_Abstract* first, const Traffic::TrafficDataSource_Abstract* second)
              { return first->sourceName() < second->sourceName(); });

    return result;
}

void Traffic::TrafficDataProvider::deferredInitialization() const
{
    // Try to (re)connect whenever the network situation changes
    connect(GlobalObject::platformAdaptor(), &Platform::PlatformAdaptor_Abstract::wifiConnected, this, &Traffic::TrafficDataProvider::connectToTrafficReceiver);
}

void Traffic::TrafficDataProvider::disconnectFromTrafficReceiver()
{
    foreach(auto dataSource, m_dataSources.value())
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

void Traffic::TrafficDataProvider::loadConnectionInfos()
{
    QFile outFile(stdFileName);
    if (!outFile.open(QIODeviceBase::ReadOnly))
    {
        return;
    }
    QDataStream outStream(&outFile);
    QList<Traffic::ConnectionInfo> connectionInfos;
    outStream >> connectionInfos;
    foreach (auto connectionInfo, connectionInfos)
    {
        addDataSource(connectionInfo);
    }
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
    foreach(auto source, m_dataSources.value())
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
#warning
            //            disconnect(m_currentSource, &Traffic::TrafficDataSource_Abstract::pressureAltitudeUpdated, this, &Traffic::TrafficDataProvider::setPressureAltitude);
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
#warning
            // connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::pressureAltitudeUpdated, this, &Traffic::TrafficDataProvider::setPressureAltitude);
            connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::factorWithoutPosition, this, &Traffic::TrafficDataProvider::onTrafficFactorWithoutPosition);
            connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::factorWithPosition, this, &Traffic::TrafficDataProvider::onTrafficFactorWithPosition);
            connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::positionUpdated, this, &Traffic::TrafficDataProvider::setPositionInfo);
            connect(m_currentSource, &Traffic::TrafficDataSource_Abstract::warning, this, &Traffic::TrafficDataProvider::setWarning);

            // Disconnect from traffic receiver
            bool doDisconnect = false;
            foreach(auto source, m_dataSources.value())
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
    foreach(auto dataSource, m_dataSources.value())
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
    foreach(auto dataSource, m_dataSources.value())
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

void Traffic::TrafficDataProvider::removeDataSource(Traffic::TrafficDataSource_Abstract* source)
{
    if (source == nullptr)
    {
        return;
    }
    if (source->canonical()) {
        return;
    }

    auto tmp = m_dataSources.value();
    tmp.removeAll(source);
    m_dataSources = tmp;

    emit dataSourcesChanged();
    source->deleteLater();
}

void Traffic::TrafficDataProvider::resetWarning()
{
    setWarning( Traffic::Warning() );
}

void Traffic::TrafficDataProvider::saveConnectionInfos()
{
    QList<Traffic::ConnectionInfo> connectionInfos;
    foreach (auto dataSource, m_dataSources.value())
    {
        if (dataSource == nullptr)
        {
            continue;
        }
        if (dataSource->canonical())
        {
            continue;
        }
        auto connectionInfo = dataSource->connectionInfo();
        if (connectionInfo.type() == Traffic::ConnectionInfo::Invalid)
        {
            continue;
        }
        connectionInfos << connectionInfo;
    }

    QFile outFile(stdFileName);
    if (!outFile.open(QIODeviceBase::WriteOnly))
    {
        return;
    }
    QDataStream outStream(&outFile);
    outStream << connectionInfos;
    outFile.close();
}

void Traffic::TrafficDataProvider::setPassword(const QString& SSID, const QString &password)
{
    foreach(auto dataSource, m_dataSources.value())
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
        result += u"</ul>"_s;
        setStatusString(result);
        return;
    }

    const QString result = tr("Not receiving traffic receiver heartbeat through any of the configured data connections.");
    setStatusString(result);
}


//
// Private Methods
//

Units::Distance Traffic::TrafficDataProvider::computePressureAltitude()
{
    for(auto dataSource : m_dataSources.value())
    {
        if (dataSource.isNull())
        {
            continue;
        }
        auto pAlt = dataSource->pressureAltitude();
        if (pAlt.isFinite())
        {
            return pAlt;
        }
    }
    return {};
}
