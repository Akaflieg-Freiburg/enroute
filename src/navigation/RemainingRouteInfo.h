/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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

#include <QGeoPath>

#include "geomaps/Waypoint.h"
#include "navigation/Aircraft.h"
#include "positioning/PositionInfo.h"
#include "units/Angle.h"
#include "units/Units.h"
#include "weather/Wind.h"

namespace Navigation {

/*! \brief Info about remaining route
 *
 *  This class bundles information about the remainder of the present flight. This includes
 *  next waypoint, final waypoint, as well as ETE, distance and expected fuel consumption.
 */

class RemainingRouteInfo {
    Q_GADGET

public:

    //
    // Constructors and destructors
    //

    /*! \brief Constructs a remaining route info */
    explicit RemainingRouteInfo();

    //
    // PROPERTIES
    //

    // ...


    //
    // Getter Methods
    //

    // ...


    //
    // Methods
    //

    // ...

private:
    // ...

    GeoMaps::Waypoint m_start;
    GeoMaps::Waypoint m_end;
};

}

// Declare meta types
Q_DECLARE_METATYPE(Navigation::RemainingRouteInfo)
