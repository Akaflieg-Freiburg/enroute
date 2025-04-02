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
#include <QRandomGenerator>
#include <QMap>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QUrl>
#include <QNetworkProxy>
#include <QtMath>
#include "GlobalObject.h"
#include "positioning/PositionProvider.h"
#include "TrafficDataSource_OgnParser.h"
#include "TrafficFactor_WithPosition.h"
#include "TrafficFactorAircraftType.h"
#include "positioning/PositionInfo.h"
#include "traffic/FlarmnetDB.h"
#include "TransponderDB.h"
#include <QTimer>
#include <QMetaEnum>

using namespace Qt::Literals::StringLiterals;

Traffic::TrafficDataSource_Ogn::TrafficDataSource_Ogn(bool isCanonical, QString hostName, quint16 port, QObject *parent) :
    Traffic::TrafficDataSource_AbstractSocket(isCanonical, parent),
    m_hostName(hostName.isEmpty() ? "aprs.glidernet.org" : std::move(hostName)), // Default to aprs.glidernet.org
    m_port(port == 0 ? 14580 : port) // Default to port 14580
{
    // Generate a random 5-digit number for the call sign
    m_callSign = QString("ENR%1").arg(QRandomGenerator::global()->bounded(10000, 99999));

    // Connect socket
    connect(&m_socket, &QTcpSocket::connected, this, &Traffic::TrafficDataSource_Ogn::onConnected);
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Traffic::TrafficDataSource_Ogn::onErrorOccurred);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Traffic::TrafficDataSource_Ogn::onReadyRead);
    connect(&m_socket, &QTcpSocket::stateChanged, this, &Traffic::TrafficDataSource_Ogn::onStateChanged);
    connect(&m_socket, &QAbstractSocket::disconnected, [](){ qDebug() << "Disconnected from OGN APRS-IS server"; });
    connect(&m_socket, &QAbstractSocket::disconnected, this, &Traffic::TrafficDataSource_Ogn::connectToTrafficReceiver, Qt::ConnectionType::QueuedConnection);

    // Set up text stream
    m_textStream.setDevice(&m_socket);
    m_textStream.setEncoding(QStringConverter::Latin1);

    // Initialize properties
    onStateChanged(m_socket.state());

    // Set up periodic update timer
    auto* periodicUpdateTimer = new QTimer(this);
    connect(periodicUpdateTimer, &QTimer::timeout, this, &Traffic::TrafficDataSource_Ogn::periodicUpdate);
    periodicUpdateTimer->start(60 * 1000); // 1 minute interval
}

Traffic::TrafficDataSource_Ogn::~TrafficDataSource_Ogn()
{

    Traffic::TrafficDataSource_Ogn::disconnectFromTrafficReceiver();
    setReceivingHeartbeat(false); // This will release the WiFi lock if necessary
}

void Traffic::TrafficDataSource_Ogn::onConnected()
{
    qDebug() << "Connected to OGN APRS-IS server";

    // Send login string
    sendLoginString();

    #if OGN_SEND_OWN_POSITION
    // Send an initial position report
    if (getOwnShipCoordinate(/*useLastValidPosition*/false).coordinate().isValid()) {
        sendPosition(positionInfo.coordinate(),
                     positionInfo.trueTrack().toDEG(),
                     positionInfo.groundSpeed().toKN(),
                     positionInfo.coordinate().altitude());
    } else {
        qDebug() << "Position is invalid, skipping initial position report.";
    }
    #endif
}

void Traffic::TrafficDataSource_Ogn::connectToTrafficReceiver()
{
    // Do not do anything if the traffic receiver is connected and is receiving.
    if (receivingHeartbeat())
    {
        return;
    }

    // set Proxy
    #if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QString proxyUrl = env.value("HTTP_PROXY");
    if (!proxyUrl.isEmpty()) {
        QUrl url(proxyUrl);
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
    m_socket.connectToHost(m_hostName, m_port);
    m_textStream.setDevice(&m_socket);

    // Update properties
    onStateChanged(m_socket.state());
}

void Traffic::TrafficDataSource_Ogn::disconnectFromTrafficReceiver()
{
    // Disconnect socket
    m_socket.abort();

    // Update properties
    onStateChanged(m_socket.state());
}

void Traffic::TrafficDataSource_Ogn::sendLoginString()
{
    // update own location
    QGeoCoordinate coordinate = getOwnShipCoordinate(/*useLastValidPosition*/true);
    if (coordinate.isValid()) {
        m_receiveLocation = coordinate;
    }

    QString loginString = Ogn::TrafficDataSource_OgnParser::formatLoginString(
        m_callSign, m_receiveLocation, m_receiveRadius, u"enroute", QCoreApplication::applicationVersion());

    m_textStream << loginString;
    m_textStream.flush();
    
    #if OGN_DEBUG
    qDebug() << "Sent login string:" << loginString;
    #endif
}

QGeoCoordinate Traffic::TrafficDataSource_Ogn::getOwnShipCoordinate(bool useLastValidPosition) const
{
    QGeoCoordinate ownShipCoordinate;
    auto* positionProviderPtr = GlobalObject::positionProvider();
    if (positionProviderPtr) {
        ownShipCoordinate = positionProviderPtr->positionInfo().coordinate();
        if (!ownShipCoordinate.isValid() && useLastValidPosition) {
            ownShipCoordinate = positionProviderPtr->lastValidCoordinate();
        }
    }
    return ownShipCoordinate;
}

void Traffic::TrafficDataSource_Ogn::onReadyRead()
{
    static TransponderDB transponderDB; // Initialize the database

    QString sentence;
    while (m_textStream.readLineInto(&sentence)) {
        // notify that we are receiving data
        setReceivingHeartbeat(true);

        // Process APRS-IS sentence
        auto ognMessage = Ogn::TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

        switch(ognMessage.type)
        {
            case Traffic::Ogn::OgnMessageType::TRAFFIC_REPORT:
            {
                if ((ognMessage.aircraftType == Traffic::AircraftType::unknown || 
                     ognMessage.aircraftType == Traffic::AircraftType::StaticObstacle)
                   && ognMessage.speed == 0) {
                    #if OGN_DEBUG
                    qDebug() << "Not an Aircraft.";
                    #endif
                    return;
                }
                if (!ognMessage.coordinate.isValid()) {
                    #if OGN_DEBUG
                    qDebug() << "Invalid coordinate for traffic report:" << sentence;
                    #endif
                    return;
                }

                // Compute horizontal and vertical distance to traffic if our own position is known.
                QGeoCoordinate ownShipCoordinate = getOwnShipCoordinate(/*useLastValidPosition*/true);
                Units::Distance hDist {};
                Units::Distance vDist {};
                if (ownShipCoordinate.isValid()) {
                    hDist = Units::Distance::fromM( ownShipCoordinate.distanceTo(ognMessage.coordinate) / 10.0 );  // TODO REMOVE /10
                    vDist = Units::Distance::fromM( qFabs( ognMessage.coordinate.altitude() - ownShipCoordinate.altitude() ) );
                }

                // Decode callsign
                QString callsign;
                if (ognMessage.addressType == Traffic::Ogn::OgnAddressType::FLARM) {
                    callsign = GlobalObject::flarmnetDB()->getRegistration(QString(ognMessage.address));
                } else if (ognMessage.flightnumber.length()) {
                    callsign = QString(ognMessage.flightnumber);
                } else if (ognMessage.addressType == Traffic::Ogn::OgnAddressType::ICAO) {
                    callsign = transponderDB.getRegistration(QString(ognMessage.address));
                }
                #if OGN_SHOW_ADDRESSTYPE
                const QMetaEnum metaEnum = QMetaEnum::fromType<Traffic::Ogn::OgnAddressType>();
                callsign += QString(" (%1)").arg(metaEnum.valueToKey(static_cast<int>(ognMessage.addressType)));
                #endif

                // Compute Alarm Level 0-3
                int alarmLevel = 0;
                if(hDist.toM() < 1000 && vDist.toFeet() < 400) {
                    alarmLevel = 3; // High alert
                } else if(hDist.toM() < 2000 && vDist.toFeet() < 600) {
                    alarmLevel = 2; // Medium alert
                } else if(hDist.toM() < 5000 && vDist.toFeet() < 800) {
                    alarmLevel = 1; // Low alert
                }
                
                // PositionInfo
                QGeoPositionInfo pInfo(ognMessage.coordinate, QDateTime::currentDateTimeUtc());
                pInfo.setAttribute(QGeoPositionInfo::Direction, ognMessage.course.toDouble());
                pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, ognMessage.speed.toDouble());
                pInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, ognMessage.verticalSpeed);
                if (!pInfo.isValid()) {
                    qDebug() << "Invalid position-info for traffic report:" << sentence;
                    return;
                }

                // Prepare the m_factor object
                m_factor.setAlarmLevel(alarmLevel);
                m_factor.setCallSign(callsign);
                m_factor.setID(ognMessage.sourceId.toString());
                m_factor.setType(ognMessage.aircraftType);
                m_factor.setPositionInfo(Positioning::PositionInfo(pInfo, sourceName()));
                m_factor.setHDist(hDist);
                m_factor.setVDist(vDist);
                m_factor.startLiveTime();
            
                // Emit the factorWithPosition signal
                emit factorWithPosition(m_factor);
                break;
            }
            default:
                break;
        }
    }
}

void Traffic::TrafficDataSource_Ogn::sendPosition(const QGeoCoordinate& coordinate, double course, double speed, double altitude)
{
    if (!m_socket.isOpen()) {
        qWarning() << "Socket is not open. Cannot send position.";
        return;
    }

    // Use the OgnParser class to format the position report
    QString positionReport = Traffic::Ogn::TrafficDataSource_OgnParser::formatPositionReport(m_callSign, coordinate, course, speed, altitude, m_aircraftType);

    // Send the position report
    m_textStream << positionReport;
    m_textStream.flush();

    qDebug() << "Sent position report:" << positionReport;
}

// called once per minute
void Traffic::TrafficDataSource_Ogn::periodicUpdate()
{
    sendKeepAlive();
    verifyConnection();

    // update receive position
    QGeoCoordinate position = getOwnShipCoordinate(/*useLastValidPosition*/true);
    if (position.isValid()) {
        updateReceivePosition(position);
    }
}

void Traffic::TrafficDataSource_Ogn::updateReceivePosition(QGeoCoordinate position)
{
    double distance = position.distanceTo(m_receiveLocation);
    if (distance > 10000) { // More than 10 km
        qDebug() << "Current position is more than 10 km away from receiver position. Updating receiver position.";
        m_receiveLocation = position;
        // I found no other way to change the filter than to reconnect.
        // We disconnect and it will automatically reconnect. 
        disconnectFromTrafficReceiver();
        connectToTrafficReceiver();
    }
}

void Traffic::TrafficDataSource_Ogn::sendKeepAlive()
{
    // Send a keep-alive message (newline character as per APRS-IS protocol)
    m_textStream << "# " << QCoreApplication::organizationName() << QCoreApplication::applicationName() << QCoreApplication::applicationVersion() << "\n";
    m_textStream.flush();
    #if OGN_DEBUG
    qDebug() << "Sent keep-alive message to APRS-IS server.";
    #endif
}

void Traffic::TrafficDataSource_Ogn::verifyConnection()
{
    if (!m_socket.isOpen() || m_socket.state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Connection to OGN APRS-IS server lost. Reconnecting...";
        disconnectFromTrafficReceiver();
        connectToTrafficReceiver();
    }
}
