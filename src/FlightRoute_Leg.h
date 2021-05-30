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

#include <QPointer>

#include "FlightRoute.h"
#include "units/Angle.h"
#include "units/Distance.h"
#include "units/Time.h"


/*! \brief Leg in a flight route */

class FlightRoute::Leg : public QObject
{
    Q_OBJECT

public:
    /*! \brief Constructs a flight route leg with given start and end point
   *
   * @param start Start point
   *
   * @param end End point
   *
   * @param aircraft Pointer to aircraft info that is used in route
   * computations. It is possible to set a nullptr here or delete the object anytime, but then wind computations will no longer work.
   *
   * @param wind Pointer to wind info that is used in route computations. It is possible to set a nullptr here or delete the object anytime, but then wind computations will no longer work.
   *
   * @param parent The standard QObject parent pointer.
   */
    explicit Leg(const GeoMaps::Waypoint& start, const GeoMaps::Waypoint& end, Aircraft *aircraft, Weather::Wind *wind, QObject *parent = nullptr);

    // Standard destructor
    ~Leg() override = default;

    /*! \brief Start point of the leg */
    Q_PROPERTY(GeoMaps::Waypoint startPoint READ startPoint CONSTANT)

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property startPoint
     */
    GeoMaps::Waypoint startPoint()  const
    {
        return _start;
    }

    /*! \brief End point of the leg */
    Q_PROPERTY(GeoMaps::Waypoint endPoint READ endPoint CONSTANT)

    /*! \brief Getter function for property of the same name
     *
     *  @returns Property endPoint
     */
    GeoMaps::Waypoint endPoint()  const
    {
        return _end;
    }

    /*! \brief Length of the leg */
    Q_PROPERTY(AviationUnits::Distance distance READ distance CONSTANT)

    /*! \brief Getter function for property of the same name
   *
   * @returns Property distance
   */
    AviationUnits::Distance distance() const;

    /*! \brief Fuel
   *
   * This property holds the fuel consumption for the leg, in liters. It holds
   * NaN if the leg is invalid or if the fuel consumption cannot be computed.
   */
    Q_PROPERTY(double Fuel READ Fuel NOTIFY valChanged)

    /*! \brief Getter function for property of the same name
   *
   * @returns Property Fuel
   */
    double Fuel() const;

    /*! \brief Ground speed
   *
   * This property holds the ground speed for the leg, in meters per second. It
   * holds NaN if the leg is invalid or if the ground speed cannot be computed.
   */
    Q_PROPERTY(AviationUnits::Speed GS READ GS NOTIFY valChanged)

    /*! \brief Getter function for property of the same name
   *
   * @returns Property GS
   */
    AviationUnits::Speed GS() const;

    /*! \brief True course
   *
   * This property holds the true course for the leg. It holds NaN if the leg is
   * invalid or shorter than 100m
   */
    Q_PROPERTY(AviationUnits::Angle TC READ TC NOTIFY valChanged)

    /*! \brief Getter function for property of the same name
   *
   * @returns Property TC
   */
    AviationUnits::Angle TC() const;

    /*! \brief Time required for this leg.
   *
   * Set to NaN if the time cannot be computed.
   */
    Q_PROPERTY(AviationUnits::Time Time READ Time NOTIFY valChanged)

    /*! \brief Getter function for property of the same name
   *
   * @returns Property Time
   */
    AviationUnits::Time Time() const{ return distance()/GS(); }

    /*! \brief True heading.
   *
   * Set to NaN if a TH cannot be computed.
   */
    Q_PROPERTY(AviationUnits::Angle TH READ TH CONSTANT)

    /*! \brief Getter function for property of the same name
   *
   * @returns Property TH
   */
    AviationUnits::Angle TH() const { return TC()+WCA(); }

    /*! \brief Human-readable description of the leg
   *
   *  This method uses the global settings object to determine if metric or
   *  aviation units shall be used.
   */
    Q_PROPERTY(QString description READ description NOTIFY valChanged)

    /*! \brief Getter function for property of the same name
   *
   * @returns Property description
   */
    QString description() const;

    /*! \brief Validity
   *
   * A leg is considered invalid of either start or endpoint are invalid.
   *
   * @returns True if the leg is valid
   */
    bool isValid() const;

    /*! \brief Wind correction angle.
   *
   * @returns Wind correction angle, or NaN if a WCA cannot be computed
   */
    AviationUnits::Angle WCA() const;

signals:
    /*! \brief Notification signal */
    void valChanged();

private:
    Q_DISABLE_COPY_MOVE(Leg)

    // Necessary data for computation of wind triangle?
    bool hasDataForWindTriangle() const;

    // Helper function for creating the text in the flight route
    QString makeDescription(bool useMetricUnits) const;

    // Minimum length of the leg in meters. If shorter, no courses are computed.
    static constexpr double minLegLength  =  100.0;

    GeoMaps::Waypoint _start;
    GeoMaps::Waypoint _end;
    QPointer<Aircraft> _aircraft {nullptr};
    QPointer<Weather::Wind> _wind {nullptr};
};
