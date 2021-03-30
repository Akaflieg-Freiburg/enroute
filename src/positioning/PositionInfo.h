/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include "units/Angle.h"
#include "units/Distance.h"
#include "units/Speed.h"

namespace Positioning {

/*! \brief Geographic position
 *
 *  This class is a thin wrapper around QGeoPositionInfo. It exports the data from QGeoPositionInfo
 *  in a way that can be read from QML. In addition, the class also contains information about the pressure altitude
 */

class PositionInfo
{
    Q_GADGET

public:
    /*! \brief Default Constructor */
    PositionInfo() = default;

    /*! \brief Constructor
     *
     * @param info QGeoPositionInfo this is copied into this class
     */
    explicit PositionInfo(const QGeoPositionInfo &info, AviationUnits::Distance pressureAlt);

    /*! \brief Coordinate
     *
     *  If the coordinate contains altitude information, this refers to
     *  true altitude.
     *
     *  @returns Coordinate
     */
    Q_INVOKABLE QGeoCoordinate coordinate() const
    {
        return m_positionInfo.coordinate();
    }

    /*! \brief Ground speed
     *
     *  @returns Ground speed or NaN if unknown.
     */
    Q_INVOKABLE AviationUnits::Speed groundSpeed() const;

    /*! \brief Validity
     *
     *  @returns True if position info is valid
     */
    Q_INVOKABLE bool isValid() const
    {
        return m_positionInfo.isValid();
    }

    /*! \brief Position error estimate
     *
     *  @returns Position error estimate or NaN if unknown.
     */
    Q_INVOKABLE AviationUnits::Distance positionErrorEstimate() const;

    /*! \brief Pressure altitude
     *
     *  This is the altitude that your altimeter will show if set to 1013,2 hPa.
     *
     *  @returns Pressure altitude or NaN if unknown.
     */
    Q_INVOKABLE AviationUnits::Distance pressureAltitude() const
    {
        return m_pressureAltitude;
    }

    /*! \brief True Altitude
     *
     *  @returns True altitude with geoid correction taken into account or NaN if unknown.
     */
    Q_INVOKABLE AviationUnits::Distance trueAltitude() const;

    /*! \brief True altitude error estimate
     *
     *  @returns True altitude error estimate or NaN if unknown.
     */
    Q_INVOKABLE AviationUnits::Distance trueAltitudeErrorEstimate() const;

    /*! \brief True track
     *
     *  @returns True track or NaN if unknown.
     */
    Q_INVOKABLE AviationUnits::Angle trueTrack() const;

    /*! \brief Magnetic variation
     *
     *  @returns Magnetic variation or NaN if unknown.
     */
    Q_INVOKABLE AviationUnits::Angle variation() const;

    /*! \brief Vertical speed
     *
     *  @returns Vertical speed or NaN if unknown.
     */
    Q_INVOKABLE AviationUnits::Speed verticalSpeed() const;

private:
    QGeoPositionInfo m_positionInfo {};
    AviationUnits::Distance m_pressureAltitude {};
};

}

Q_DECLARE_METATYPE(Positioning::PositionInfo)
