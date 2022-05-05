/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

/*! \brief Leg in a flight route */

class Leg {
    Q_GADGET

public:

    //
    // Constructors and destructors
    //

    /*! \brief Constructs a flight route leg with given start and end point
     *
     * @param start Start point
     *
     * @param end End point
     */
    explicit Leg(GeoMaps::Waypoint  start, GeoMaps::Waypoint  end);

    // Default constructor
    explicit Leg() = default;

    //
    // PROPERTIES
    //

    /*! \brief Length of the leg */
    Q_PROPERTY(Units::Distance distance READ distance CONSTANT)

    /*! \brief End point of the leg */
    Q_PROPERTY(GeoMaps::Waypoint endPoint READ endPoint CONSTANT)

    /*! \brief Validity */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Distance threshold
     *
     *   A position is considered near the leg if the distance is less than this threshold
     */
    Q_PROPERTY(Units::Distance nearThreshold MEMBER nearThreshold CONSTANT)

    /*! \brief Start point of the leg */
    Q_PROPERTY(GeoMaps::Waypoint startPoint READ startPoint CONSTANT)

    /*! \brief True course
     *
     * This property holds the true course for the leg. It holds NaN if the leg is
     * invalid or shorter than 100m
     */
    Q_PROPERTY(Units::Angle TC READ TC CONSTANT)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     * @returns Property distance
     */
    [[nodiscard]] auto distance() const -> Units::Distance;

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property endPoint
     */
    [[nodiscard]] auto endPoint()  const -> GeoMaps::Waypoint { return m_end; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property valid
     */
    [[nodiscard]] auto isValid() const -> bool;

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property startPoint
     */
    [[nodiscard]] auto startPoint()  const -> GeoMaps::Waypoint { return m_start; }

    /*! \brief Getter function for property of the same name
     *
     * @returns Property TC
     */
    [[nodiscard]] auto TC() const -> Units::Angle;


    //
    // Methods
    //

    /*! \brief Brief description of Dist ETE, TC and THGetter function for property of the same name
     *
     *  @param wind Estimated wind
     *
     *  @param aircraft Aircraft in use
     *
     *  @returns Estimated WCA on leg
     */
    Q_INVOKABLE [[nodiscard]] auto description(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> QString;

    /*! \brief ETE for leg
     *
     *  @param wind Estimated wind
     *
     *  @param aircraft Aircraft in use
     *
     *  @returns ETE for leg
     */
    Q_INVOKABLE [[nodiscard]] auto ETE(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> Units::Time{ return distance()/GS(wind, aircraft); }

    /*! \brief Estimated fuel consumption on leg
     *
     *  @param wind Estimated wind
     *
     *  @param aircraft Aircraft in use
     *
     *  @returns Estimated fuel consumption on leg
     */
    Q_INVOKABLE [[nodiscard]] auto Fuel(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> Units::Volume;

    /*! \brief Estimated ground speed on leg
     *
     *  @param wind Estimated wind
     *
     *  @param aircraft Aircraft in use
     *
     *  @returns Estimated ground speed on leg
     */
    Q_INVOKABLE [[nodiscard]] auto GS(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> Units::Speed;

    /*! \brief Check if positionInfo is travelling on this leg
     *
     *  The positionInfo is considered to be travelling along this leg if
     *
     *  - isNear() is true, and
     *  - the true track (TT) is known, and
     *  - the difference TT-QUJ(endPoint) is in [-60, 60] degrees
     *  - the difference TT-QUJ(startPoint) is in [120, 240] degrees
     *
     *  @param positionInfo PositionInfo that is checked to be following
     *
     *  @return True if the conditions are met
     */
    Q_INVOKABLE [[nodiscard]] auto isFollowing(const Positioning::PositionInfo& positionInfo) const -> bool;

    /*! \brief Check if position is closer than nearThreshold to this leg
     *
     *  @param positionInfo Position that is checked to be near
     *
     *  @return True if all data is valid and position is closer than nearThreshold to this leg.
     */
    Q_INVOKABLE [[nodiscard]] auto isNear(const Positioning::PositionInfo& positionInfo) const -> bool;

    /*! \brief Estimated true heading on leg
     *
     *  @param wind Estimated wind
     *
     *  @param aircraft Aircraft in use
     *
     *  @returns Estimated true heading on leg
     */
    Q_INVOKABLE [[nodiscard]] auto TH(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> Units::Angle { return TC()+WCA(wind, aircraft); }

    /*! \brief Estimated WCA on leg
     *
     *  @param wind Estimated wind
     *
     *  @param aircraft Aircraft in use
     *
     *  @returns Estimated WCA on leg
     */
    Q_INVOKABLE [[nodiscard]] auto WCA(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> Units::Angle;


    //
    // Members and constants
    //

    // Width of the leg. A position is considered near the leg if the distance is less.
    static constexpr Units::Distance nearThreshold = Units::Distance::fromNM(3.0);

private:
    // Necessary data for computation of wind triangle?
    [[nodiscard]] auto hasDataForWindTriangle(Weather::Wind wind, const Navigation::Aircraft& aircraft) const -> bool;

    // Minimum length of the leg in meters. If shorter, no courses are computed.
    static constexpr Units::Distance minLegLength = Units::Distance::fromM(100.0);

    GeoMaps::Waypoint m_start;
    GeoMaps::Waypoint m_end;
    QGeoPath m_geoPath;
};

}

// Declare meta types
Q_DECLARE_METATYPE(Navigation::Leg)
