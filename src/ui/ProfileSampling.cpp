/***************************************************************************
 *   Copyright (C) 2023-2025 by Stefan Kebekus                             *
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

#include <QGeoRectangle>

#include "GeoMapProvider.h"
#include "GlobalObject.h"
#include "ui/ProfileSampling.h"

using namespace Qt::Literals::StringLiterals;


namespace {

const QStringList airspaceCategories = {"ATZ", "TMZ", "RMZ", "TIA", "TIZ", "NRA", "DNG", "D", "C", "B", "A", "CTR", "R", "P", "PJE", "SUA"};

// Drawing style of an airspace category, as used for the keys of airspacePolygons()
QString styleKey(const QString& CAT)
{
    if ((CAT == u"A"_s) || (CAT == u"B"_s) || (CAT == u"C"_s) || (CAT == u"D"_s))
    {
        return u"A"_s;
    }
    if ((CAT == u"R"_s) || (CAT == u"P"_s) || (CAT == u"DNG"_s))
    {
        return u"R"_s;
    }
    if ((CAT == u"ATZ"_s) || (CAT == u"RMZ"_s) || (CAT == u"TIA"_s) || (CAT == u"TIZ"_s))
    {
        return u"RMZ"_s;
    }
    if ((CAT == u"PJE"_s) || (CAT == u"SUA"_s))
    {
        return u"PJE"_s;
    }
    if ((CAT == u"CTR"_s) || (CAT == u"NRA"_s) || (CAT == u"TMZ"_s))
    {
        return CAT;
    }
    return {};
}

} // namespace


bool Ui::ProfileSampling::fillTerrain(QList<Sample>& samples)
{
    bool complete = true;
    for (auto& sample : samples)
    {
        sample.terrain = GlobalObject::geoMapProvider()->terrainElevationAMSL(sample.coordinate);
        if (!sample.terrain.isFinite())
        {
            sample.terrain = Units::Distance::fromM(0.0);
            complete = false;
        }
    }
    return complete;
}


QPolygonF Ui::ProfileSampling::terrainPolygon(const QList<Sample>& samples, const AltitudeToY& altToY, double width, double height)
{
    QPolygonF polygon;
    polygon.reserve(samples.size()+2);
    for (const auto& sample : samples)
    {
        polygon << QPointF(sample.x, altToY(sample.terrain));
    }
    polygon << QPointF(width, height+2000) << QPointF(-20, height+2000);
    return polygon;
}


QVariantMap Ui::ProfileSampling::emptyAirspacePolygons()
{
    QVariantMap result;
    const QVector<QPolygonF> empty;
    for (const auto& key : {u"A"_s, u"CTR"_s, u"R"_s, u"RMZ"_s, u"NRA"_s, u"PJE"_s, u"TMZ"_s})
    {
        result[key] = QVariant::fromValue(empty);
    }
    return result;
}


QVariantMap Ui::ProfileSampling::airspacePolygons(const QList<Sample>& samples, const AltitudeToY& altToY, Units::Pressure QNH, Units::Distance geometricAltitude, Units::Distance barometricAltitude)
{
    QMap<QString, QVector<QPolygonF>> polygons;
    if (samples.isEmpty())
    {
        return emptyAirspacePolygons();
    }

    QList<QGeoCoordinate> coordinates;
    coordinates.reserve(samples.size());
    for (const auto& sample : samples)
    {
        coordinates << sample.coordinate;
    }
    QGeoRectangle const viewBBox(coordinates);

    QList<QPointF> upper;
    QList<QPointF> lower;
    const auto airspaces = GlobalObject::geoMapProvider()->airspaces();
    for (const auto& airspace : airspaces)
    {
        if (!airspaceCategories.contains(airspace.CAT()))
        {
            continue;
        }
        if (!airspace.polygon().boundingGeoRectangle().intersects(viewBBox))
        {
            continue;
        }

        auto airspacePolygon = airspace.polygon();
        for (qsizetype i = 0; i < samples.size(); i++)
        {
            const auto& sample = samples[i];

            // The last sample is never inside, so that open polygons are closed
            if ((i != samples.size()-1) && airspacePolygon.contains(sample.coordinate))
            {
                auto u = airspace.estimatedUpperBoundMSL(sample.terrain, QNH, geometricAltitude, barometricAltitude);
                auto l = airspace.estimatedLowerBoundMSL(sample.terrain, QNH, geometricAltitude, barometricAltitude);
                if (l < u)
                {
                    upper << QPointF(sample.x, altToY(u));
                    lower << QPointF(sample.x, altToY(l));
                    continue;
                }
            }

            if (!upper.isEmpty())
            {
                std::reverse(lower.begin(), lower.end());
                QPolygonF polygon(upper + lower);
                polygon << upper[0];
                auto key = styleKey(airspace.CAT());
                if (!key.isEmpty())
                {
                    polygons[key] << polygon;
                }
                upper.clear();
                lower.clear();
            }
        }
    }

    auto result = emptyAirspacePolygons();
    for (auto it = polygons.cbegin(); it != polygons.cend(); ++it)
    {
        result[it.key()] = QVariant::fromValue(it.value());
    }
    return result;
}
