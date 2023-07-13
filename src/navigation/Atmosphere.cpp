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

#include "navigation/Atmosphere.h"

double Lb = 0.0065; // Temperature gradient: degree Kelvin / kilometer
double Tb = 288.15; // Temperature at 0m height: degree Kelvin
double g0 = 9.80665; // Gravity: meter/second²
double Rstar = 8.3144598; // universal gas constant: J/(mol·K)
double P0 = 101325; // Pressure at sea evel: Pascal
double M = 0.0289644; // molar mass of Earth's air: kg/mol


Units::Distance Navigation::Atmosphere::height(Units::Pressure p)
{
    double exponent = 1.0/5.255; // (Rstar*Lb)/(g0*M);

    double height_in_meter = (Tb/Lb)*(1.0-pow(p.toPa()/P0, exponent));

    qWarning() << "exponent" << exponent;
    qWarning() << "pressure in " << p.toPa();
    qWarning() << "pressurecheck" << pressure(Units::Distance::fromM(height_in_meter)).toPa();

    return Units::Distance::fromM(height_in_meter);
}


Units::Pressure Navigation::Atmosphere::pressure(Units::Distance height)
{
    double exponent = (g0*M)/(Rstar*Lb);

    double pressure_in_pascal = P0*pow( (Tb-height.toM()*Lb)/Tb, exponent);

    return Units::Pressure::fromPa(pressure_in_pascal);
}
