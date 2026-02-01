/***************************************************************************
 *   Copyright (C) 2021-2025 by Stefan Kebekus                             *
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

#include "TrafficDataSource_Ogn.h"
#include "GlobalObject.h"
#include "geomaps/GeoMapProvider.h"
#include "navigation/Navigator.h"
#include "positioning/PositionProvider.h"
#include "OgnParser.h"
#include "traffic/ConnectionInfo.h"
#include "traffic/FlarmnetDB.h"
#include "traffic/TrafficDataSource_AbstractSocket.h"
#include "traffic/TrafficFactorAircraftType.h"
#include "traffic/TrafficFactor_WithPosition.h"
#include "traffic/TransponderDB.h"

#include <chrono>
#include <utility>
#include <QAbstractSocket>
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QGeoPositionInfo>
#include <QMap>
#include <QMetaEnum>
#include <QNetworkProxy>
#include <QObject>
#include <QProcessEnvironment>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QString>
#include <QStringConverter>
#include <QTcpSocket>
#include <QTextStream>
#include <QTimer>
#include <QUrl>
#include <QtGlobal>
#include <QtMath>

#define OGN_DEBUG 0

using namespace Qt::Literals::StringLiterals;

namespace {

// Helper function to convert OgnAircraftType to Traffic::AircraftType
Traffic::AircraftType convertOgnAircraftType(Ogn::OgnAircraftType ognType)
{
    using namespace Ogn;
    switch (ognType) {
        case OgnAircraftType::unknown:         return Traffic::AircraftType::unknown;
        case OgnAircraftType::Aircraft:        return Traffic::AircraftType::Aircraft;
        case OgnAircraftType::Airship:         return Traffic::AircraftType::Airship;
        case OgnAircraftType::Balloon:         return Traffic::AircraftType::Balloon;
        case OgnAircraftType::Copter:          return Traffic::AircraftType::Copter;
        case OgnAircraftType::Drone:           return Traffic::AircraftType::Drone;
        case OgnAircraftType::Glider:          return Traffic::AircraftType::Glider;
        case OgnAircraftType::HangGlider:      return Traffic::AircraftType::HangGlider;
        case OgnAircraftType::Jet:             return Traffic::AircraftType::Jet;
        case OgnAircraftType::Paraglider:      return Traffic::AircraftType::Paraglider;
        case OgnAircraftType::Skydiver:        return Traffic::AircraftType::Skydiver;
        case OgnAircraftType::StaticObstacle:  return Traffic::AircraftType::StaticObstacle;
        case OgnAircraftType::TowPlane:        return Traffic::AircraftType::TowPlane;
        default:                               return Traffic::AircraftType::unknown;
    }
}

// Helper function to convert Traffic::AircraftType to OgnAircraftType
Ogn::OgnAircraftType convertToOgnAircraftType(Traffic::AircraftType trafficType)
{
    using namespace Ogn;
    switch (trafficType) {
        case Traffic::AircraftType::unknown:        return OgnAircraftType::unknown;
        case Traffic::AircraftType::Aircraft:       return OgnAircraftType::Aircraft;
        case Traffic::AircraftType::Airship:        return OgnAircraftType::Airship;
        case Traffic::AircraftType::Balloon:        return OgnAircraftType::Balloon;
        case Traffic::AircraftType::Copter:         return OgnAircraftType::Copter;
        case Traffic::AircraftType::Drone:          return OgnAircraftType::Drone;
        case Traffic::AircraftType::Glider:         return OgnAircraftType::Glider;
        case Traffic::AircraftType::HangGlider:     return OgnAircraftType::HangGlider;
        case Traffic::AircraftType::Jet:            return OgnAircraftType::Jet;
        case Traffic::AircraftType::Paraglider:     return OgnAircraftType::Paraglider;
        case Traffic::AircraftType::Skydiver:       return OgnAircraftType::Skydiver;
        case Traffic::AircraftType::StaticObstacle: return OgnAircraftType::StaticObstacle;
        case Traffic::AircraftType::TowPlane:       return OgnAircraftType::TowPlane;
        default:                                    return OgnAircraftType::unknown;
    }
}

// Helper function to convert OgnAddressType to string
QString addressTypeToString(Ogn::OgnAddressType type)
{
    switch (type) {
        case Ogn::OgnAddressType::ICAO:        return u"ICAO"_s;
        case Ogn::OgnAddressType::FLARM:       return u"FLARM"_s;
        case Ogn::OgnAddressType::OGN_TRACKER: return u"OGN_TRACKER"_s;
        case Ogn::OgnAddressType::UNKNOWN:
        default:                               return u"UNKNOWN"_s;
    }
}

} // anonymous namespace

Traffic::TrafficDataSource_Ogn::TrafficDataSource_Ogn(bool isCanonical, QString hostName, quint16 port, QObject *parent) :
    Traffic::TrafficDataSource_AbstractSocket(isCanonical, parent),
    m_hostName(hostName.isEmpty() ? u"aprs.glidernet.org"_s : std::move(hostName)), // Default to aprs.glidernet.org
    m_port(port == 0 ? 14580 : port) // Default to port 14580
{
    // Connection Info
    m_connectionInfo = Traffic::ConnectionInfo(Traffic::ConnectionInfo::OgnInfo());

    // Generate a random number for the call sign.
    // This could be a problem if we have more than 10000 users at the same time. 
    m_callSign = QString(u"ENR%1"_s).arg(QRandomGenerator::global()->bounded(100000, 999999));

    m_textStream.setEncoding(QStringConverter::Latin1);

    // Once the socket connects, send a login string
    connect(&m_socket, &QTcpSocket::connected, this, [this]() {
        if (!m_receiveRadius.isFinite())
        {
            return;
        }

        // Send login string, e.g. "user ENR12345 pass 379 vers enroute 1.0.0 filter r/-48.0000/7.8512/99 t/o"
        updateCurrentCoordinate();

        #if OGN_DEBUG
        qDebug() << "OGN APRS-IS Login:"
                 << " callSign:" << m_callSign
                 << " filter center:" << m_currentPosition
                 << " radius (km):" << qRound(m_receiveRadius.toKM());
        #endif

        QString const loginString = QString::fromStdString(Ogn::OgnParser::formatLoginString(
            m_callSign.toStdString(),
            m_currentPosition.latitude(),
            m_currentPosition.longitude(),
            qRound(m_receiveRadius.toKM()),
            "enroute",
            QCoreApplication::applicationVersion().toStdString()
        ));
        m_textStream << loginString;
        m_textStream.flush();
    });

    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Traffic::TrafficDataSource_Ogn::onErrorOccurred);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Traffic::TrafficDataSource_Ogn::onReadyRead);
    connect(&m_socket, &QTcpSocket::stateChanged, this, &Traffic::TrafficDataSource_Ogn::onStateChanged);
    connect(&m_socket, &QAbstractSocket::disconnected, this, &Traffic::TrafficDataSource_Ogn::connectToTrafficReceiver, Qt::ConnectionType::QueuedConnection);

    // Initialize properties
    onStateChanged(m_socket.state());

    // Set up heartbeat timer
    auto* heartbeatTimer = new QTimer(this);
    connect(heartbeatTimer, &QTimer::timeout, this, &Traffic::TrafficDataSource_Ogn::verifyConnection);
    heartbeatTimer->start(1s); // 1 second interval

    // Set up periodic update timer
    auto* periodicUpdateTimer = new QTimer(this);
    connect(periodicUpdateTimer, &QTimer::timeout, this, &Traffic::TrafficDataSource_Ogn::periodicUpdate);
    periodicUpdateTimer->start(1min); // 1 minute interval

    // Whenever the approximate position changes, update the window for which traffic data is received.
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::approximateLastValidCoordinateChanged, this, [this]() {
        if (!m_receiveRadius.isFinite())
        {
            return;
        }

        updateCurrentCoordinate();
        setFilter(m_currentPosition);
    });

    // When map center changes, update the OGN server filter position
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::mapCenterChanged, this, [this]() {
        if (!m_receiveRadius.isFinite())
        {
            return;
        }

        updateCurrentCoordinate();
        setFilter(m_currentPosition);
    });

    // Update cached ownship filter data when aircraft settings change
    connect(GlobalObject::navigator(), &Navigation::Navigator::aircraftChanged, this, &Traffic::TrafficDataSource_Ogn::updateOwnshipFilterData);
    
    // Initialize cached ownship filter data
    updateOwnshipFilterData();
}

Traffic::TrafficDataSource_Ogn::~TrafficDataSource_Ogn()
{
    Traffic::TrafficDataSource_Ogn::disconnectFromTrafficReceiver();
    setReceivingHeartbeat(false);
}

void Traffic::TrafficDataSource_Ogn::connectToTrafficReceiver()
{
    // set Proxy
#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    const QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    const QString proxyUrl = env.value(u"HTTP_PROXY"_s);
    if (!proxyUrl.isEmpty()) {
        const QUrl url(proxyUrl);
        QNetworkProxy proxy;
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(url.host());
        proxy.setPort(url.port(8080)); // Default to port 8080 if not specified
        proxy.setUser(url.userName());
        proxy.setPassword(url.password());
        // Apply the proxy to the socket
        m_socket.setProxy(proxy);
    }
#endif
    
    // Start new connection
    m_socket.abort();
    setErrorString();
    m_socket.setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_socket.setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    m_textStream.setDevice(&m_socket);
    m_socket.connectToHost(m_hostName, m_port);

    // Update properties
    onStateChanged(m_socket.state());
}

void Traffic::TrafficDataSource_Ogn::disconnectFromTrafficReceiver()
{
    #if OGN_DEBUG
    qDebug() << "Disconnecting from OGN APRS-IS server";
    #endif

    // Disconnect socket
    m_socket.abort();

    // Update properties
    onStateChanged(m_socket.state());

    #if OGN_DEBUG
    qDebug() << "Disconnected";
    #endif
}

void Traffic::TrafficDataSource_Ogn::onReadyRead()
{
    // In this function 
    // avoid heap allocations for performance reasons.
    while (m_textStream.readLineInto(&m_lineBuffer))
    {
        emit dataReceived(m_lineBuffer);
        processOgnMessage(m_lineBuffer);
    }
}

void Traffic::TrafficDataSource_Ogn::updateOwnshipFilterData()
{
    // Cache aircraft name as std::u16string to match QString's 16-bit characters, convert to uppercase
    QString const name = GlobalObject::navigator()->aircraft().name().toUpper();
    m_ownAircraftName = std::u16string(reinterpret_cast<const char16_t*>(name.utf16()), name.size());
    
    // Cache and split transponder codes, convert to uppercase std::string for efficient comparison
    QString const transponderCode = GlobalObject::navigator()->aircraft().transponderCode().toUpper();
    m_ownTransponderCodes.clear();
    if (!transponderCode.isEmpty())
    {
        QStringList const codes = transponderCode.toUpper().split(QRegularExpression(u"\\s+"_s), Qt::SkipEmptyParts);
        m_ownTransponderCodes.reserve(codes.size());
        for (const QString& code : codes)
        {
            m_ownTransponderCodes.push_back(code.toStdString());
        }
    }
    
    #if OGN_DEBUG
    qDebug() << "Updated ownship filter data - Name:" << name
             << "Transponder codes count:" << m_ownTransponderCodes.size();
    #endif
}

void Traffic::TrafficDataSource_Ogn::processOgnMessage(const QString& data)
{
    // In this function 
    // avoid heap allocations for performance reasons.
    
    // Initialize the database
    static TransponderDB const transponderDB; 

    // notify that we are receiving data
    setReceivingHeartbeat(true);

    // Process APRS-IS sentence
    m_ognMessage.reset();
    m_ognMessage.sentence = data.toStdString();
    Ogn::OgnParser::parseAprsisMessage(m_ognMessage);

    if (m_ognMessage.type != Ogn::OgnMessageType::TRAFFIC_REPORT)
    {
        return;
    }
    if ((m_ognMessage.aircraftType == Ogn::OgnAircraftType::unknown || m_ognMessage.aircraftType == Ogn::OgnAircraftType::StaticObstacle) && m_ognMessage.speed == 0.0)
    {
        return;
    }
    // Check if coordinate is valid
    if (std::isnan(m_ognMessage.latitude) || std::isnan(m_ognMessage.longitude))
    {
        return;
    }

    #if OGN_DEBUG
        if (m_ognMessage.type == Ogn::OgnMessageType::TRAFFIC_REPORT) {
            qDebug() << "Traffic type:" << static_cast<int>(m_ognMessage.aircraftType) 
                    << " speed:" << m_ognMessage.speed 
                    << " coord valid:" << (!std::isnan(m_ognMessage.latitude) && !std::isnan(m_ognMessage.longitude))
                    << " alt:" << m_ognMessage.altitude
                    << " callsign:" << QString::fromUtf8(m_ognMessage.flightnumber.data(), static_cast<qsizetype>(m_ognMessage.flightnumber.size()))
                    << " addr:" << QString::fromUtf8(m_ognMessage.address.data(), static_cast<qsizetype>(m_ognMessage.address.size()));

        }
    #endif

    // Filter out ownship by ICAO 24-bit address (supports multiple codes separated by spaces)
    // Cached codes are uppercase, case-sensitive comparison
    for (const std::string& code : m_ownTransponderCodes)
    {
        if (code.size() == m_ognMessage.address.size() &&
            std::equal(m_ognMessage.address.begin(), m_ognMessage.address.end(), code.begin()))
        {
            #if OGN_DEBUG
            qDebug() << "Filtering out ownship with transponder code:" << QString::fromStdString(code);
            #endif
            return;
        }
    }    

    // Decode callsign
    QString callsign;
    if (m_ognMessage.addressType == Ogn::OgnAddressType::FLARM)
    {
        callsign = GlobalObject::flarmnetDB()->getRegistration(QString::fromUtf8(m_ognMessage.address.data(), static_cast<qsizetype>(m_ognMessage.address.size())));
    }
    else if (static_cast<int>(!m_ognMessage.flightnumber.empty()) != 0)
    {
        callsign = QString::fromUtf8(m_ognMessage.flightnumber.data(), static_cast<qsizetype>(m_ognMessage.flightnumber.size()));
    }
    else if (m_ognMessage.addressType == Ogn::OgnAddressType::ICAO)
    {
        callsign = transponderDB.getRegistration(QString::fromUtf8(m_ognMessage.address.data(), static_cast<qsizetype>(m_ognMessage.address.size())));
    }
    #if OGN_SHOW_ADDRESSTYPE
        callsign += QString(" (%1)").arg(addressTypeToString(m_ognMessage.addressType));
    #endif

    // Filter out ownship by aircraft name (callsign)
    // Direct comparison between QString and std::u16string using std::equal
    if (!m_ownAircraftName.empty() && 
        static_cast<size_t>(callsign.size()) == m_ownAircraftName.size() &&
        std::equal(reinterpret_cast<const char16_t*>(callsign.utf16()), 
                   reinterpret_cast<const char16_t*>(callsign.utf16()) + callsign.size(),
                   m_ownAircraftName.begin()))
    {
        #if OGN_DEBUG
        qDebug() << "Filtering out ownship with callsign:" << callsign;
        #endif
        return;
    }

    // Compute horizontal/vertical distance and the alarm Level
    int alarmLevel = 0;
    Units::Distance hDist;
    Units::Distance vDist;
      
    if (m_currentPosition.isValid())
    {
        const QGeoCoordinate ognCoordinate(m_ognMessage.latitude, m_ognMessage.longitude, m_ognMessage.altitude);
        hDist = Units::Distance::fromM(m_currentPosition.distanceTo(ognCoordinate));
        vDist = Units::Distance::fromM(qFabs(m_ognMessage.altitude - m_currentPosition.altitude()));
        
        // Only set alarm level if we're using actual GPS position, not map center
        if (m_usingGps)
        {
            if (hDist.toM() < 1000 && vDist.toFeet() < 400)
            {
                alarmLevel = 3; // High alert
            }
            else if (hDist.toM() < 2000 && vDist.toFeet() < 600)
            {
                alarmLevel = 2; // Medium alert
            }
            else if (hDist.toM() < 5000 && vDist.toFeet() < 800)
            {
                alarmLevel = 1; // Low alert
            }
        }
    }

    // PositionInfo
    QGeoPositionInfo pInfo(QGeoCoordinate(m_ognMessage.latitude, m_ognMessage.longitude, m_ognMessage.altitude), QDateTime::currentDateTimeUtc());
    pInfo.setAttribute(QGeoPositionInfo::Direction, m_ognMessage.course);  // Already in degrees
    pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, m_ognMessage.speed * 0.514444);  // Convert knots to m/s
    pInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, m_ognMessage.verticalSpeed);
    if (!pInfo.isValid())
    {
        return;
    }

    // Prepare the factor object
    Traffic::TrafficFactor_WithPosition factor;
    factor.setAlarmLevel(alarmLevel);
    factor.setCallSign(callsign);
    factor.setID(QString::fromUtf8(m_ognMessage.sourceId.data(), static_cast<qsizetype>(m_ognMessage.sourceId.size())));
    factor.setType(convertOgnAircraftType(m_ognMessage.aircraftType));
    factor.setPositionInfo(Positioning::PositionInfo(pInfo, sourceName()));
    factor.setHDist(hDist);
    factor.setVDist(vDist);
    factor.startLiveTime();

    #if OGN_DEBUG
    qDebug() << "Emitting traffic factor - ID:" << QString::fromUtf8(m_ognMessage.sourceId.data(), static_cast<qsizetype>(m_ognMessage.sourceId.size()))
             << " alarmLevel:" << alarmLevel
             << " type:" << static_cast<int>(m_ognMessage.aircraftType)
             << " callsign:" << callsign
             << " speed:" << m_ognMessage.speed << "kts"
             << " course:" << m_ognMessage.course << "°"
             << " altitude:" << m_ognMessage.altitude << "m"
             << " hDist:" << hDist.toM() << "m"
             << " vDist:" << vDist.toFeet() << "ft"
             << " valid:" << factor.positionInfo().isValid();
    #endif

    // Emit the factorWithPosition signal
    emit factorWithPosition(factor);
}

void Traffic::TrafficDataSource_Ogn::sendPosition(const QGeoCoordinate& coordinate, double course, double speed, double altitude)
{
    if (!m_socket.isOpen())
    {
#if OGN_DEBUG
        qDebug() << "Socket is not open. Cannot send position.";
#endif
        return;
    }

    // Use the OgnParser class to format the position report
    QString const positionReport = QString::fromStdString(Ogn::OgnParser::formatPositionReport(
        m_callSign.toStdString(), coordinate.latitude(), coordinate.longitude(), altitude, course, speed, convertToOgnAircraftType(m_aircraftType)));

    // Send the position report
    m_textStream << positionReport;
    m_textStream.flush();

#if OGN_DEBUG
    qDebug() << "Sent position report:" << positionReport;
#endif
}

// called once per minute
void Traffic::TrafficDataSource_Ogn::periodicUpdate()
{
    sendKeepAlive();
    //verifyConnection();

// update position report
#if OGN_SEND_OWN_POSITION
    if (getOwnShipCoordinate(/*useLastValidPosition*/false).coordinate().isValid())
    {
        sendPosition(positionInfo.coordinate(),
                     positionInfo.trueTrack().toDEG(),
                     positionInfo.groundSpeed().toKN(),
                     positionInfo.coordinate().altitude());
    }
    else
    {
#if OGN_DEBUG
        qDebug() << "Position is invalid, skipping position report.";
#endif
    }
#endif
}

void Traffic::TrafficDataSource_Ogn::sendKeepAlive()
{
    if (!m_socket.isOpen()) {
#if OGN_DEBUG
        qDebug() << "Cannot send keep-alive: socket not open";
#endif
        return;
    }
    // Send a keep-alive comment (APRS-IS protocol)
    m_textStream << "# keep-alive\n";
    m_textStream.flush();
#if OGN_DEBUG
    qDebug() << "Sent keep-alive to APRS-IS server";
#endif
}

void Traffic::TrafficDataSource_Ogn::verifyConnection()
{
    if (!m_socket.isOpen() || m_socket.state() != QAbstractSocket::ConnectedState)
    {
#if OGN_DEBUG
        qWarning() << "Connection to OGN APRS-IS server lost. State:" << m_socket.state() << "Reconnecting...";
#else
        qWarning() << "Connection to OGN APRS-IS server lost. Reconnecting...";
#endif
        disconnectFromTrafficReceiver();
        connectToTrafficReceiver();
    }
    else {
        setReceivingHeartbeat(true);
    }
}

void Traffic::TrafficDataSource_Ogn::updateCurrentCoordinate()
{
#define OGN_USE_MAPCENTER 1
#if OGN_USE_MAPCENTER    
    // Check if we have a recent GPS position (less than 3 minutes old)
    auto posInfo = GlobalObject::positionProvider()->positionInfo();
    const QGeoCoordinate gpsCoordinate = posInfo.coordinate();
    
    if (gpsCoordinate.isValid())
    {
        qint64 const positionAgeMs = posInfo.timestamp().msecsTo(QDateTime::currentDateTimeUtc());
        if (positionAgeMs < 180000) // 3 minutes = 180000 ms
        {
            // Use GPS position if valid and recent
            m_currentPosition = gpsCoordinate;
            m_usingGps = true;
            return;
        }
    }

    // GPS is too old or invalid, use map center
    auto mapCenter = GlobalObject::positionProvider()->mapCenter();  
    auto groundElevation = GlobalObject::geoMapProvider()->terrainElevationAMSL(mapCenter);
    if (groundElevation.isFinite())
    {
        mapCenter.setAltitude(groundElevation.toM());
    }
    m_currentPosition = mapCenter;
    m_usingGps = false;
#else
    // Always use GPS position
    m_currentPosition = GlobalObject::positionProvider()->approximateLastValidCoordinate();
    m_usingGps = true;
#endif
}

void Traffic::TrafficDataSource_Ogn::setFilter(const QGeoCoordinate& coordinate)
{
    if (!coordinate.isValid())
    {
        return;
    }
    
    // Throttle filter updates: max one per second AND only if moved more than 5km
    qint64 const currentTime = QDateTime::currentMSecsSinceEpoch();
    if (m_lastFilterPosition.isValid())
    {
        qint64 const timeSinceLastUpdate = currentTime - m_lastFilterUpdateTime;
        if (timeSinceLastUpdate < MIN_FILTER_UPDATE_INTERVAL_MS)
        {
            return; // Too soon since last update
        }

        double const distanceKm = m_lastFilterPosition.distanceTo(coordinate) / 1000.0;
        if (distanceKm < MIN_FILTER_UPDATE_DISTANCE_KM)
        {
            return; // Haven't moved far enough
        }
    }

    // Update filter
    QString const filterCmd = QString::fromStdString(Ogn::OgnParser::formatFilterCommand(
        coordinate.latitude(), coordinate.longitude(), qRound(m_receiveRadius.toKM())));
    m_textStream << filterCmd;
    m_textStream.flush();

    // Update throttling state - only after successfully sending
    m_lastFilterPosition = coordinate;
    m_lastFilterUpdateTime = currentTime;

    #if OGN_DEBUG
        QString const source = m_usingGps ? "GPS" : "map center";
        qDebug() << "Updated OGN filter to" << source << ":" << coordinate << "radius:" << qRound(m_receiveRadius.toKM()) << "km";
    #endif
}
