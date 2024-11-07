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

#include "units/Density.h"
#include "units/Distance.h"
#include "units/Pressure.h"
#include "units/Temperature.h"

namespace Weather {

/*! \brief This class calculates the density altiude according to temperature, QNH,  Altitude (and dewpoint) */
class DensityAltitude {
public:
    //
    // Methods
    //

    /*! \brief calculates the density altiude without dewpoint information
     * 
     * @param oat outside air temperature
     * @param qnh The atmospheric pressure at sea level
     * @param geometricAltitude The elevation above sea level
     * @returns dry air approximation of density altiude according to https://aerotoolbox.com/density-altitude/
     */
    static Units::Distance calculateDensityAltitudeDryAirApproximation(Units::Temperature oat, Units::Pressure qnh, Units::Distance geometricAltitude);

    
    /*! \brief  calculates the density altiude with dewpoint information
    * 
    * @param oat outside air temperature
    * @param qnh The atmospheric pressure at sea level
    * @param geometricAltitude The elevation above sea level
    * @param dewPoint dew point
    * @returns density altiude according to https://aerotoolbox.com/density-altitude/
    */
    static Units::Distance calculateDensityAltitude(Units::Temperature oat, Units::Pressure qnh, Units::Distance geometricAltitude, Units::Temperature dewPoint);
private:
    // calculates the actual density altitude either with or without a given dewpoint
    static Units::Distance calculateDensityAltitudeInternal(Units::Temperature oat, Units::Pressure qnh, Units::Distance geometricAltitude, Units::Temperature dewPoint, bool bWithDewPoint);

    // calculates the vapour pressure using the Herman Wobus approximation polynominal curve
    static Units::Pressure calculateVapourPressure(Units::Temperature dewPoint);

    // calculates the air density
    static Units::Pressure calculateAbsolutePressure(Units::Pressure qnh, Units::Distance height);

    // calculates the air density either with or without a given dewpoint
    static Units::Density calculateAirDensity(Units::Temperature temperature, Units::Pressure qnh, Units::Distance height, Units::Temperature dewPoint, bool bWithDewPoint);


    // converts the geometricAltitude into geopotential altitude
    static Units::Distance geometricAltitudeToGeopotentialAltitude(Units::Distance geometricAltitude);
};

} // namespace Weather
