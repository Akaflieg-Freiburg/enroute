/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>

#include "Librarian.h"
#include "geomaps/WaypointLibrary.h"

GeoMaps::WaypointLibrary::WaypointLibrary(QObject *parent)
    : GlobalObject(parent)
{
    (void)loadFromGeoJSON();
    connect(this, &GeoMaps::WaypointLibrary::waypointsChanged, this, [this]()
            { (void)save(); });
}

//
// Methods
//

void GeoMaps::WaypointLibrary::add(const GeoMaps::Waypoint &waypoint)
{
    if (!waypoint.isValid())
    {
        return;
    }

    m_waypoints.append(waypoint);
    std::sort(m_waypoints.begin(), m_waypoints.end(), [](const Waypoint &a, const Waypoint &b)
              { return a.name() < b.name(); });
    emit waypointsChanged();
}

void GeoMaps::WaypointLibrary::clear()
{
    if (m_waypoints.isEmpty())
    {
        return;
    }

    m_waypoints.clear();
    emit waypointsChanged();
}

QVector<GeoMaps::Waypoint> GeoMaps::WaypointLibrary::filteredWaypoints(const QString &filter) const
{
    QVector<GeoMaps::Waypoint> result;

    QString simplifiedFilter = GlobalObject::librarian()->simplifySpecialChars(filter);
    foreach (auto waypoint, m_waypoints)
    {
        auto simplifiedName = GlobalObject::librarian()->simplifySpecialChars(waypoint.name());
        if (simplifiedName.contains(simplifiedFilter, Qt::CaseInsensitive))
        {
            result.append(waypoint);
        }
    }
    return result;
}

bool GeoMaps::WaypointLibrary::hasNearbyEntry(const GeoMaps::Waypoint &waypoint) const
{
    for (const auto &wp : qAsConst(m_waypoints))
    {
        if (wp.isNear(waypoint))
        {
            return true;
        }
    }
    return false;
}

auto GeoMaps::WaypointLibrary::loadFromGeoJSON(QString fileName) -> QString
{
    if (fileName.isEmpty())
    {
        fileName = stdFileName;
    }

    QFile file(fileName);
    auto success = file.open(QIODevice::ReadOnly);
    if (!success)
    {
        return tr("Cannot open file '%1' for reading.").arg(fileName);
    }
    auto fileContent = file.readAll();
    if (fileContent.isEmpty())
    {
        return tr("Cannot read data from file '%1'.").arg(fileName);
    }
    file.close();

    QJsonParseError parseError{};
    auto document = QJsonDocument::fromJson(fileContent, &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        return tr("Cannot parse file '%1'. Reason: %2.").arg(fileName, parseError.errorString());
    }

    QVector<GeoMaps::Waypoint> newWaypoints;
    foreach (auto value, document.object()[QStringLiteral("features")].toArray())
    {
        auto wp = GeoMaps::Waypoint(value.toObject());
        if (!wp.isValid())
        {
            return tr("Cannot parse content of file '%1'.").arg(fileName);
        }
        newWaypoints.append(wp);
    }

    m_waypoints = newWaypoints;
    emit waypointsChanged();

    return {};
}

bool GeoMaps::WaypointLibrary::remove(const GeoMaps::Waypoint &waypoint)
{
    if (m_waypoints.removeOne(waypoint))
    {
        emit waypointsChanged();
        return true;
    }
    return false;
}

bool GeoMaps::WaypointLibrary::replace(const GeoMaps::Waypoint &oldWaypoint, const GeoMaps::Waypoint &newWaypoint)
{
    if (!newWaypoint.isValid())
    {
        return false;
    }

    if (m_waypoints.removeOne(oldWaypoint))
    {
        m_waypoints.append(newWaypoint);
        std::sort(m_waypoints.begin(), m_waypoints.end(), [](const Waypoint &a, const Waypoint &b)
                  { return a.name() < b.name(); });
        emit waypointsChanged();
        return true;
    }
    return false;
}

auto GeoMaps::WaypointLibrary::save(QString fileName) const -> QString
{
    if (fileName.isEmpty())
    {
        fileName = stdFileName;
    }

    QFile file(fileName);
    auto success = file.open(QIODevice::WriteOnly);
    if (!success)
    {
        return tr("Unable to open the file '%1' for writing.").arg(fileName);
    }
    auto numBytesWritten = file.write(toGeoJSON());
    if (numBytesWritten == -1)
    {
        file.close();
        QFile::remove(fileName);
        return tr("Unable to write to file '%1'.").arg(fileName);
    }
    file.close();
    return {};
}

auto GeoMaps::WaypointLibrary::toGeoJSON() const -> QByteArray
{
    QJsonArray waypointArray;
    foreach (auto waypoint, m_waypoints)
    {
        if (waypoint.isValid())
        {
            waypointArray.append(waypoint.toJSON());
        }
    }
    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("type"), "FeatureCollection");
    jsonObj.insert(QStringLiteral("enroute"), QStringLiteral("waypoint library"));
    jsonObj.insert(QStringLiteral("features"), waypointArray);
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}

#warning documentation

auto GeoMaps::WaypointLibrary::toGpx() const -> QByteArray
{
    // now in UTC, ISO 8601 alike
    //
    QString now = QDateTime::currentDateTimeUtc().toString(QStringLiteral("yyyy-MM-dd HH:mm:ssZ"));

    // gpx header
    //
    QString gpx = QString("<?xml version='1.0' encoding='UTF-8'?>\n"
                          "<gpx version='1.1' creator='Enroute - https://akaflieg-freiburg.github.io/enroute'\n"
                          "     xmlns='http://www.topografix.com/GPX/1/1'\n"
                          "     xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>\n"
                          "  <metadata>\n"
                          "    <name>Enroute " + now + "</name>\n"
                          "    <time>" + now + "</time>\n"
                          "  </metadata>\n");

    // 2 spaces additional indent
    //
    // waypoints
    //
    QString tag = "wpt";
    QString indent = "  ";
    for(const auto& _waypoint : m_waypoints) {

        if (!_waypoint.isValid()) {
            continue; // skip silently
        }

        QGeoCoordinate position = _waypoint.coordinate();
        auto code = _waypoint.ICAOCode();
        auto name = _waypoint.extendedName();

        if (code.isEmpty()) {
            code = name;
        }

        auto lat = QString::number(position.latitude(), 'f', 8);
        auto lon = QString::number(position.longitude(), 'f', 8);
        gpx += indent + "<" + tag + " lat='" + lat + "' lon='" + lon + "'>\n";

        if (_waypoint.coordinate().type() == QGeoCoordinate::Coordinate3D) {

            // elevation in meters always for gpx
            //
            auto elevation = QString::number(_waypoint.coordinate().altitude(), 'f', 2);
            gpx += indent + "  <ele>" + elevation + "</ele>\n";
        }

        gpx += indent + "  <name>" + code + "</name>\n" +
                indent + "  <cmt>" + name + "</cmt>\n" +
                indent + "  <desc>" + name + "</desc>\n" +
                indent + "</" + tag + ">\n";
    }

//    gpx += gpxElements(QStringLiteral("  "), QStringLiteral("wpt"));

    gpx += QLatin1String("</gpx>\n");

    return gpx.toUtf8();
}

/*
auto Navigation::FlightRoute::gpxElements(const QString& indent, const QString& tag) const -> QString
{
    QString gpx = QLatin1String("");

    // waypoints
    //
    for(const auto& _waypoint : m_waypoints) {

        if (!_waypoint.isValid()) {
            continue; // skip silently
        }

        QGeoCoordinate position = _waypoint.coordinate();
        auto code = _waypoint.ICAOCode();
        auto name = _waypoint.extendedName();

        if (code.isEmpty()) {
            code = name;
        }

        auto lat = QString::number(position.latitude(), 'f', 8);
        auto lon = QString::number(position.longitude(), 'f', 8);
        gpx += indent + "<" + tag + " lat='" + lat + "' lon='" + lon + "'>\n";

        if (_waypoint.coordinate().type() == QGeoCoordinate::Coordinate3D) {

            // elevation in meters always for gpx
            //
            auto elevation = QString::number(_waypoint.coordinate().altitude(), 'f', 2);
            gpx += indent + "  <ele>" + elevation + "</ele>\n";
        }

        gpx += indent + "  <name>" + code + "</name>\n" +
                indent + "  <cmt>" + name + "</cmt>\n" +
                indent + "  <desc>" + name + "</desc>\n" +
                indent + "</" + tag + ">\n";
    }

    return gpx;
}
*/
