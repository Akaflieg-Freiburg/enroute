/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include <QJsonArray>

#include "units/Distance.h"
#include "Airspace.h"


GeoMaps::Airspace::Airspace(const QJsonObject &geoJSONObject) {
    // Paranoid safety checks
    if (geoJSONObject["type"] != "Feature") {
        return;
    }

    // Get geometry
    if (!geoJSONObject.contains("geometry")) {
        return;
    }
    auto geometry = geoJSONObject["geometry"].toObject();
    if (geometry["type"] != "Polygon") {
        return;
    }
    if (!geometry.contains("coordinates")) {
        return;
    }
    auto polygonArray = geometry["coordinates"].toArray();
    if (polygonArray.size() != 1) {
        return;
    }
    auto polygonCoordinates = polygonArray[0].toArray();
    foreach (auto coordinate, polygonCoordinates) {
        auto coordinateArray = coordinate.toArray();
        auto geoCoordinate =
                QGeoCoordinate(coordinateArray[1].toDouble(), coordinateArray[0].toDouble());
        _polygon.addCoordinate(geoCoordinate);
    }

    // Get properties
    if (!geoJSONObject.contains("properties")) {
        return;
    }
    auto properties = geoJSONObject["properties"].toObject();
    if (!properties.contains("CAT")) {
        return;
    }
    _CAT = properties["CAT"].toString();

    if (!properties.contains("NAM")) {
        return;
    }
    _name = properties["NAM"].toString();

    if (!properties.contains("TOP")) {
        return;
    }
    _upperBound = properties["TOP"].toString();

    if (!properties.contains("BOT")) {
        return;
    }
    _lowerBound = properties["BOT"].toString();
}


auto GeoMaps::Airspace::estimatedLowerBoundMSL() const -> Units::Distance
{
    double result = 0.0;
    bool ok = false;

    QString AL = _lowerBound.simplified();

    if (AL.startsWith("FL", Qt::CaseInsensitive)) {
        result = AL.remove(0, 2).toDouble(&ok);
        if (ok) {
            return Units::Distance::fromFT(100*result);
        }
        return Units::Distance::fromFT(0.0);
    }

    if (AL.endsWith("msl")) {
        AL.chop(3);
        AL = AL.simplified();
    }
    if (AL.endsWith("agl")) {
        AL.chop(3);
        AL = AL.simplified();
    }
    if (AL.endsWith("ft")) {
        AL.chop(2);
        AL = AL.simplified();
    }

    result = AL.toDouble(&ok);
    if (ok) {
        return Units::Distance::fromFT(result);
    }
    return Units::Distance::fromFT(0.0);
}


QString GeoMaps::Airspace::makeMetric(const QString& standard) const
{
    QStringList list = standard.split(' ', Qt::SkipEmptyParts);
    if (list.isEmpty()) {
        return standard;
    }

    if (list[0] == "FL") {
        if (list.size() < 2) {
            return standard;
        }
        bool ok = false;
        auto feetHeight = 100*list[1].toInt(&ok);
        if (!ok) {
            return standard;
        }
        list[1] =QString("%1 m").arg(qRound(Units::Distance::fromFT(feetHeight).toM()));
        return list.join(' ');
    }

    bool ok = false;
    auto feetHeight = list[0].toInt(&ok);
    if (!ok) {
        return standard;
    }
    list[0] = QString("%1 m").arg(qRound(Units::Distance::fromFT(feetHeight).toM()));
    return list.join(' ');
}


bool GeoMaps::operator==(const GeoMaps::Airspace& A, const GeoMaps::Airspace& B)
{
    return ((A._name == B._name) &&
            (A._CAT == B._CAT) &&
            (A._upperBound == B._upperBound) &&
            (A._lowerBound == B._lowerBound) &&
            (A._polygon == B._polygon) );
}


uint GeoMaps::qHash(const GeoMaps::Airspace& A)
{
    uint result = 0;
    result += qHash(A.name());
    result += qHash(A.CAT());
    result += qHash(A.upperBound());
    result += qHash(A.lowerBound());
    result += qHash(A.polygon());
    return result;
}
