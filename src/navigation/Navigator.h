/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include "positioning/PositionInfo.h"


namespace Navigation {

/*! \brief Main hub for navigation data
 *
 *  This class collects all data items that are relevant for navigation.
 *
 *  The methods in this class are reentrant, but not thread safe.
 */

class Navigator : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Navigator(QObject *parent = nullptr);

    /*! \brief Pointer to static instance
     *
     *  This method returns a pointer to a static instance of this class. In rare
     *  situations, during shutdown of the app, a nullptr might be returned.
     *
     *  @returns A pointer to a static instance of this class
     */
    static Navigator *globalInstance();

    /*! \brief Estimate whether the device is flying or on the ground
     *
     *  This property holds an estimate, as to whether the device is flying or
     *  on the ground.  The current implementation considers the device is
     *  flying if the groundspeed can be read and is greater then 30 knots.
     */
    Q_PROPERTY(bool isInFlight READ isInFlight NOTIFY isInFlightChanged)

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property isInFlight
     */
    bool isInFlight() const
    {
        return m_isInFlight;
    }

signals:
    /*! \brief Notifier signal */
    void isInFlightChanged();

private slots:
    // Connected to sources, in order to receive new data
    void onPositionUpdated(const Positioning::PositionInfo& info);


private:
    Q_DISABLE_COPY_MOVE(Navigator)

    // Aircraft is considered flying if speed is at least this high
    static constexpr double minFlightSpeedInKN = 30.0;
    // Hysteresis for flight speed
    static constexpr double flightSpeedHysteresisInKn = 5.0;

    bool m_isInFlight {false};
};

}
