/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include <QtMath>

#include "AviationUnits.h"
#include "GlobalSettings.h"

auto AviationUnits::Angle::toString() const -> QString {
    double angleInDegrees = toDEG();

    angleInDegrees = qAbs(angleInDegrees);
    int deg = static_cast<int>(qFloor(angleInDegrees));
    angleInDegrees = (angleInDegrees - qFloor(angleInDegrees)) * 60.0;
    int min = static_cast<int>(qFloor(angleInDegrees));
    angleInDegrees = (angleInDegrees - qFloor(angleInDegrees)) * 60.0;

    return QString("%1° %2' %3\"").arg(deg).arg(min).arg(angleInDegrees, 0, 'f', 2);
}


auto AviationUnits::Angle::toNormalizedDEG() const -> double {
    double angle = toDEG();
    if (!std::isfinite(angle))
        return qQNaN();

    double a = angle / 360.0;
    return 360.0 * (a - qFloor(a));
}


auto AviationUnits::stringToCoordinate(const QString &geoLat, const QString &geoLong) -> QGeoCoordinate {
    // Interpret coordinates.
    auto lat = geoLat.chopped(1).toDouble();
    if (geoLat.right(1) == "S")
        lat *= -1.0;

    auto lon = geoLong.chopped(1).toDouble();
    if (geoLat.right(1) == "W")
        lon *= -1.0;

    return QGeoCoordinate(lat, lon);
}


QString AviationUnits::Distance::toString(bool useMetric, bool vertical, bool forceSign) const
{
    if (!isFinite())
        return QString();

    double roundedDist;
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
    if (qAbs(roundedDist) > 1000.0)
        roundedDist = qRound(roundedDist/100.0)*100.0;
    else if (qAbs(roundedDist) > 100.0)
        roundedDist = qRound(roundedDist/10.0)*10.0;

    QString signString;
    if (forceSign && roundedDist > 0.0)
        signString += "+";
    return signString + QString::number(roundedDist) + " " + unit;
}


auto AviationUnits::Speed::toString() const -> QString {
    if (GlobalSettings::useMetricUnitsStatic())
        return QString("%1 km/h").arg( qRound(toKMH()) );
    return QString("%1 kt").arg( qRound(toKT()) );
}


auto operator<<(QDataStream &out, const AviationUnits::Speed &speed) -> QDataStream &
{
    out << speed.toMPS();
    return out;
}


auto operator>>(QDataStream &in, AviationUnits::Speed &speed) -> QDataStream &
{
    double buffer;
    in >> buffer;
    speed = AviationUnits::Speed::fromMPS(buffer);
    return in;
}


auto AviationUnits::Time::toHoursAndMinutes() const -> QString {
    // Paranoid safety checks
    if (!isFinite())
        return "-:--";

    auto minutes = qRound(qAbs(toM()));
    auto hours = minutes / 60;
    minutes = minutes % 60;

    QString result;
    if (isNegative())
        result += "-";
    result +=
        QString("%1:%2").arg(hours, 1, 10, QLatin1Char('0')).arg(minutes, 2, 10, QLatin1Char('0'));
    return result;
}
