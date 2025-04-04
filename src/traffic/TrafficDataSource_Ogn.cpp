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
#include "GlobalObject.h"
#include "positioning/PositionProvider.h"
#include "TrafficDataSource_OgnParser.h"
#include "TrafficFactor_WithPosition.h"
#include "TrafficFactorAircraftType.h"
#include "positioning/PositionInfo.h"

using namespace Qt::Literals::StringLiterals;

Traffic::TrafficDataSource_Ogn::TrafficDataSource_Ogn(bool isCanonical, QString hostName, quint16 port, QObject *parent) :
    Traffic::TrafficDataSource_AbstractSocket(isCanonical, parent),
    m_hostName(hostName.isEmpty() ? "aprs.glidernet.org" : std::move(hostName)), // Default to aprs.glidernet.org
    m_port(port == 0 ? 14580 : port) // Default to port 14580
{
    // Generate a random 5-digit number for the call sign
    m_callSign = QString("ENR%1").arg(QRandomGenerator::global()->bounded(10000, 99999));

    // Connect socket
    connect(&m_socket, &QTcpSocket::connected, this, &Traffic::TrafficDataSource_Ogn::sendLoginString);
    connect(&m_socket, &QTcpSocket::errorOccurred, this, &Traffic::TrafficDataSource_Ogn::onErrorOccurred);
    connect(&m_socket, &QTcpSocket::readyRead, this, &Traffic::TrafficDataSource_Ogn::onReadyRead);
    connect(&m_socket, &QTcpSocket::stateChanged, this, &Traffic::TrafficDataSource_Ogn::onStateChanged);
    connect(&m_socket, &QAbstractSocket::disconnected, this, &Traffic::TrafficDataSource_Ogn::connectToTrafficReceiver, Qt::ConnectionType::QueuedConnection);

    // Set up text stream
    m_textStream.setDevice(&m_socket);
    m_textStream.setEncoding(QStringConverter::Latin1);

    // Initialize properties
    onStateChanged(m_socket.state());
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

QString Traffic::TrafficDataSource_Ogn::calculatePassword(const QString& callSign) const
{
    // APRS-IS passcode calculation: Sum of ASCII values of the first 6 characters of the call sign
    int sum = 0;
    for (int i = 0; i < callSign.length() && i < 6; ++i) {
        sum += callSign.at(i).unicode();
    }
    return QString::number(sum % 10000); // Modulo 10000 to get a 4-digit passcode
}

void Traffic::TrafficDataSource_Ogn::sendLoginString()
{
    // Calculate the password based on the call sign
    QString passcode = calculatePassword(m_callSign);

    // Login string
    QString loginString = QString("user %1 pass %2 vers %3 %4 filter %5\n")
                              .arg(m_callSign)          // Use stored call sign
                              .arg(passcode)            // Calculated passcode
                              .arg("enroute")           // Software name
                              .arg("1.0")               // Software version
                              .arg("r/47.9/12.3/100");  // Radius filter: 48.5°N, 10.0°E, 200 km

    m_textStream << loginString;
    m_textStream.flush();

    qDebug() << "Sent login string:" << loginString;
}

void Traffic::TrafficDataSource_Ogn::onReadyRead()
{
    QString sentence;
    while (m_textStream.readLineInto(&sentence)) {
        // Process APRS-IS sentence
        auto ognMessage = Ogn::TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

        switch(ognMessage.type)
        {
            case Traffic::Ogn::OgnMessageType::TRAFFIC_REPORT:
            {
                if (!ognMessage.coordinate.isValid()) {
                    qDebug() << "Invalid coordinate for traffic report:" << sentence;
                    return;
                }

                QGeoPositionInfo pInfo(ognMessage.coordinate, QDateTime::currentDateTimeUtc());
                pInfo.setAttribute(QGeoPositionInfo::Direction, ognMessage.course.toDouble());
                pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, ognMessage.speed.toDouble());
                pInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, ognMessage.verticalSpeed.toDouble());
                if (!pInfo.isValid()) {
                    qDebug() << "Invalid position-info for traffic report:" << sentence;
                    return;
                }

                // Compute horizontal and vertical distance to traffic if our own position is known.
                Units::Distance hDist {};
                Units::Distance vDist {};
                auto* positionProviderPtr = GlobalObject::positionProvider();
                if (positionProviderPtr != nullptr) {
                    auto ownShipCoordinate = positionProviderPtr->positionInfo().coordinate();
                    if (ownShipCoordinate.isValid()) {
                        hDist = Units::Distance::fromM( ownShipCoordinate.distanceTo(ognMessage.coordinate) );
                        vDist = Units::Distance::fromM( ognMessage.coordinate.altitude()) - Units::Distance::fromM(ownShipCoordinate.altitude() );
                    }
                    else
                    {
                        qDebug() << "ownShipCoordinate is invalid, cannot compute distances.";
                        hDist = Units::Distance::fromM(30000);
                    }
                }
                else
                {
                    qDebug() << "Position provider is null, cannot compute distances.";
                    hDist = Units::Distance::fromM(30000);
                }

                // Prepare the m_factor object
                m_factor.setAlarmLevel(0);
                m_factor.setCallSign(ognMessage.sourceId.toString());
                m_factor.setID(ognMessage.sourceId.toString());
                m_factor.setType(ognMessage.aircraftType);
                m_factor.setPositionInfo(Positioning::PositionInfo(pInfo, sourceName()));
                m_factor.setHDist(hDist);
                m_factor.setVDist(vDist);
                m_factor.startLiveTime();
            
                // Emit the factorWithPosition signal
                setReceivingHeartbeat(true);
                emit factorWithPosition(m_factor);
                break;
            }
            default:
                break;
        }
    }
}
