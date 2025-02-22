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

#include <QJsonArray>

#include "GlobalObject.h"
#include "Waypoint.h"
#include "navigation/Aircraft.h"
#include "navigation/Navigator.h"
#include "units/Distance.h"

using namespace Qt::Literals::StringLiterals;


GeoMaps::Waypoint::Waypoint()
{
    m_properties.insert(QStringLiteral("CAT"), QStringLiteral("WP"));
    m_properties.insert(QStringLiteral("NAM"), QStringLiteral("Waypoint"));
    m_properties.insert(QStringLiteral("TYP"), QStringLiteral("WP"));
}


GeoMaps::Waypoint::Waypoint(const QGeoCoordinate& coordinate, const QString& name)
    : m_coordinate(coordinate)
{
    m_properties.insert(QStringLiteral("CAT"), QStringLiteral("WP"));
    if (name.isEmpty())
    {
        m_properties.insert(QStringLiteral("NAM"), QStringLiteral("Waypoint"));
    }
    else
    {
        m_properties.insert(QStringLiteral("NAM"), name);
    }
    m_properties.insert(QStringLiteral("TYP"), QStringLiteral("WP"));
    if (coordinate.type() == QGeoCoordinate::Coordinate3D) {
        m_properties.insert(QStringLiteral("ELE"), coordinate.altitude() );
    }
}


GeoMaps::Waypoint::Waypoint(const QJsonObject &geoJSONObject)
{
    // Paranoid safety checks
    if (geoJSONObject[QStringLiteral("type")] != "Feature") {
        return;
    }

    // Get properties
    QJsonObject properties;
    if (geoJSONObject.contains(QStringLiteral("properties")))
    {
        properties = geoJSONObject[QStringLiteral("properties")].toObject();
        foreach(auto propertyName, properties.keys())
            m_properties.insert(propertyName, properties[propertyName].toVariant());
    }

    // Get geometry
    if (!geoJSONObject.contains(QStringLiteral("geometry"))) {
        return;
    }
    auto geometry = geoJSONObject[QStringLiteral("geometry")].toObject();
    if (geometry[QStringLiteral("type")] != "Point") {
        return;
    }
    if (!geometry.contains(QStringLiteral("coordinates"))) {
        return;
    }
    auto coordinateArray = geometry[QStringLiteral("coordinates")].toArray();
    if (coordinateArray.size() != 2) {
        return;
    }

    m_coordinate = QGeoCoordinate(coordinateArray[1].toDouble(), coordinateArray[0].toDouble() );
    if (m_properties.contains(QStringLiteral("ELE"))) {
        m_coordinate.setAltitude(properties[QStringLiteral("ELE")].toDouble());
    }

    // If the file was not created by Enroute, then the property array will not conform to the specification here: https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation
    // We create a fake entry instead.
    if (!m_properties.contains(QStringLiteral("TYP")))
    {
        m_properties[u"TYP"_s] = "WP";
        m_properties[u"CAT"_s] = "WP";
        m_properties[u"NAM"_s] = "";

        if (m_properties.contains(u"name"_s))
        {
            m_properties[u"NAM"_s] = m_properties[u"name"_s];
        }
        if (m_properties[u"NAM"_s] == "")
        {
            m_properties[u"NAM"_s] = "Waypoint";
        }
    }
}


//
// GETTER METHODS
//

auto GeoMaps::Waypoint::isValid() const -> bool
{
    if (!m_coordinate.isValid()) {
        return false;
    }
    if (!m_properties.contains(QStringLiteral("TYP"))) {
        return false;
    }
    auto TYP = m_properties.value(QStringLiteral("TYP")).toString();

    // Handle airfields
    if (TYP == u"AD") {
        // Property CAT
        if (!m_properties.contains(QStringLiteral("CAT"))) {
            return false;
        }
        auto CAT = m_properties.value(QStringLiteral("CAT")).toString();
        if ((CAT != u"AD") && (CAT != u"AD-GRASS") && (CAT != u"AD-PAVED") &&
                (CAT != u"AD-INOP") && (CAT != u"AD-GLD") && (CAT != u"AD-MIL") &&
                (CAT != u"AD-MIL-GRASS") && (CAT != u"AD-MIL-PAVED") && (CAT != u"AD-UL") &&
                (CAT != u"AD-WATER")) {
            return false;
        }

        // Property ELE
        if (!m_properties.contains(QStringLiteral("ELE"))) {
            return false;
        }
        bool ok = false;
        m_properties.value(QStringLiteral("ELE")).toInt(&ok);
        if (!ok) {
            return false;
        }

        // Property NAM
        if (!m_properties.contains(QStringLiteral("NAM"))) {
            return false;
        }
        return true;
    }

    // Handle NavAids
    if (TYP == u"NAV") {
        // Property CAT
        if (!m_properties.contains(QStringLiteral("CAT"))) {
            return false;
        }
        auto CAT = m_properties.value(QStringLiteral("CAT")).toString();
        if ((CAT != u"DME") && (CAT != u"NDB") && (CAT != u"VOR") && (CAT != u"VOR-DME") &&
            (CAT != u"VORTAC") && (CAT != u"DVOR") && (CAT != u"DVOR-DME") &&
            (CAT != u"DVORTAC")) {
            return false;
        }

        // Property COD
        if (!m_properties.contains(QStringLiteral("COD"))) {
            return false;
        }

        // Property NAM
        if (!m_properties.contains(QStringLiteral("NAM"))) {
            return false;
        }

        // Property NAV
        if (!m_properties.contains(QStringLiteral("NAV"))) {
            return false;
        }

        // Property MOR
        if (!m_properties.contains(QStringLiteral("MOR"))) {
            return false;
        }

        return true;
    }

    // Handle waypoints
    if (TYP == u"WP") {
        // Property CAT
        if (!m_properties.contains(QStringLiteral("CAT"))) {
            return false;
        }
        auto CAT = m_properties.value(QStringLiteral("CAT")).toString();
        if ((CAT != u"MRP") && (CAT != u"RP") && (CAT != u"WP")) {
            return false;
        }

        // Property COD
        if ((CAT == u"MRP") || (CAT == u"RP")) {
            if (!m_properties.contains(QStringLiteral("COD"))) {
                return false;
            }
        }

        // Property NAM
        if (!m_properties.contains(QStringLiteral("NAM"))) {
            return false;
        }

        // Property SCO
        if ((CAT == u"MRP") || (CAT == u"RP")) {
            if (!m_properties.contains(QStringLiteral("SCO"))) {
                return false;
            }
        }

        return true;
    }

    // Unknown TYP
    return false;
}


//
// METHODS
//

auto GeoMaps::Waypoint::isNear(const Waypoint& other) const -> bool
{
    if (!m_coordinate.isValid()) {
        return false;
    }
    if (!other.coordinate().isValid()) {
        return false;
    }

    return m_coordinate.distanceTo(other.m_coordinate) < 2000;
}


auto GeoMaps::Waypoint::toJSON() const -> QJsonObject
{
    QJsonArray coords;
    coords.insert(0, m_coordinate.longitude());
    coords.insert(1, m_coordinate.latitude());
    QJsonObject geometry;
    geometry.insert(QStringLiteral("type"), "Point");
    geometry.insert(QStringLiteral("coordinates"), coords);
    QJsonObject feature;
    feature.insert(QStringLiteral("type"), "Feature");
    feature.insert(QStringLiteral("properties"), QJsonObject::fromVariantMap(m_properties));
    feature.insert(QStringLiteral("geometry"), geometry);

    return feature;
}


void GeoMaps::Waypoint::toGPX(QXmlStreamWriter& stream) const
{
    if (!isValid())
    {
        return;
    }

    auto lat = QString::number(m_coordinate.latitude(), 'f', 8);
    auto lon = QString::number(m_coordinate.longitude(), 'f', 8);

    stream.writeStartElement(QStringLiteral("wpt"));
    stream.writeAttribute(QStringLiteral("lat"), lat);
    stream.writeAttribute(QStringLiteral("lon"), lon);
    if (m_coordinate.type() == QGeoCoordinate::Coordinate3D) {
        auto elevation = QString::number(m_coordinate.altitude(), 'f', 2);
        stream.writeTextElement(QStringLiteral("ele"), elevation);
    }
    stream.writeTextElement(QStringLiteral("name"), extendedName());
    stream.writeEndElement(); // wpt
}


//
// PROPERTIES
//


auto GeoMaps::Waypoint::extendedName() const -> QString
{
    if (m_properties.value(QStringLiteral("TYP")).toString() == u"NAV") {
        return QStringLiteral("%1 (%2)").arg(m_properties.value(QStringLiteral("NAM")).toString(), m_properties.value(QStringLiteral("CAT")).toString());
    }

    return m_properties.value(QStringLiteral("NAM")).toString();
}


auto GeoMaps::Waypoint::icon() const -> QString
{
    auto CAT = category();

    // We prefer SVG icons. There are, however, a few icons that cannot be
    // rendered by Qt's tinySVG renderer. We have generated PNGs for those
    // and treat them separately here.
    if ((CAT == u"AD-GLD") || (CAT == u"AD-GRASS") || (CAT == u"AD-MIL-GRASS") || (CAT == u"AD-UL")) {
        return QStringLiteral("/icons/waypoints/%1.png").arg(CAT);
    }

    return QStringLiteral("/icons/waypoints/%1.svg").arg(CAT);
}


auto GeoMaps::Waypoint::tabularDescription() const -> QList<QString>
{
    QList<QString> result;

    if (m_properties.value(QStringLiteral("TYP")).toString() == u"NAV")
    {
        result.append("ID  " + m_properties.value(QStringLiteral("COD")).toString() + " " + m_properties.value(QStringLiteral("MOR")).toString());
        result.append("NAV " + m_properties.value(QStringLiteral("NAV")).toString());
    }

    if (m_properties.value(QStringLiteral("TYP")).toString() == u"AD")
    {
        if (m_properties.contains(QStringLiteral("COD"))) {
            result.append("ID  " + m_properties.value(QStringLiteral("COD")).toString());
        }
        if (m_properties.contains(QStringLiteral("INF"))) {
            result.append("INF " + m_properties.value(QStringLiteral("INF")).toString().replace(u"\n"_s, u"<br>"_s));
        }
        if (m_properties.contains(QStringLiteral("COM"))) {
            result.append("COM " + m_properties.value(QStringLiteral("COM")).toString().replace(u"\n"_s, u"<br>"_s));
        }
        if (m_properties.contains(QStringLiteral("NAV"))) {
            result.append("NAV " + m_properties.value(QStringLiteral("NAV")).toString().replace(u"\n"_s, u"<br>"_s));
        }
        if (m_properties.contains(QStringLiteral("OTH"))) {
            result.append("OTH " + m_properties.value(QStringLiteral("OTH")).toString().replace(u"\n"_s, u"<br>"_s));
        }
        if (m_properties.contains(QStringLiteral("RWY"))) {
            result.append("RWY " + m_properties.value(QStringLiteral("RWY")).toString().replace(u"\n"_s, u"<br>"_s));
        }
    }

    if (m_properties.value(QStringLiteral("TYP")).toString() == u"WP") {
        if (m_properties.contains(QStringLiteral("ICA"))) {
            result.append("ID  " + m_properties.value(QStringLiteral("COD")).toString());
        }
        if (m_properties.contains(QStringLiteral("COM"))) {
            result.append("COM " + m_properties.value(QStringLiteral("COM")).toString());
        }
    }

    if (m_properties.contains(QStringLiteral("ELE"))) {
        auto ele = Units::Distance::fromM( m_properties.value(QStringLiteral("ELE")).toDouble() );
        auto eleString = GlobalObject::navigator()->aircraft().verticalDistanceToString(ele);
        result.append(QStringLiteral("ELEV%1 AMSL").arg(eleString));
    }

    if (m_properties.contains(QStringLiteral("NOT"))) {
        result.append("NOTE" + m_properties.value(QStringLiteral("NOT")).toString());
    }

    return result;
}


auto GeoMaps::Waypoint::twoLineTitle() const -> QString
{
    QString codeName;
    if (m_properties.contains(QStringLiteral("COD"))) {
        codeName += m_properties.value(QStringLiteral("COD")).toString();
    }
    if (m_properties.contains(QStringLiteral("MOR"))) {
        codeName += " " + m_properties.value(QStringLiteral("MOR")).toString();
    }

    if (!codeName.isEmpty()) {
        return QStringLiteral("<strong>%1</strong><br><font size='2'>%2</font>").arg(codeName, extendedName());
    }

    return extendedName();
}


size_t GeoMaps::qHash(const GeoMaps::Waypoint& waypoint)
{
    auto result = qHash(waypoint.m_coordinate);

    QMapIterator<QString, QVariant> i(waypoint.m_properties);
    while (i.hasNext()) {
        i.next();
        result += qHash(i.key())+1;
        result += qHash(i.value().toString());
    }
    return result;
}
