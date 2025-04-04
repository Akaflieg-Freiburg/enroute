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
#include "traffic/TrafficDataSource_OgnParser.h"
#include <QRandomGenerator>
#include <QMap>
#include <QFile>
#include <QRegularExpression>

using namespace Qt::Literals::StringLiterals;

Traffic::TrafficDataSource_Ogn::TrafficDataSource_Ogn(bool isCanonical, QString hostName, quint16 port, QObject *parent) :
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
}

Traffic::TrafficDataSource_Ogn::~TrafficDataSource_Ogn()
{

    Traffic::TrafficDataSource_Ogn::disconnectFromTrafficReceiver();
}

void Traffic::TrafficDataSource_Ogn::connectToTrafficReceiver()
{
    qDebug() << "Connecting to traffic receiver at" << m_hostName << ":" << m_port;

    // Check if already connected
    if (m_socket.state() == QAbstractSocket::ConnectedState) {
        qDebug() << "Already connected to traffic receiver.";
        return;
    }

    // Start new connection
    m_socket.abort();
    m_socket.setSocketOption(QAbstractSocket::LowDelayOption, 1);
    m_socket.setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    m_socket.connectToHost(m_hostName, m_port);
    m_textStream.setDevice(&m_socket);
}

void Traffic::TrafficDataSource_Ogn::disconnectFromTrafficReceiver()
{
    qDebug() << "Disconnecting from traffic receiver";

    // Disconnect socket
    m_socket.abort();
}

QString Traffic::TrafficDataSource_Ogn::calculatePassword(const QString& callSign) const
{
    qDebug() << "Calculating password for call sign:" << callSign;

    // APRS-IS passcode calculation: Sum of ASCII values of the first 6 characters of the call sign
    int sum = 0;
    for (int i = 0; i < callSign.length() && i < 6; ++i) {
        sum += callSign.at(i).unicode();
    }
    sum = sum % 10000; // Modulo 10000 to get a 4-digit passcode
    qDebug() << "Calculated password sum:" << sum;  
    return QString::number(sum);
}

void Traffic::TrafficDataSource_Ogn::sendLoginString()
{
    // Calculate the password based on the call sign
    QString passcode = calculatePassword(m_callSign);

    // Login string
    QString loginString = QString("user %1 pass %2 vers %3 %4 filter %5\n")
                              .arg(m_callSign)          // Use stored call sign
                              .arg(passcode)            // Calculated passcode
                              .arg("charly")            // Software name
                              .arg("1.0")               // Software version
                              .arg("r/48.5/10.0/200");  // Radius filter: 48.5°N, 10.0°E, 200 km

    m_textStream << loginString;
    m_textStream.flush();

    qDebug() << "Sent login string:" << loginString;
}

void Traffic::TrafficDataSource_Ogn::onErrorOccurred()
{
    qDebug() << "ErrorOccurred: " << m_socket.errorString();
}

void Traffic::TrafficDataSource_Ogn::onStateChanged()
{
    qDebug() << "State Changed: " << m_socket.state();
}

void Traffic::TrafficDataSource_Ogn::onReadyRead()
{
    qDebug() << "Received Data";

    QString sentence;
    while (m_textStream.readLineInto(&sentence)) {
        // Process APRS-IS sentence
        auto ognMessage = Ogn::TrafficDataSource_OgnParser::parseAprsisMessage(sentence);

        // Open a file and write the sentence into it
        QFile file("received_data.txt");
        if (file.open(QIODevice::Append | QIODevice::Text)) {
            QTextStream out(&file);
            out << sentence << "\n";
            file.close();
        } else {
            qDebug() << "Failed to open file for writing:" << file.errorString();
        }

        // switch(ognMessage.type)
        // {
        //     case Traffic::OgnMessageType::TRAFFIC_REPORT:
        //         QGeoCoordinate targetCoordinate;
        //         targetCoordinate.setLatitude(ognMessage.latitude.toDouble());
        //         targetCoordinate.setLongitude(ognMessage.longitude.toDouble());
        //         targetCoordinate.setAltitude(ognMessage.altitudeValue.toDouble());

        //         QGeoPositionInfo pInfo(targetCoordinate, QDateTime::currentDateTimeUtc());
        //         pInfo.setAttribute(QGeoPositionInfo::Direction, ognMessage.course.toDouble());
        //         pInfo.setAttribute(QGeoPositionInfo::GroundSpeed, ognMessage.speed.toDouble());
        //         pInfo.setAttribute(QGeoPositionInfo::VerticalSpeed, ognMessage.verticalSpeed.toDouble());
            
        //         // Prepare the m_factor object
        //         m_factor.setAlarmLevel(0);
        //         //m_factor.setCallSign(GlobalObject::flarmnetDB()->getRegistration(address.toString()));
        //         m_factor.setCallSign(ognMessage.address);
        //         m_factor.setID(ognMessage.address);
        //         m_factor.setType(ognMessage.aircraftType);
        //         m_factor.setPositionInfo(Positioning::PositionInfo(pInfo, sourceName()));
        //         m_factor.startLiveTime();
            
        //         // Emit the factorWithPosition signal
        //         emit factorWithPosition(m_factor);
        //         break;
        //     default:
        //         break;
        // }
    }
}
