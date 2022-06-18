/***************************************************************************
 *   Copyright (C) 2019, 2021 by Stefan Kebekus                            *
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
#include <QUrl>

#include "Waypoint.h"
#include "units/Distance.h"


GeoMaps::Waypoint::Waypoint()
{
    m_properties.insert(QStringLiteral("CAT"), QStringLiteral("WP"));
    m_properties.insert(QStringLiteral("NAM"), QStringLiteral("Waypoint"));
    m_properties.insert(QStringLiteral("TYP"), QStringLiteral("WP"));

    // Set cached property
    m_isValid = computeIsValid();
}


GeoMaps::Waypoint::Waypoint(const QGeoCoordinate& coordinate)
    : m_coordinate(coordinate)
{
    m_properties.insert(QStringLiteral("CAT"), QStringLiteral("WP"));
    m_properties.insert(QStringLiteral("NAM"), QStringLiteral("Waypoint"));
    m_properties.insert(QStringLiteral("TYP"), QStringLiteral("WP"));

    // Set cached property
    m_isValid = computeIsValid();
}


GeoMaps::Waypoint::Waypoint(const QJsonObject &geoJSONObject)
{
    // Paranoid safety checks
    if (geoJSONObject[QStringLiteral("type")] != "Feature") {
        return;
    }

    // Get properties
    if (!geoJSONObject.contains(QStringLiteral("properties"))) {
        return;
    }
    auto properties = geoJSONObject[QStringLiteral("properties")].toObject();
    foreach(auto propertyName, properties.keys())
        m_properties.insert(propertyName, properties[propertyName].toVariant());

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

    // Set cached property
    m_isValid = computeIsValid();
}


//
// METHODS
//

auto GeoMaps::Waypoint::computeIsValid() const -> bool
{
    if (!m_coordinate.isValid()) {
        return false;
    }
    if (!m_properties.contains(QStringLiteral("TYP"))) {
        return false;
    }
    auto TYP = m_properties.value(QStringLiteral("TYP")).toString();

    // Handle airfields
    if (TYP == QLatin1String("AD")) {
        // Property CAT
        if (!m_properties.contains(QStringLiteral("CAT"))) {
            return false;
        }
        auto CAT = m_properties.value(QStringLiteral("CAT")).toString();
        if ((CAT != QLatin1String("AD")) && (CAT != QLatin1String("AD-GRASS")) && (CAT != QLatin1String("AD-PAVED")) &&
                (CAT != QLatin1String("AD-INOP")) && (CAT != QLatin1String("AD-GLD")) && (CAT != QLatin1String("AD-MIL")) &&
                (CAT != QLatin1String("AD-MIL-GRASS")) && (CAT != QLatin1String("AD-MIL-PAVED")) && (CAT != QLatin1String("AD-UL")) &&
                (CAT != QLatin1String("AD-WATER"))) {
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
    if (TYP == QLatin1String("NAV")) {
        // Property CAT
        if (!m_properties.contains(QStringLiteral("CAT"))) {
            return false;
        }
        auto CAT = m_properties.value(QStringLiteral("CAT")).toString();
        if ((CAT != QLatin1String("NDB")) && (CAT != QLatin1String("VOR")) && (CAT != QLatin1String("VOR-DME")) &&
                (CAT != QLatin1String("VORTAC")) && (CAT != QLatin1String("DVOR")) && (CAT != QLatin1String("DVOR-DME")) &&
                (CAT != QLatin1String("DVORTAC"))) {
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
    if (TYP == QLatin1String("WP")) {
        // Property CAT
        if (!m_properties.contains(QStringLiteral("CAT"))) {
            return false;
        }
        auto CAT = m_properties.value(QStringLiteral("CAT")).toString();
        if ((CAT != QLatin1String("MRP")) && (CAT != QLatin1String("RP")) && (CAT != QLatin1String("WP"))) {
            return false;
        }

        // Property COD
        if ((CAT == QLatin1String("MRP")) || (CAT == QLatin1String("RP"))) {
            if (!m_properties.contains(QStringLiteral("COD"))) {
                return false;
            }
        }

        // Property NAM
        if (!m_properties.contains(QStringLiteral("NAM"))) {
            return false;
        }

        // Property SCO
        if ((CAT == QLatin1String("MRP")) || (CAT == QLatin1String("RP"))) {
            if (!m_properties.contains(QStringLiteral("SCO"))) {
                return false;
            }
        }

        return true;
    }

    // Unknown TYP
    return false;
}


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


auto GeoMaps::Waypoint::relocated(const QGeoCoordinate& newCoordinate) const -> GeoMaps::Waypoint
{
    Waypoint copy(*this);
    copy.m_coordinate = newCoordinate;
    return copy;
}


auto GeoMaps::Waypoint::renamed(const QString &newName) const -> GeoMaps::Waypoint
{
    Waypoint copy(*this);
    copy.m_properties.replace(QStringLiteral("NAM"), newName);
    return copy;
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
    if (m_properties.value(QStringLiteral("TYP")).toString() == QLatin1String("NAV")) {
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
    if ((CAT == QLatin1String("AD-GLD")) || (CAT == QLatin1String("AD-GRASS")) || (CAT == QLatin1String("AD-MIL-GRASS")) || (CAT == QLatin1String("AD-UL"))) {
        return QStringLiteral("/icons/waypoints/%1.png").arg(CAT);
    }

    return QStringLiteral("/icons/waypoints/%1.svg").arg(CAT);
}


auto GeoMaps::Waypoint::tabularDescription() const -> QList<QString>
{
    QList<QString> result;

    if (m_properties.value(QStringLiteral("TYP")).toString() == QLatin1String("NAV")) {
        result.append("ID  " + m_properties.value(QStringLiteral("COD")).toString() + " " + m_properties.value(QStringLiteral("MOR")).toString());
        result.append("NAV " + m_properties.value(QStringLiteral("NAV")).toString());
        if (m_properties.contains(QStringLiteral("ELE"))) {
            result.append(QStringLiteral("ELEV%1 ft AMSL").arg(qRound(Units::Distance::fromM(m_properties.value(QStringLiteral("ELE")).toDouble()).toFeet())));
        }
    }

    if (m_properties.value(QStringLiteral("TYP")).toString() == QLatin1String("AD")) {
        if (m_properties.contains(QStringLiteral("COD"))) {
            result.append("ID  " + m_properties.value(QStringLiteral("COD")).toString());
        }
        if (m_properties.contains(QStringLiteral("INF"))) {
            result.append("INF " + m_properties.value(QStringLiteral("INF")).toString().replace(QLatin1String("\n"), QLatin1String("<br>")));
        }
        if (m_properties.contains(QStringLiteral("COM"))) {
            result.append("COM " + m_properties.value(QStringLiteral("COM")).toString().replace(QLatin1String("\n"), QLatin1String("<br>")));
        }
        if (m_properties.contains(QStringLiteral("NAV"))) {
            result.append("NAV " + m_properties.value(QStringLiteral("NAV")).toString().replace(QLatin1String("\n"), QLatin1String("<br>")));
        }
        if (m_properties.contains(QStringLiteral("OTH"))) {
            result.append("OTH " + m_properties.value(QStringLiteral("OTH")).toString().replace(QLatin1String("\n"), QLatin1String("<br>")));
        }
        if (m_properties.contains(QStringLiteral("RWY"))) {
            result.append("RWY " + m_properties.value(QStringLiteral("RWY")).toString().replace(QLatin1String("\n"), QLatin1String("<br>")));
        }

        result.append( QStringLiteral("ELEV%1 ft AMSL").arg(qRound(Units::Distance::fromM(m_properties.value(QStringLiteral("ELE")).toDouble()).toFeet())));
    }

    if (m_properties.value(QStringLiteral("TYP")).toString() == QLatin1String("WP")) {
        if (m_properties.contains(QStringLiteral("ICA"))) {
            result.append("ID  " + m_properties.value(QStringLiteral("COD")).toString());
        }
        if (m_properties.contains(QStringLiteral("COM"))) {
            result.append("COM " + m_properties.value(QStringLiteral("COM")).toString());
        }
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


auto GeoMaps::operator==(const GeoMaps::Waypoint& A, const GeoMaps::Waypoint& B) -> bool
{
    return ((A.m_coordinate == B.m_coordinate) &&
            (A.m_properties == B.m_properties));
}


auto GeoMaps::operator!=(const GeoMaps::Waypoint& A, const GeoMaps::Waypoint& B) -> bool
{
    return ((A.m_coordinate != B.m_coordinate) ||
            (A.m_properties != B.m_properties));
}

auto GeoMaps::qHash(const GeoMaps::Waypoint& wp) -> uint
{
    return qHash(wp.coordinate());
}
