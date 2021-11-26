/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include "units/Distance.h"


QString Units::Distance::toString(Units::Distance::DistanceUnit units, bool roundBigNumbers, bool forceSign) const
{
    if (!isFinite()) {
        return {};
    }

    double roundedDist = NAN;
    QString unit;

    switch (units) {
    case Feet:
        roundedDist = qRound(toFeet());
        unit = "ft";
        break;
    case Meter:
        roundedDist = qRound(toM());
        unit = "m";
        break;
    case Kilometer:
        roundedDist = qRound(toKM()*10.0)/10.0;
        unit = "km";
        break;
    case StatuteMile:
        roundedDist = qRound(toMIL()*10.0)/10.0;
        unit = "mil";
        break;
    case NauticalMile:
        roundedDist = qRound(toNM()*10.0)/10.0;
        unit = "nm";
        break;
    }

    // Round value to reasonable numbers
    if (roundBigNumbers) {
        if (qAbs(roundedDist) > 1000.0) {
            roundedDist = qRound(roundedDist/100.0)*100.0;
        } else if (qAbs(roundedDist) > 100.0) {
            roundedDist = qRound(roundedDist/10.0)*10.0;
        }
    }

    QString signString;
    if (forceSign && roundedDist > 0.0) {
        signString += "+";
    }
    return signString + QString::number(roundedDist) + " " + unit;
}
