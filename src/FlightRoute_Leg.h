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

#include "AviationUnits.h"
#include "FlightRoute.h"


/*! \brief Leg in a flight route */

class FlightRoute::Leg : public QObject
{
    Q_OBJECT

public:
  /*! \brief Constructs a flight route with given start and end point
   *
   * The Waypoint start and end are copied, so that this class does not make any
   * assumption about the lifetome of *start and *end
   *
   * @param start Pointer to the starting point
   *
   * @param end Pointer to the end point
   *
   * @param aircraft Pointer to aircraft info that is used in route
   * computations. It is possible to set a nullptr here or delete the object anytime, but then wind computations will no longer work.
   *
   * @param wind Pointer to wind info that is used in route computations. It is possible to set a nullptr here or delete the object anytime, but then wind computations will no longer work.
   *
   * @param parent The standard QObject parent pointer.
   */
  explicit Leg(const GeoMaps::Waypoint* start, const GeoMaps::Waypoint *end, Aircraft *aircraft, Weather::Wind *wind, QObject *parent = nullptr);

  // Standard destructor
  ~Leg() override = default;

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

  /*! \brief Human-readable description of the leg */
  Q_PROPERTY(QString description READ description NOTIFY valChanged)

  /*! \brief Getter function for property of the same name
   *
   * @returns Property description
   */
  QString description() const;

  /*! \brief Human-readable description of the leg */
  Q_PROPERTY(QString descriptionMetric READ descriptionMetric NOTIFY valChanged)

  /*! \brief Getter function for property of the same name
   *
   * @returns Property descriptionMetric
   */
  QString descriptionMetric() const;

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

  QPointer<GeoMaps::Waypoint> _start {nullptr};
  QPointer<GeoMaps::Waypoint> _end {nullptr};
  QPointer<Aircraft> _aircraft {nullptr};
  QPointer<Weather::Wind> _wind {nullptr};
};
