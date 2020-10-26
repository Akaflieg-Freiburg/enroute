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

QString AviationUnits::Angle::toString() const {
    double angleInDegrees = toDEG();

    angleInDegrees = qAbs(angleInDegrees);
    int deg = static_cast<int>(qFloor(angleInDegrees));
    angleInDegrees = (angleInDegrees - qFloor(angleInDegrees)) * 60.0;
    int min = static_cast<int>(qFloor(angleInDegrees));
    angleInDegrees = (angleInDegrees - qFloor(angleInDegrees)) * 60.0;

    return QString("%1° %2' %3\"").arg(deg).arg(min).arg(angleInDegrees, 0, 'f', 2);
}

double AviationUnits::Angle::toNormalizedDEG() const {
    double angle = toDEG();
    if (!std::isfinite(angle))
        return qQNaN();

    double a = angle / 360.0;
    return 360.0 * (a - qFloor(a));
}

QGeoCoordinate AviationUnits::stringToCoordinate(const QString &geoLat, const QString &geoLong) {
    // Interpret coordinates.
    auto lat = geoLat.chopped(1).toDouble();
    if (geoLat.right(1) == "S")
        lat *= -1.0;

    auto lon = geoLong.chopped(1).toDouble();
    if (geoLat.right(1) == "W")
        lon *= -1.0;

    return QGeoCoordinate(lat, lon);
}

QString AviationUnits::Speed::toString() const {
    // Find out that unit system we should use
    bool useMetric = false;
    auto globalSettings = GlobalSettings::globalInstance();
    if (globalSettings)
        useMetric = globalSettings->useMetricUnits();

    if (useMetric)
        return QString("%1 km/h").arg( qRound(toKMH()) );
    return QString("%1 kt").arg( qRound(toKT()) );
}

QString AviationUnits::Time::toHoursAndMinutes() const {
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
