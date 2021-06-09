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

#include <QGeoPositionInfo>

#include "traffic/TrafficDataSource_Abstract.h"


namespace Traffic {

/*! \brief Traffic receiver: Simulator file with FLARM/NMEA sentences
 *
 *  For testing purposes, this class connects to a simulator file with time
 *  stamps and FLARM/NMEA sentences, as provided by FLARM Inc.
 */
class TrafficDataSource_Simulate : public TrafficDataSource_Abstract {
    Q_OBJECT

public:
    /*! \brief Default constructor
     *
     *  @param fileName Name of the simulator file
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit TrafficDataSource_Simulate(QObject *parent = nullptr);

    // Standard destructor
    ~TrafficDataSource_Simulate() override = default;



    /*! \brief Getter function for the property with the same name
     *
     *  This method implements the pure virtual method declared by its
     *  superclass.
     *
     *  @returns Property sourceName
     */
    QString sourceName() const override
    {
        return tr("Simulator data");
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
#warning doku
    void sendSimulatorData();

private:
    // Simulator related members
    QTimer simulatorTimer;
};

}
