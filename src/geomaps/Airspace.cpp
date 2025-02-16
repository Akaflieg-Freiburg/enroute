/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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

#include "Airspace.h"
#include "units/Distance.h"

using namespace Qt::Literals::StringLiterals;


GeoMaps::Airspace::Airspace(const QJsonObject &geoJSONObject) {
    // Paranoid safety checks
    if (geoJSONObject[QStringLiteral("type")] != "Feature") {
        return;
    }

    // Get geometry
    if (!geoJSONObject.contains(QStringLiteral("geometry"))) {
        return;
    }
    auto geometry = geoJSONObject[QStringLiteral("geometry")].toObject();
    if (geometry[QStringLiteral("type")] != "Polygon") {
        return;
    }
    if (!geometry.contains(QStringLiteral("coordinates"))) {
        return;
    }
    auto polygonArray = geometry[QStringLiteral("coordinates")].toArray();
    if (polygonArray.size() != 1) {
        return;
    }
    const auto polygonCoordinates = polygonArray[0].toArray();
    for (const auto coordinate : polygonCoordinates)
    {
        auto coordinateArray = coordinate.toArray();
        auto geoCoordinate = QGeoCoordinate(coordinateArray[1].toDouble(), coordinateArray[0].toDouble());
        m_polygon.addCoordinate(geoCoordinate);
    }

    // Get properties
    if (!geoJSONObject.contains(QStringLiteral("properties"))) {
        return;
    }
    auto properties = geoJSONObject[QStringLiteral("properties")].toObject();
    if (!properties.contains(QStringLiteral("CAT"))) {
        return;
    }
    m_CAT = properties[QStringLiteral("CAT")].toString();

    if (!properties.contains(QStringLiteral("NAM"))) {
        return;
    }
    m_name = properties[QStringLiteral("NAM")].toString();

    if (!properties.contains(QStringLiteral("TOP"))) {
        return;
    }
    m_upperBound = properties[QStringLiteral("TOP")].toString();

    if (!properties.contains(QStringLiteral("BOT"))) {
        return;
    }
    m_lowerBound = properties[QStringLiteral("BOT")].toString();
}


auto GeoMaps::Airspace::estimatedLowerBoundMSL() const -> Units::Distance
{
    double result = 0.0;
    bool ok = false;

    QString AL = m_lowerBound.simplified();

    if (AL.startsWith(u"FL"_s, Qt::CaseInsensitive)) {
        result = AL.remove(0, 2).toDouble(&ok);
        if (ok) {
            return Units::Distance::fromFT(100*result);
        }
        return Units::Distance::fromFT(0.0);
    }

    if (AL.endsWith(u"msl"_s)) {
        AL.chop(3);
        AL = AL.simplified();
    }
    if (AL.endsWith(u"agl"_s)) {
        AL.chop(3);
        AL = AL.simplified();
    }
    if (AL.endsWith(u"ft"_s)) {
        AL.chop(2);
        AL = AL.simplified();
    }

    result = AL.toDouble(&ok);
    if (ok) {
        return Units::Distance::fromFT(result);
    }
    return Units::Distance::fromFT(0.0);
}


auto GeoMaps::Airspace::makeMetric(const QString& standard) -> QString
{
    QStringList list = standard.split(' ', Qt::SkipEmptyParts);
    if (list.isEmpty()) {
        return standard;
    }

    if (list[0] == u"FL") {
        if (list.size() < 2) {
            return standard;
        }
        bool ok = false;
        auto feetHeight = 100*list[1].toInt(&ok);
        if (!ok) {
            return standard;
        }
        list[1] =QStringLiteral("%1 m").arg(qRound(Units::Distance::fromFT(feetHeight).toM()));
        return list.join(' ');
    }

    bool ok = false;
    auto feetHeight = list[0].toInt(&ok);
    if (!ok) {
        return standard;
    }
    list[0] = QStringLiteral("%1 m").arg(qRound(Units::Distance::fromFT(feetHeight).toM()));
    return list.join(' ');
}


auto GeoMaps::operator==(const GeoMaps::Airspace& A, const GeoMaps::Airspace& B) -> bool
{
    return ((A.m_name == B.m_name) &&
            (A.m_CAT == B.m_CAT) &&
            (A.m_upperBound == B.m_upperBound) &&
            (A.m_lowerBound == B.m_lowerBound) &&
            (A.m_polygon == B.m_polygon) );
}


auto GeoMaps::qHash(const GeoMaps::Airspace& A) -> size_t
{
    auto result = qHash(A.name());
    result += qHash(A.CAT());
    result += qHash(A.upperBound());
    result += qHash(A.lowerBound());
    result += qHash(A.polygon());
    return result;
}
