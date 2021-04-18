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

#pragma once

#include <QPointer>
#include <QTcpSocket>

#include "traffic/TrafficDataSource_Abstract.h"


namespace Traffic {

/*! \brief Traffic receiver: TCP connection to FLARM/NMEA source
 *
 *  This class connects to a traffic receiver via a TCP connection. It expects to
 *  find a receiver at the specifed IP-Address and port that emits FLARM/NMEA sentences.
 *
 *  In most use cases, the
 *  connection will be established via the device's WiFi interface.  The class will
 *  therefore try to lock the WiFi once a heartbeat has been detected, and release the WiFi at the appropriate time.
 */
class TrafficDataSource_Tcp : public TrafficDataSource_Abstract {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     *  @param hostName Name of the host where the traffic receiver is expected
     *
     *  @param port Port at the host where the traffic receiver is expected
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficDataSource_Tcp(QString hostName, quint16 port, QObject *parent = nullptr);

    // Standard destructor
    ~TrafficDataSource_Tcp() override;

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its superclass.
     *
     *  @returns Property sourceName
     */
    QString sourceName() const override
    {
        return tr("TCP connection to %1 port %2").arg(m_hostName).arg(m_port);
    }

public slots:
    /*! \brief Start attempt to connect to traffic receiver
     *
     *  This method implements the pure virtual method declared by its superclass.
     */
    void connectToTrafficReceiver() override;

    /*! \brief Disconnect from traffic receiver
     *
     *  This method implements the pure virtual method declared by its superclass.
     */
    void disconnectFromTrafficReceiver() override;

private slots:
    // Handle socket errors.
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

    // Read one line from the socket's text stream and passes the string on to
    // processFLARMMessage.
    void readFromStream();

    // Update the property "errorString" and "connectivityStatus" and emit notification signals
    void onStateChanged(QAbstractSocket::SocketState socketState);

    // Acquire or release WiFi lock
    static void onReceivingHeartbeatChanged(bool receivingHB);

private:
    QPointer<QTcpSocket> socket;
    QTextStream textStream;
    QString m_hostName;
    quint16 m_port;
};

}
