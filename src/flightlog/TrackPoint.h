/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

#include <QDateTime>
#include <QGeoCoordinate>

namespace Flightlog {

/*! \brief A single point along a recorded flight track
 *
 *  Stores position, altitude, and timestamp for one GPS fix.
 *  GPS altitude is stored in QGeoCoordinate::altitude().
 *  Designed to be lightweight for storing thousands of points per flight.
 */
struct TrackPoint
{
    /*! \brief Geographic position (latitude, longitude, and GPS altitude in meters MSL) */
    QGeoCoordinate coordinate;

    /*! \brief Pressure altitude in meters, or NaN if unknown */
    double pressureAltitude {std::numeric_limits<double>::quiet_NaN()};

    /*! \brief UTC timestamp of this fix */
    QDateTime timestamp;

    /*! \brief Check if this track point has valid data */
    [[nodiscard]] auto isValid() const -> bool
    {
        return coordinate.isValid() && timestamp.isValid();
    }
};

} // namespace Flightlog
