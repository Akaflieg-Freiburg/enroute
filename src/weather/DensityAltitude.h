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

namespace Weather {

/*! \brief This class calculates the density altiude according to temperature, QNH,  Altitude (and dewpoint) */
class DensityAltitude {
public:
    //
    // Methods
    //

    /*! \brief calculates the density altiude without dewpoint information
     * 
     * @param oat outside air temperature in degrees Celsius.
     * @param qnh The atmospheric pressure at sea level in hPa.
     * @param geometricAltitudeInFt The elevation above sea level in feet.
     * @returns dry air approximation of density altiude according to https://aerotoolbox.com/density-altitude/
     */
    static double calculateDensityAltitudeDryAirApproximation(double oat, double qnh, double geometricAltitudeInFt);

    
    /*! \brief 
    * 
    * @param oat outside air temperature in celsius
    * @param qnh The atmospheric pressure at sea level in hPa.
    * @param geometricAltitudeInFt The elevation above sea level in feet.
    * @param dewPoint dew point in degrees Celsius.
    * @returns density altiude according to https://aerotoolbox.com/density-altitude/
    */
    static double calculateDensityAltitude(double oat, double qnh, double geometricAltitudeInFt, double dewPoint);
private:
    // calculates the actual density altitude either with or without a given dewpoint
    static double calculateDensityAltitudeInternal(double oat, double qnh, double geometricAltitudeInFt, double dewPoint, bool bWithDewPoint);

    // converts the geometricAltitude into geopotential altitude
    static double geometricAltitudeToGeopotentialAltitude(double geometricAltitudeInKm);

    // calculates the air density either with or without a given dewpoint
    static double calculateAirDensity(double temperature, double qnh, double height, double dewPoint, bool bWithDewPoint);

    // calculates the air density
    static double calculateAbsolutePressure(double qnh, double height);

    // calculates the vapour pressure using the Herman Wobus approximation polynominal curve
    static double calculateVapourPressure(double dewPoint);
};

} // namespace Weather
