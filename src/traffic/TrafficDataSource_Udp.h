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
#include <QUdpSocket>

#include "traffic/TrafficDataSource_AbstractSocket.h"


namespace Traffic {

/*! \brief Traffic receiver: UDP connection to GDL90 source
 *
 *  This class connects to a traffic receiver via a GDL90 connection. It expects
 *  to find a receiver at the specifed IP-Address and port that emits GDL90
 *  messages.
 *
 *  In most use cases, the connection will be established via the device's WiFi
 *  interface.  The class will therefore try to lock the WiFi once a heartbeat
 *  has been detected, and release the WiFi at the appropriate time.
 */
class TrafficDataSource_Udp : public TrafficDataSource_AbstractSocket {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     *  @param port Port at the host where the traffic receiver is expected
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit TrafficDataSource_Udp(quint16 port, QObject *parent = nullptr);

    // Standard destructor
    ~TrafficDataSource_Udp() override;

    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property sourceName
     */
    [[nodiscard]] auto sourceName() const -> QString override
    {
        return tr("UDP connection to port %1").arg(m_port);
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

private slots:
    // Read messages from the socket datagrams and passes the messages on to
    // processGDLMessage
    void onReadyRead();

private:
    QPointer<QUdpSocket> m_socket;
    quint16 m_port;

    // We use this vector to store the last 512 datatgram hashes in a circular
    // array. This is used to sort out doubly sent datagrams. The nextHashIndex
    // points to the next vector entry that will be re-written.
    QVector<uint> receivedDatagramHashes {512, 0};
    qsizetype nextHashIndex {0};

    // GPS altitude of owncraft
    Units::Distance m_trueAltitude;
    Units::Distance m_trueAltitude_FOM;
    QTimer m_trueAltitudeTimer;

};

} // namespace Traffic
