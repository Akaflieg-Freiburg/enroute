/***************************************************************************
 *   Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org          *
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include <QDateTime>
#include <QQmlEngine>

#include "FlightRoute.h"
#include "geomaps/GPX.h"
#include "geomaps/GeoJSON.h"
#include "geomaps/GeoMapProvider.h"

auto Navigation::FlightRoute::toGpx() const -> QByteArray
{
    // now in UTC, ISO 8601 alike
    //
    QString const now = QDateTime::currentDateTimeUtc().toString(
        QStringLiteral("yyyy-MM-dd HH:mm:ssZ"));

    // gpx header
    //
    QString gpx = QString("<?xml version='1.0' encoding='UTF-8'?>\n"
                          "<gpx version='1.1' creator='Enroute - https://akaflieg-freiburg.github.io/enroute'\n"
                          "     xmlns='http://www.topografix.com/GPX/1/1'\n"
                          "     xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'>\n"
                          "  <metadata>\n"
                          "    <name>Enroute " + now + "</name>\n"
                                                       "    <time>" + now + "</time>\n");

    auto bbox = boundingRectangle();
    if (bbox.isValid()) {
        gpx += "    <bounds"
               " minlat='" + QString::number(bbox.bottomLeft().latitude(), 'f', 8) + "'"
                                                                                     " minlon='" + QString::number(bbox.topLeft().longitude(), 'f', 8) + "'"
                                                                                                                                                         " maxlat='" + QString::number(bbox.topLeft().latitude(), 'f', 8) + "'"
                                                                                                                                                                                                                            " maxlon='" + QString::number(bbox.topRight().longitude(), 'f', 8) + "'/>\n";
    }

    gpx += u"  </metadata>\n"_qs;

    // 2 spaces additional indent
    //
    gpx += gpxElements(QStringLiteral("  "), QStringLiteral("wpt"));

    // start gpx rte
    // rte does _not_ contain segments
    //
    gpx += "  <rte>\n"
           "    <name>Enroute " + now + "</name>\n";

    // 4 spaces additional indent
    //
    gpx += gpxElements(QStringLiteral("    "), QStringLiteral("rtept"));

    // close gpx
    //
    gpx += u"  </rte>\n"_qs;

    // the next few lines export the route as gpx track.
    // We leave this disabled right now. If we discover later
    // that other apps depend on the route beeing exported
    // as track rather than as route we can easily enable
    // it again.
    //
#if 0
    // start gpx trk
    // trk does contains segments <trkseg>
    //
    gpx += "  <trk>\n"
           "    <name>Enroute " + now + "</name>\n"
                                        "    <trkseg>\n";

    // 6 spaces additional indent
    //
    gpx += gpxElements(QString("      "), "trkpt");

    // close gpx
    //
    gpx += "    </trkseg>\n"
           "  </trk>\n";
#endif

    gpx += u"</gpx>\n"_qs;

    return gpx.toUtf8();
}

auto Navigation::FlightRoute::gpxElements(const QString& indent, const QString& tag) const -> QString
{
    QString gpx = u""_qs;

    // waypoints
    //
    for(const auto& _waypoint : m_waypoints) {

        if (!_waypoint.isValid()) {
            continue; // skip silently
        }

        QGeoCoordinate const position = _waypoint.coordinate();
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

auto Navigation::FlightRoute::load(const QString& fileName) -> QString
{
    QString myFileName = fileName;
    if (fileName.startsWith(u"file://"_qs))
    {
        myFileName = fileName.mid(7);
    }


    auto result = GeoMaps::GPX::read(myFileName);
    if (result.isEmpty())
    {
        result = GeoMaps::GeoJSON::read(myFileName);
    }
    if (result.isEmpty())
    {
        return tr("Error reading file '%1'").arg(myFileName);
    }
    if (result.length() > 100)
    {
        return tr("The file '%1' contains too many waypoints. Flight routes with more than 100 waypoints are not supported.").arg(myFileName);
    }

    m_waypoints.clear();
    foreach(auto waypoint, result)
    {
        if (!waypoint.isValid())
        {
            continue;
        }

        auto pos = waypoint.coordinate();
        auto distPos = pos.atDistanceAndAzimuth(1000.0, 0.0, 0.0);
        auto nearest = GlobalObject::geoMapProvider()->closestWaypoint(pos, distPos);
        if (nearest.type() == u"WP")
        {
            m_waypoints << waypoint;
        }
        else
        {
            m_waypoints << nearest;
        }

    }

    updateLegs();
    emit waypointsChanged();
    return {};
}
