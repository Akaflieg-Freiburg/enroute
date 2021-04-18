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

#include <QAbstractSocket>

#include "traffic/TrafficDataSource_Abstract.h"


namespace Traffic {

/*! \brief Base class for all traffic receiver data sources that receive data via network sockets
 *
 *  This is an abstract base class for all classes that connect to a traffic
 *  receiver via network sockets.  In addition to TrafficDataSource_Abstract, it contains a few protected methods
 *  that will might be useful for Tcp and Udp connections.
 *
 *  It is assume that most users will connect to their traffic receicers via the WiFi network.  On Android, this
 *  class will therefore acquire/relase a WiFi whenever traffic receiver heartbeat messages are detected or lost.
 */

class TrafficDataSource_AbstractSocket : public TrafficDataSource_Abstract {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TrafficDataSource_AbstractSocket(QObject *parent = nullptr);

    // Standard destructor
    ~TrafficDataSource_AbstractSocket() override;

protected slots:
    // Handle socket errors. This method will call TrafficDataSource_Abstract::setErrorString() with a suitable,
    // human-readable, translated error message.
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

    // Handle socket state changes. This method will call TrafficDataSource_Abstract::setConnectivityStatus() with a suitable,
    // human-readable, translated message.
    void onStateChanged(QAbstractSocket::SocketState socketState);

private slots:
    // Acquire or release WiFi lock
    static void onReceivingHeartbeatChanged(bool receivingHB);


};

}
