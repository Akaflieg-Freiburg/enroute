/***************************************************************************
 *   Copyright (C) 2021-2026 by Stefan Kebekus                             *
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

#include <QGeoCoordinate>
#include <QString>

#include "positioning/PositionInfo.h"
#include "traffic/TrafficFactor_Abstract.h"
#include "units/Distance.h"


namespace Traffic {

/*! \brief Plain-data record of a traffic factor
 *
 *  This is a lightweight, copyable value type that carries the data of a
 *  traffic factor as parsed by a traffic data source. Unlike
 *  TrafficFactor_Abstract, it is not a QObject: it has no bindings, no lifetime
 *  counter and no derived/animated properties. Data sources construct records
 *  of this type and ship them to the TrafficDataProvider through signals; the
 *  provider then feeds them into its long-lived TrafficFactor_Abstract models
 *  via updateFrom()/replaceBy().
 *
 *  This struct holds the position-independent data shared by all traffic
 *  factors. The variants below add position information.
 */
struct TrafficFactorData {
    /*! \brief Alarm level, in the range 0…3 */
    int alarmLevel = 0;

    /*! \brief Call sign, or an empty string if unknown */
    QString callSign;

    /*! \brief Horizontal distance to traffic, or an invalid distance if unknown */
    Units::Distance hDist;

    /*! \brief Identifier string, as assigned by the traffic receiver */
    QString ID;

    /*! \brief Aircraft type */
    TrafficFactor_Abstract::Type type = TrafficFactor_Abstract::unknown;

    /*! \brief Vertical distance to traffic, or an invalid distance if unknown */
    Units::Distance vDist;
};


/*! \brief Plain-data record of a traffic factor whose precise position is known */
struct TrafficFactorData_WithPosition {
    /*! \brief Position-independent data */
    TrafficFactorData data;

    /*! \brief PositionInfo of the traffic */
    Positioning::PositionInfo positionInfo;
};


/*! \brief Plain-data record of a traffic factor whose position is not known
 *
 *  Such a factor is reported only with a distance; the coordinate holds the
 *  center of the cylinder where the traffic is most likely located.
 */
struct TrafficFactorData_DistanceOnly {
    /*! \brief Position-independent data */
    TrafficFactorData data;

    /*! \brief Center coordinate of the range ring where the traffic is located */
    QGeoCoordinate coordinate;
};


/*! \brief Priority comparison between a freshly received data record and a live model
 *
 *  This free function answers the question "should the incoming traffic factor
 *  \a lhs displace the existing model object \a rhs?". It applies the same
 *  precedence as TrafficFactor_Abstract::hasHigherPriorityThan(), treating \a
 *  lhs as valid (a freshly parsed record always carries current data).
 *
 *  @param lhs Freshly received data record
 *
 *  @param rhs Existing, long-lived traffic factor
 *
 *  @returns True if \a lhs has higher priority than \a rhs
 */
[[nodiscard]] bool hasHigherPriorityThan(const TrafficFactorData& lhs, const TrafficFactor_Abstract& rhs);

} // namespace Traffic
