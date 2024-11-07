/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include "units/Density.h"
#include "units/Distance.h"
#include "units/Pressure.h"
#include "units/Temperature.h"


namespace Navigation {

/*! \brief Atmospherical data
 *
 *  This class provides the standard computation routines for the ICAO atmosphere, for the height range of 0-11km
 */

class Atmosphere
{
    Q_GADGET
    QML_VALUE_TYPE(atmosphere)

public:
    /*! \brief Computation of density as a function of pressure and temperature, using the perfect gas law
     *
     *  @param p Pressure for which the density is computed
     *
     *  @param t Temperature for which the density is computed
     *
     *  @returns Air density
     */
    Q_INVOKABLE static Units::Density density(Units::Pressure p, Units::Temperature t);

    /*! \brief Computation of density as a function of height
     *
     *  @param h height
     *
     *  @returns Air density
     */
    Q_INVOKABLE static Units::Density density(Units::Distance h);

    /*! \brief Computation of height as a function of density
     *
     *  @param d Air density
     *
     *  @returns Barometric height above the 1013.25 hPa level (which equals 0 Meter)
     */
    Q_INVOKABLE static Units::Distance height(Units::Density d);

    /*! \brief Computation of height as a function of pressure
     *
     *  @param pressure Air pressure
     *
     *  @returns Barometric height above the 1013.25 hPa level (which equals 0 Meter)
     */
    Q_INVOKABLE static Units::Distance height(Units::Pressure pressure);

    /*! \brief Computation of pressure as a function of height
     *
     *  @param height Barometric height above the 1013.25 hPa level (which equals 0 Meter)
     *
     *  @returns Air pressure
     */
    Q_INVOKABLE static Units::Pressure pressure(Units::Distance height);

    /*! \brief Computation of relative humidity as a function of temperature and dewpoint
     *
     *  @param temperature Air temperature
     *
     *  @param dewpoint Dew point

     *  @returns Relative humidity in the range 0..100, or NAN in case of an error
     */
    Q_INVOKABLE static double relativeHumidity(Units::Temperature temperature, Units::Temperature dewpoint);
};

} // namespace Navigation
