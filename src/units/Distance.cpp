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


auto AviationUnits::Distance::toString(bool useMetric, bool vertical, bool forceSign) const -> QString
{
    if (!isFinite()) {
        return QString();
}

    double roundedDist = NAN;
    QString unit;

    if (vertical && useMetric) {
        roundedDist = qRound(toM());
        unit = "m";
    }
    if (vertical && !useMetric) {
        roundedDist = qRound(toFeet());
        unit = "ft";
    }
    if (!vertical && useMetric) {
        if (qAbs(toM()) < 5000) {
            roundedDist = qRound(toM());
            unit = "m";
        } else {
            roundedDist = qRound(toKM()*10.0)/10.0;
            unit = "km";
        }
    }
    if (!vertical && !useMetric) {
        roundedDist = qRound(toNM()*10.0)/10.0;
        unit = "nm";
    }

    // Round value to reasonable numbers
    if (qAbs(roundedDist) > 1000.0) {
        roundedDist = qRound(roundedDist/100.0)*100.0;
    } else if (qAbs(roundedDist) > 100.0) {
        roundedDist = qRound(roundedDist/10.0)*10.0;
}

    QString signString;
    if (forceSign && roundedDist > 0.0) {
        signString += "+";
}
    return signString + QString::number(roundedDist) + " " + unit;
}
