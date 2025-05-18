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

#pragma once

#include <QPointer>
#include <QTcpSocket>

#include "traffic/TrafficDataSource_AbstractSocket.h"
#include "traffic/TrafficDataSource_OgnParser.h"

using namespace Qt::Literals::StringLiterals;

namespace Traffic {

/*! \brief Traffic receiver: OGN glidernet.org via internet connection.
 * 
 * Technically, this is APRS-IS protocol over TCP/IP, 
 * connecting to aprs.glidernet.org port 14580
 * and receiving sentences similar to NMEA like the following: 
 * 
 *   FLRDDE626>APRS,qAS,EGHL:/074548h5111.32N/00102.04W'086/007/A=000607 id0ADDE626 -019fpm +0.0rot 5.5dB 3e -4.3kHz
 * 
 * \see http://wiki.glidernet.org/wiki:subscribe-to-ogn-data
 * \see http://wiki.glidernet.org/wiki:ogn-flavoured-aprs
 * \see https://github.com/svoop/ogn_client-ruby/wiki/SenderBeacon
 */

class TrafficDataSource_Ogn : public TrafficDataSource_AbstractSocket {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param isCanonical Intializer for property canonical
     *
     *  @param hostName Name of the host where the traffic receiver is expected
     *
     *  @param port Port at the host where the traffic receiver is expected
     *
     *  @param parent The standard QObject parent pointer
     */
    TrafficDataSource_Ogn(bool isCanonical, QString hostName, quint16 port, QObject *parent = nullptr);

    // Standard destructor
    ~TrafficDataSource_Ogn() override;

    //
    // Properties
    //

    /*! \brief Host
     */
    Q_PROPERTY(QString host READ host CONSTANT)

    /*! \brief Port
     */
    Q_PROPERTY(quint16 port READ port CONSTANT)

    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property connectionInfo
     */
    [[nodiscard]] Traffic::ConnectionInfo connectionInfo() const override {
        return Traffic::ConnectionInfo(Traffic::ConnectionInfo::OgnInfo());
    }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property dataFormat
     */
    [[nodiscard]] QString dataFormat() const override { return u"APRS-IS"_s; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property host
     */
    [[nodiscard]] QString host() const
    {
        return m_hostName;
    }

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property icon
     */
    [[nodiscard]] QString icon() const override { return u"/icons/material/ic_wifi.svg"_s; }

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property sourceName
     */
    [[nodiscard]] auto sourceName() const -> QString override
    {
        return tr("OGN glidernet.org APRS-IS connection");
    }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property port
     */
    [[nodiscard]] quint16 port() const
    {
        return m_port;
    }

public slots:
    /*! \brief Start attempt to connect to traffic receiver
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     */
    void connectToTrafficReceiver() override;


    /*! \brief Disconnect from traffic receiver
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     */
    void disconnectFromTrafficReceiver() override;

    /*! \brief Send position report to APRS-IS server
     *
     *  This method sends a position report with the given parameters to the APRS-IS server.
     *
     *  @param coordinate The geographic coordinate (latitude, longitude, altitude).
     *  @param course The course in degrees.
     *  @param speed The speed in knots.
     *  @param altitude The altitude in meters.
     */
    void sendPosition(const QGeoCoordinate& coordinate, double course, double speed, double altitude);

private slots:
    /*! \brief Called when the socket successfully connects to the server
     *
     *  This function handles post-connection tasks such as sending the login string
     *  and an initial position report.
     */
    void onConnected();

    // Read lines from the socket's text stream and passes the string on to
    // processAprsisMessage.
    void onReadyRead();

private:
    Q_DISABLE_COPY_MOVE(TrafficDataSource_Ogn)

    QTcpSocket m_socket;
    QTextStream m_textStream;
    QString m_hostName;
    quint16 m_port;

    Traffic::Ogn::OgnMessage m_ognMessage;

    // our own CallSign
    QString m_callSign;

    // our own AircraftType
    Traffic::AircraftType m_aircraftType = {Traffic::AircraftType::Aircraft};

    // Receive Filter
    QGeoCoordinate m_receiveLocation = {48.0, 7.85, 250};
    unsigned int m_receiveRadius = {50};

    // Parse and process APRS-IS messages
    void processAprsisMessage(const QString& message);

    // Send login string to APRS-IS server
    void sendLoginString();

    // Get own coordinates.
    // @param useLastValidPosition If true, use the last valid position if the current position is invalid.
    static QGeoCoordinate getOwnShipCoordinate(bool useLastValidPosition);

    // Periodic update function called once per minute
    void periodicUpdate();

    // Update receive position
    void updateReceivePosition(const QGeoCoordinate &position);

    // Send a keep-alive message to the APRS-IS server
    void sendKeepAlive();

    // Verify the connection to the APRS-IS server and reconnect if necessary
    void verifyConnection();
};

} // namespace Traffic
