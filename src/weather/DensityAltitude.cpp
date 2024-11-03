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


#include "weather/DensityAltitude.h"

#include <math.h>


Units::Distance Weather::DensityAltitude::calculateDensityAltitudeDryAirApproximation(Units::Temperature oat, Units::Pressure qnh, Units::Distance geometricAltitude)
{
    return calculateDensityAltitudeInternal(oat, qnh, geometricAltitude, Units::Temperature::fromDegreeCelsius(0), false);
}


Units::Distance Weather::DensityAltitude::calculateDensityAltitude(Units::Temperature oat, Units::Pressure qnh, Units::Distance geometricAltitude, Units::Temperature dewPoint)
{
    return calculateDensityAltitudeInternal(oat, qnh, geometricAltitude, dewPoint, true);
}


Units::Distance Weather::DensityAltitude::calculateDensityAltitudeInternal(Units::Temperature oat, Units::Pressure qnh, Units::Distance geometricAltitude, Units::Temperature dewPoint, bool bWithDewPoint)
{
    const Units::Distance geopotentialAltitude = geometricAltitudeToGeopotentialAltitude(geometricAltitude);
    const double pAir = calculateAirDensity(oat, qnh, geopotentialAltitude, dewPoint, bWithDewPoint).toKgPerCubeMeter(); // Air density

    const double g = 9.80665; // Acceleration due to gravity
    const double M = 0.028964; // Molar mass of dry air
    const double R = 8.31432; // Universal Gas Constant
    const double Gamma = 0.0065; // Environmental Lapse Rate (valid below 11 000 m)
    const double T0 = 288.15; // Sea level Standard Temperature
    const double P0 = 101325; // Sea level Standard Pressure
    
    // density equation formula
    const double h1 = T0 / Gamma;
    const double h2 = R * T0 * pAir;
    const double h3 = M * P0;
    const double h4 = h2 / h3;
    const double h5 = 1 - (pow(h4, ((Gamma * R) / (g * M - Gamma * R))));
    const double h = h1 * h5;
    
    return Units::Distance::fromM(h);
}

Units::Pressure Weather::DensityAltitude::calculateVapourPressure(Units::Temperature dewPoint)
{
    const double eso = 6.1078; // polynomial constant
    const double dewPointInCelsius = dewPoint.toDegreeCelsius();

    // remark: errors in odd coefficients in https://aerotoolbox.com/density-altitude/
    // refer to https://wahiduddin.net/calc/density_altitude.htm instead
    const double c[10] = {
         0.99999683,         
        -0.90826951e-2, 
         0.78736169e-4,    
        -0.61117958e-6,
         0.43884187e-8,     
        -0.29883885e-10,
         0.21874425e-12,    
        -0.17892321e-14,
         0.11112018e-16,    
        -0.30994571e-19
    };
    
    const double p = (c[0] + dewPointInCelsius * (c[1] + dewPointInCelsius * (c[2] + dewPointInCelsius * (c[3] + dewPointInCelsius * (c[4] + dewPointInCelsius * (c[5] + dewPointInCelsius * (c[6] + dewPointInCelsius * (c[7] + dewPointInCelsius * (c[8] + dewPointInCelsius * (c[9]))))))))));
    const double Es = eso / (pow(p, 8)); // Saturation pressure of water vapour, Herman Wobus approximation
    const double Pv = Es; // partial pressure of water vapour
    
    return Units::Pressure::fromHPa(Pv);
}


Units::Pressure Weather::DensityAltitude::calculateAbsolutePressure(Units::Pressure qnh, Units::Distance height)
{
  const double heightInM = height.toM();
  const double k1 = 0.190263; // Constant
  const double k2 = 8.417286e-5; // Constant

  // Absolute pressure formula
  const double Pa1 = pow(qnh.toHPa(), k1);
  const double Pa2 = k2 * heightInM;
  const double Pa3 = (Pa1 - Pa2);
  const double Pa = pow(Pa3, (1 / (k1))); // Absolute pressure
  
  return Units::Pressure::fromHPa(Pa);
}


Units::Density Weather::DensityAltitude::calculateAirDensity(Units::Temperature temperature, Units::Pressure qnh, Units::Distance height, Units::Temperature dewPoint, bool bWithDewPoint)
{
    const double Rd = 287.058; // Gas constant dry air
    const double Rv = 461.495; // Gas constant water vapour
    const double Pv = bWithDewPoint ? calculateVapourPressure(dewPoint).toHPa() : 0; // partial pressure of water vapour
    const double Pa = calculateAbsolutePressure(qnh, height).toHPa(); // Absolute or Station Pressure
    const double Pd = Pa - Pv; // Partial pressure of dry air
    const double temperatureInK = temperature.toDegreeKelvin();

    // air density formula
    const double pAir1 = ((Pd * 100) / (Rd * temperatureInK));
    const double pAir2 = ((Pv * 100) / (Rv * temperatureInK));
    const double pAir = pAir1 + pAir2; // Calculated Density of Air

    return Units::Density::fromKgPerCubeMeter(pAir);
}


Units::Distance Weather::DensityAltitude::geometricAltitudeToGeopotentialAltitude(Units::Distance geometricAltitude)
{
    const double E = 6356.766; // ISA radius of the Earth

    // Geopotential Altitude formula
    const double H = (geometricAltitude.toKM() * E) / (E + geometricAltitude.toKM()); // Geopotential Altitude
    return Units::Distance::fromKM(H);
}
