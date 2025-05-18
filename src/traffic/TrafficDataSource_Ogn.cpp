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
    // Generate a random number for the call sign.
    // This could be a problem if we have more than 10000 users at the same time. 
    m_callSign = QString("ENR%1").arg(QRandomGenerator::global()->bounded(100000, 999999));

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
    QGeoCoordinate const coordinate = getOwnShipCoordinate(/*useLastValidPosition*/ true);
    if (coordinate.isValid()) {
        m_receiveLocation = coordinate;
    }

    QString const loginString
        = Ogn::TrafficDataSource_OgnParser::formatLoginString(m_callSign,
                                                              m_receiveLocation,
                                                              m_receiveRadius,
                                                              u"enroute",
                                                              QCoreApplication::applicationVersion());

    m_textStream << loginString;
    m_textStream.flush();

#if OGN_DEBUG
    qDebug() << "Sent login string:" << loginString;
#endif
}

QGeoCoordinate Traffic::TrafficDataSource_Ogn::getOwnShipCoordinate(bool useLastValidPosition)
{
    QGeoCoordinate ownShipCoordinate;
    auto* positionProviderPtr = GlobalObject::positionProvider();
    if (positionProviderPtr != nullptr) {
        ownShipCoordinate = positionProviderPtr->positionInfo().coordinate();
        if (!ownShipCoordinate.isValid() && useLastValidPosition) {
            ownShipCoordinate = Positioning::PositionProvider::lastValidCoordinate();
        }
    }
    return ownShipCoordinate;
}

void Traffic::TrafficDataSource_Ogn::onReadyRead()
{
    QString sentence;
    while (m_textStream.readLineInto(&sentence))
    {
        processAPRS(sentence);
    }
}

void Traffic::TrafficDataSource_Ogn::sendPosition(const QGeoCoordinate& coordinate, double course, double speed, double altitude)
{
    if (!m_socket.isOpen()) {
#if OGN_DEBUG
        qDebug() << "Socket is not open. Cannot send position.";
#endif
        return;
    }

    // Use the OgnParser class to format the position report
    QString const positionReport = Traffic::Ogn::TrafficDataSource_OgnParser::formatPositionReport(
        m_callSign, coordinate, course, speed, altitude, m_aircraftType);

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
    verifyConnection();

    // update receive position
    QGeoCoordinate const position = getOwnShipCoordinate(/*useLastValidPosition*/ true);
    if (position.isValid()) {
        updateReceivePosition(position);
    }

// update position report
#if OGN_SEND_OWN_POSITION
    if (getOwnShipCoordinate(/*useLastValidPosition*/false).coordinate().isValid()) {
        sendPosition(positionInfo.coordinate(),
                     positionInfo.trueTrack().toDEG(),
                     positionInfo.groundSpeed().toKN(),
                     positionInfo.coordinate().altitude());
    } else {
#if OGN_DEBUG
        qDebug() << "Position is invalid, skipping position report.";
#endif
    }
#endif
}

void Traffic::TrafficDataSource_Ogn::updateReceivePosition(const QGeoCoordinate &position)
{
    double const distance = position.distanceTo(m_receiveLocation);
    if (distance > 10000) { // More than 10 km
#if OGN_DEBUG
        qDebug() << "Current position is more than 10 km away from OGN receive position. Updating receive position.";
#endif
        m_receiveLocation = position;
        QString const filterCommand
            = Traffic::Ogn::TrafficDataSource_OgnParser::formatFilterCommand(m_receiveLocation,
                                                                             m_receiveRadius);
#if OGN_DEBUG
        qDebug() << filterCommand;
#endif
        m_textStream << filterCommand;
        m_textStream.flush();
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
