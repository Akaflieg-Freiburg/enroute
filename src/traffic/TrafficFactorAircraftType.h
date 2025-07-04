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
#include <QObject>

namespace Traffic {
    Q_NAMESPACE // Ensure the namespace is registered with Qt's meta-object system

    /*! \brief Aircraft type
     *
     *  This enum defines a few aircraft types. The list is modeled after the FLARM/NMEA specification.
     */
    enum AircraftType
    {
        unknown, /*!< Unknown aircraft type */
        Aircraft, /*!< Fixed wing aircraft */
        Airship, /*!< Airship, such as a zeppelin or a blimp */
        Balloon, /*!< Balloon */
        Copter, /*!< Helicopter, gyrocopter or rotorcraft */
        Drone, /*!< Drone */
        Glider, /*!< Glider, including powered gliders and touring motor gliders */
        HangGlider, /*!< Hang glider */
        Jet, /*!< Jet aircraft */
        Paraglider, /*!< Paraglider */
        Skydiver, /*!< Skydiver */
        StaticObstacle, /*!< Static obstacle */
        TowPlane /*!< Tow plane */
    };
    Q_ENUM_NS(AircraftType) // Register the enum with Qt's meta-object system
}
