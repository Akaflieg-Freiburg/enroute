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

#include "SimpleWaypoint.h"
#include "units/Distance.h"
#include "weather/Station.h"


GeoMaps::SimpleWaypoint::SimpleWaypoint()
{
    m_properties.insert("CAT", QString("WP"));
    m_properties.insert("NAM", QString("Waypoint"));
    m_properties.insert("TYP", QString("WP"));
}


GeoMaps::SimpleWaypoint::SimpleWaypoint(const QGeoCoordinate& coordinate)
    : m_coordinate(coordinate)
{
    m_properties.insert("CAT", QString("WP"));
    m_properties.insert("NAM", QString("Waypoint"));
    m_properties.insert("TYP", QString("WP"));
}


GeoMaps::SimpleWaypoint::SimpleWaypoint(const QJsonObject &geoJSONObject)
{
    // Paranoid safety checks
    if (geoJSONObject["type"] != "Feature") {
        return;
    }

    // Get properties
    if (!geoJSONObject.contains("properties")) {
        return;
    }
    auto properties = geoJSONObject["properties"].toObject();
    foreach(auto propertyName, properties.keys())
        m_properties.insert(propertyName, properties[propertyName].toVariant());

    // Get geometry
    if (!geoJSONObject.contains("geometry")) {
        return;
    }
    auto geometry = geoJSONObject["geometry"].toObject();
    if (geometry["type"] != "Point") {
        return;
    }
    if (!geometry.contains("coordinates")) {
        return;
    }
    auto coordinateArray = geometry["coordinates"].toArray();
    if (coordinateArray.size() != 2) {
        return;
    }
    m_coordinate = QGeoCoordinate(coordinateArray[1].toDouble(), coordinateArray[0].toDouble() );
    if (m_properties.contains("ELE")) {
        m_coordinate.setAltitude(properties["ELE"].toDouble());
    }
}


//
// METHODS
//

auto GeoMaps::SimpleWaypoint::isNear(const SimpleWaypoint& other) const -> bool
{
    if (!m_coordinate.isValid()) {
        return false;
    }
    if (!other.coordinate().isValid()) {
        return false;
    }

    return m_coordinate.distanceTo(other.m_coordinate) < 2000;
}


auto GeoMaps::SimpleWaypoint::toJSON() const -> QJsonObject
{
    QJsonArray coords;
    coords.insert(0, m_coordinate.longitude());
    coords.insert(1, m_coordinate.latitude());
    QJsonObject geometry;
    geometry.insert("type", "Point");
    geometry.insert("coordinates", coords);
    QJsonObject feature;
    feature.insert("type", "Feature");
    feature.insert("properties", QJsonObject::fromVariantMap(m_properties));
    feature.insert("geometry", geometry);

    return feature;
}


//
// PROPERTIES
//


auto GeoMaps::SimpleWaypoint::extendedName() const -> QString
{
    if (m_properties.value("TYP").toString() == "NAV") {
        return QString("%1 (%2)").arg(m_properties.value("NAM").toString(), m_properties.value("CAT").toString());
    }

    return m_properties.value("NAM").toString();
}


void GeoMaps::SimpleWaypoint::setExtendedName(const QString &newExtendedName)
{
    m_properties.replace("NAM", newExtendedName);
}


auto GeoMaps::SimpleWaypoint::icon() const -> QString
{
    auto CAT = category();

    // We prefer SVG icons. There are, however, a few icons that cannot be
    // rendered by Qt's tinySVG renderer. We have generated PNGs for those
    // and treat them separately here.
    if ((CAT == "AD-GLD") || (CAT == "AD-GRASS") || (CAT == "AD-MIL-GRASS") || (CAT == "AD-UL")) {
        return QStringLiteral("/icons/waypoints/%1.png").arg(CAT);
    }

    return QStringLiteral("/icons/waypoints/%1.svg").arg(CAT);
}


auto GeoMaps::SimpleWaypoint::isValid() const -> bool
{
    if (!m_coordinate.isValid()) {
        return false;
    }
    if (!m_properties.contains("TYP")) {
        return false;
    }
    auto TYP = m_properties.value("TYP").toString();

    // Handle airfields
    if (TYP == "AD") {
        // Property CAT
        if (!m_properties.contains("CAT")) {
            return false;
        }
        auto CAT = m_properties.value("CAT").toString();
        if ((CAT != "AD") && (CAT != "AD-GRASS") && (CAT != "AD-PAVED") &&
                (CAT != "AD-INOP") && (CAT != "AD-GLD") && (CAT != "AD-MIL") &&
                (CAT != "AD-MIL-GRASS") && (CAT != "AD-MIL-PAVED") && (CAT != "AD-UL") &&
                (CAT != "AD-WATER")) {
            return false;
        }

        // Property ELE
        if (!m_properties.contains("ELE")) {
            return false;
        }
        bool ok = false;
        m_properties.value("ELE").toInt(&ok);
        if (!ok) {
            return false;
        }

        // Property NAM
        if (!m_properties.contains("NAM")) {
            return false;
        }
        return true;
    }

    // Handle NavAids
    if (TYP == "NAV") {
        // Property CAT
        if (!m_properties.contains("CAT")) {
            return false;
        }
        auto CAT = m_properties.value("CAT").toString();
        if ((CAT != "NDB") && (CAT != "VOR") && (CAT != "VOR-DME") &&
                (CAT != "VORTAC") && (CAT != "DVOR") && (CAT != "DVOR-DME") &&
                (CAT != "DVORTAC")) {
            return false;
        }

        // Property COD
        if (!m_properties.contains("COD")) {
            return false;
        }

        // Property NAM
        if (!m_properties.contains("NAM")) {
            return false;
        }

        // Property NAV
        if (!m_properties.contains("NAV")) {
            return false;
        }

        // Property MOR
        if (!m_properties.contains("MOR")) {
            return false;
        }

        return true;
    }

    // Handle waypoints
    if (TYP == "WP") {
        // Property CAT
        if (!m_properties.contains("CAT")) {
            return false;
        }
        auto CAT = m_properties.value("CAT").toString();
        if ((CAT != "MRP") && (CAT != "RP") && (CAT != "WP")) {
            return false;
        }

        // Property COD
        if ((CAT == "MRP") || (CAT == "RP")) {
            if (!m_properties.contains("COD")) {
                return false;
            }
        }

        // Property NAM
        if (!m_properties.contains("NAM")) {
            return false;
        }

        // Property SCO
        if ((CAT == "MRP") || (CAT == "RP")) {
            if (!m_properties.contains("SCO")) {
                return false;
            }
        }

        return true;
    }

    // Unknown TYP
    return false;
}


auto GeoMaps::SimpleWaypoint::tabularDescription() const -> QList<QString>
{
    QList<QString> result;

    if (m_properties.value("TYP").toString() == "NAV") {
        result.append("ID  " + m_properties.value("COD").toString() + " " + m_properties.value("MOR").toString());
        result.append("NAV " + m_properties.value("NAV").toString());
        if (m_properties.contains("ELE")) {
            result.append(QString("ELEV%1 ft AMSL").arg(qRound(AviationUnits::Distance::fromM(m_properties.value("ELE").toDouble()).toFeet())));
        }
    }

    if (m_properties.value("TYP").toString() == "AD") {
        if (m_properties.contains("COD")) {
            result.append("ID  " + m_properties.value("COD").toString());
        }
        if (m_properties.contains("INF")) {
            result.append("INF " + m_properties.value("INF").toString().replace("\n", "<br>"));
        }
        if (m_properties.contains("COM")) {
            result.append("COM " + m_properties.value("COM").toString().replace("\n", "<br>"));
        }
        if (m_properties.contains("NAV")) {
            result.append("NAV " + m_properties.value("NAV").toString().replace("\n", "<br>"));
        }
        if (m_properties.contains("OTH")) {
            result.append("OTH " + m_properties.value("OTH").toString().replace("\n", "<br>"));
        }
        if (m_properties.contains("RWY")) {
            result.append("RWY " + m_properties.value("RWY").toString().replace("\n", "<br>"));
        }

        result.append( QString("ELEV%1 ft AMSL").arg(qRound(AviationUnits::Distance::fromM(m_properties.value("ELE").toDouble()).toFeet())));
    }

    if (m_properties.value("TYP").toString() == "WP") {
        if (m_properties.contains("ICA")) {
            result.append("ID  " + m_properties.value("COD").toString());
        }
        if (m_properties.contains("COM")) {
            result.append("COM " + m_properties.value("COM").toString());
        }
    }

    return result;
}


auto GeoMaps::SimpleWaypoint::twoLineTitle() const -> QString
{
    QString codeName;
    if (m_properties.contains("COD")) {
        codeName += m_properties.value("COD").toString();
    }
    if (m_properties.contains("MOR")) {
        codeName += " " + m_properties.value("MOR").toString();
    }

    if (!codeName.isEmpty()) {
        return QString("<strong>%1</strong><br><font size='2'>%2</font>").arg(codeName, extendedName());
    }

    return extendedName();
}
