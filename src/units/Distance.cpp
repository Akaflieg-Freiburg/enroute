/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include <cmath>

using namespace Qt::Literals::StringLiterals;


auto Units::Distance::toString(Units::Distance::DistanceUnit units, bool roundBigNumbers, bool forceSign) const -> QString
{
    if (!isFinite())
    {
        return {};
    }

    double roundedDist = NAN;
    QString unit;

    switch (units)
    {
    case Feet:
        roundedDist = qRound(toFeet());
        unit = QStringLiteral("ft");
        break;
    case Meter:
        roundedDist = qRound(toM());
        unit = QStringLiteral("m");
        break;
    case Kilometer:
        roundedDist = qRound(toKM()*10.0)/10.0;
        unit = QStringLiteral("km");
        break;
    case StatuteMile:
        roundedDist = qRound(toMIL()*10.0)/10.0;
        unit = QStringLiteral("mil");
        break;
    case NauticalMile:
        roundedDist = qRound(toNM()*10.0)/10.0;
        unit = QStringLiteral("nm");
        break;
    }

    // Round value to reasonable numbers
    if (roundBigNumbers)
    {
        if (qAbs(roundedDist) > 1000.0)
        {
            roundedDist = qRound(roundedDist/100.0)*100.0;
        }
        else if (qAbs(roundedDist) > 100.0)
        {
            roundedDist = qRound(roundedDist/10.0)*10.0;
        }
    }

    QString signString;
    if (forceSign && roundedDist > 0.0)
    {
        signString += u"+"_s;
    }
    return signString + QString::number(roundedDist) + u" "_s + unit;
}


QDataStream& operator<<(QDataStream& stream, const Units::Distance& distance)
{
    stream << distance.toM();
    return stream;
}


QDataStream& operator>>(QDataStream& stream, Units::Distance& distance)
{
    double tmp = NAN;
    stream >> tmp;
    distance = Units::Distance::fromM(tmp);
    return stream;
}

