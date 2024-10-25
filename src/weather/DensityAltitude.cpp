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
#include "units/Distance.h"
#include "units/Temperature.h"
#include <math.h>

double Weather::DensityAltitude::calculateDensityAltitudeDryAirApproximation(double oat, double qnh, double geometricAltitudeInFt)
{
    return calculateDensityAltitudeInternal(oat, qnh, geometricAltitudeInFt, 0, false);
}

double Weather::DensityAltitude::calculateDensityAltitude(double oat, double qnh, double geometricAltitudeInFt, double dewPoint)
{
    return calculateDensityAltitudeInternal(oat, qnh, geometricAltitudeInFt, dewPoint, true);
}


double Weather::DensityAltitude::calculateDensityAltitudeInternal(double oat, double qnh, double geometricAltitudeInFt, double dewPoint, bool bWithDewPoint)
{
    const double geometricAltitudeInKm = Units::Distance::fromFT(geometricAltitudeInFt).toKM();
    const double geopotentialAltitude = geometricAltitudeToGeopotentialAltitude(geometricAltitudeInKm);
    const double pAir = calculateAirDensity(oat, qnh, geopotentialAltitude, dewPoint, bWithDewPoint); // Air density

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
    
    return Units::Distance::fromM(h).toFeet();
}

double Weather::DensityAltitude::calculateVapourPressure(double dewPoint)
{
    const double eso = 6.1078; // polynomial constant

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
    
    const double p = (c[0] + dewPoint * (c[1] + dewPoint * (c[2] + dewPoint * (c[3] + dewPoint * (c[4] + dewPoint * (c[5] + dewPoint * (c[6] + dewPoint * (c[7] + dewPoint * (c[8] + dewPoint * (c[9]))))))))));
    const double Es = eso / (pow(p, 8)); // Saturation pressure of water vapour, Herman Wobus approximation
    const double Pv = Es; // partial pressure of water vapour
    
    return Pv;
}

double Weather::DensityAltitude::calculateAbsolutePressure(double qnh, double height)
{
  const double heightInM = Units::Distance::fromKM(height).toM();
  const double k1 = 0.190263; // Constant
  const double k2 = 8.417286e-5; // Constant

  // Absolute pressure formula
  const double Pa1 = pow(qnh, k1);
  const double Pa2 = k2 * heightInM;
  const double Pa3 = (Pa1 - Pa2);
  const double Pa = pow(Pa3, (1 / (k1))); // Absolute pressure
  
  return Pa;
}

double Weather::DensityAltitude::calculateAirDensity(double temperature, double qnh, double height, double dewPoint, bool bWithDewPoint)
{
    const double Rd = 287.058; // Gas constant dry air
    const double Rv = 461.495; // Gas constant water vapour
    const double Pv = bWithDewPoint ? calculateVapourPressure(dewPoint) : 0; // partial pressure of water vapour
    const double Pa = calculateAbsolutePressure(qnh, height); // Absolute or Station Pressure
    const double Pd = Pa - Pv; // Partial pressure of dry air
    const double temperatureInK = Units::Temperature::fromDegreeCelsius(temperature).toDegreeKelvin();

    // air density formula
    const double pAir1 = ((Pd * 100) / (Rd * temperatureInK));
    const double pAir2 = ((Pv * 100) / (Rv * temperatureInK));
    const double pAir = pAir1 + pAir2; // Calculated Density of Air
    return pAir;
}

double Weather::DensityAltitude::geometricAltitudeToGeopotentialAltitude(double geometricAltitudeInKm)
{
    const double E = 6356.766; // ISA radius of the Earth

    // Geopotential Altitude formula
    const double H = (geometricAltitudeInKm * E) / (E + geometricAltitudeInKm); // Geopotential Altitude
    return H;
}
