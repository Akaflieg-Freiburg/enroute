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
#include "positioning/PositionProvider.h"
#include "TrafficDataSource_OgnParser.h"

#include <QRandomGenerator>
#include <QMap>
#include <QRegularExpression>
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QUrl>
#include <QNetworkProxy>
#include <QtMath>
#include <QTimer>
#include <QMetaEnum>

using namespace Qt::Literals::StringLiterals;

Traffic::TrafficDataSource_Ogn::TrafficDataSource_Ogn(bool isCanonical, QString hostName, quint16 port, QObject *parent) :
    Traffic::TrafficDataSource_AbstractSocket(isCanonical, parent),
    m_hostName(hostName.isEmpty() ? u"aprs.glidernet.org"_s : std::move(hostName)), // Default to aprs.glidernet.org
    m_port(port == 0 ? 14580 : port) // Default to port 14580
{
    // Generate a random number for the call sign.
    // This could be a problem if we have more than 10000 users at the same time. 
    m_callSign = QString(u"ENR%1"_s).arg(QRandomGenerator::global()->bounded(100000, 999999));

    // Once the socket connects, send a login string
    connect(&m_socket, &QTcpSocket::connected, this, [this]() {
        if (!m_receiveRadius.isFinite())
        {
            return;
        }

        // Send login string, e.g. "user ENR12345 pass 1234 vers 1.0.0 1.0 filter r/-48.0000/7.8512/99 t/o"
        auto approximatelastValidCoordinate = Positioning::PositionProvider::lastValidCoordinate();
        // Calculate the password based on the call sign
        // APRS-IS passcode calculation: Sum of ASCII values of the first 6 characters of the call sign
        // e.g. "1234"
        int sum = 0;
        for (int i = 0; i < m_callSign.length() && i < 6; ++i)
        {
            sum += m_callSign.at(i).unicode();
        }

        m_textStream << QString("user %1 pass %2 vers %3 %4 filter r/%5/%6/%7 t/o\n")
                            .arg(m_callSign)
                            .arg(QString::number(sum % 10000))
                            .arg("enroute")
                            .arg(QCoreApplication::applicationVersion())
                            .arg(approximatelastValidCoordinate.latitude(), 1, 'f', 4)
                            .arg(approximatelastValidCoordinate.longitude(), 1, 'f', 4)
                            .arg(qRound(m_receiveRadius.toKM()));
        m_textStream.flush();
    });

    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Traffic::TrafficDataSource_Ogn::onErrorOccurred);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Traffic::TrafficDataSource_Ogn::onReadyRead);
    connect(&m_socket, &QTcpSocket::stateChanged, this, &Traffic::TrafficDataSource_Ogn::onStateChanged);
    connect(&m_socket, &QAbstractSocket::disconnected, this, &Traffic::TrafficDataSource_Ogn::connectToTrafficReceiver, Qt::ConnectionType::QueuedConnection);

    // Set up text stream
    m_textStream.setDevice(&m_socket);
    m_textStream.setEncoding(QStringConverter::Latin1);

    // Initialize properties
    onStateChanged(m_socket.state());

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

        auto approximatelastValidCoordinate = GlobalObject::positionProvider()->approximateLastValidCoordinate();
        m_textStream << u"# filter r/%1/%2/%3 t/o"_s
                            .arg(approximatelastValidCoordinate.latitude(), 1, 'f', 4)
                            .arg(approximatelastValidCoordinate.longitude(), 1, 'f', 4)
                            .arg(qRound(m_receiveRadius.toKM()));
        m_textStream.flush();
    });
}

Traffic::TrafficDataSource_Ogn::~TrafficDataSource_Ogn()
{
    Traffic::TrafficDataSource_Ogn::disconnectFromTrafficReceiver();
    setReceivingHeartbeat(false); // This will release the WiFi lock if necessary
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
    if (!m_socket.isOpen())
    {
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
    // Send a keep-alive message (newline character as per APRS-IS protocol)
    m_textStream << "# " << QCoreApplication::organizationName() << QCoreApplication::applicationName() << QCoreApplication::applicationVersion() << "\n";
    m_textStream.flush();
}

void Traffic::TrafficDataSource_Ogn::verifyConnection()
{
    if (!m_socket.isOpen() || m_socket.state() != QAbstractSocket::ConnectedState)
    {
        qWarning() << "Connection to OGN APRS-IS server lost. Reconnecting...";
        disconnectFromTrafficReceiver();
        connectToTrafficReceiver();
    }
}
