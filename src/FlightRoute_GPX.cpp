/***************************************************************************
 *   Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org          *
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

#include "FlightRoute.h"
#include "GeoMapProvider.h"


QByteArray FlightRoute::toGpx() const
{
    // now in UTC, ISO 8601 alike
    //
    QString now = QDateTime::currentDateTimeUtc().toString("yyyy-MM-dd HH:mm:ssZ");

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

    gpx += "  </metadata>\n";

    // 2 spaces additional indent
    //
    gpx += gpxElements(QString("  "), "wpt");

    // start gpx rte
    // rte does _not_ contain segments
    //
    gpx += "  <rte>\n"
           "    <name>Enroute " + now + "</name>\n";

    // 4 spaces additional indent
    //
    gpx += gpxElements(QString("    "), "rtept");

    // close gpx
    //
    gpx += "  </rte>\n";

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

    gpx += "</gpx>\n";

    return gpx.toUtf8();
}


QString FlightRoute::gpxElements(const QString& indent, const QString& tag) const
{
    QString gpx = "";

    // waypoints
    //
    for(const auto& _waypoint : _waypoints) {

        if (!_waypoint->isValid())
            continue; // skip silently

        QGeoCoordinate position = _waypoint->coordinate();
        auto code = _waypoint->get("COD").toString();
        auto name = _waypoint->extendedName();

        if (code.isEmpty()) {
            code = name;
        }

        auto lat = QString::number(position.latitude(), 'f', 8);
        auto lon = QString::number(position.longitude(), 'f', 8);
        gpx += indent + "<" + tag + " lat='" + lat + "' lon='" + lon + "'>\n";

        if (_waypoint->contains("ELE")) {

            // elevation in meters always for gpx
            //
            auto elevation = QString::number(_waypoint->get("ELE").toDouble(), 'f', 2);
            gpx += indent + "  <ele>" + elevation + "</ele>\n";
        }

        gpx += indent + "  <name>" + code + "</name>\n" +
                indent + "  <cmt>" + name + "</cmt>\n" +
                indent + "  <desc>" + name + "</desc>\n" +
                indent + "</" + tag + ">\n";
    }

    return gpx;
}


QString FlightRoute::loadFromGpx(QString fileName, GeoMapProvider *geoMapProvider)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return tr("Error opening file '%1'").arg(fileName);

    QXmlStreamReader xml(&file);
    return loadFromGpx(xml, geoMapProvider);
}


QString FlightRoute::loadFromGpx(const QByteArray& data, GeoMapProvider *geoMapProvider)
{
    QXmlStreamReader xml(data);
    return loadFromGpx(xml, geoMapProvider);
}


QString FlightRoute::loadFromGpx(QXmlStreamReader& xml, GeoMapProvider *geoMapProvider)
{

    // collect all route points and track points and waypoints
    //
    QList<Waypoint*> rtept;
    QList<Waypoint*> trkpt;
    QList<Waypoint*> wpt;

    // lambda function to read a single gpx rtept, trkpt or wpt
    //
    auto addPoint = [&] (const QString tag, QList<Waypoint*> &target) {

        // capture rtept, trkpt or wpt

        QXmlStreamAttributes attrs = xml.attributes();
        if (!attrs.hasAttribute("lon") || !attrs.hasAttribute("lat")) {
            qDebug() << "missing lat or lon attribute";
            return;
        }

        bool ok;
        double lon = xml.attributes().value("lon").toFloat(&ok);
        if (!ok) {
            qDebug() << "Unable to convert lon to float: " << xml.attributes().value("lon");
            return;
        }

        double lat = xml.attributes().value("lat").toFloat(&ok);
        if (!ok) {
            qDebug() << "Unable to convert lat to float: " << xml.attributes().value("lat");
            return;
        }

        QGeoCoordinate pos(lat, lon);

        QString name;
        QString desc;
        QString cmt;
        while (!xml.atEnd() && !xml.hasError())
        {
            if (!xml.readNext())
                break;

            QString xmlTag = xml.name().toString();

            if (xml.isStartElement()) {

                if (xmlTag == "ele") {
                    QString alt_s = xml.readElementText(QXmlStreamReader::SkipChildElements);
                    double alt = alt_s.toFloat(&ok);
                    if (!ok) {
                        qDebug() << "can't convert elevation to double: " << alt_s;
                        return;
                    }
                    pos.setAltitude(alt);
                } else if (xmlTag == "name") {
                    name = xml.readElementText(QXmlStreamReader::SkipChildElements);
                } else if (xmlTag == "desc") {
                    desc = xml.readElementText(QXmlStreamReader::SkipChildElements);
                } else if (xmlTag == "cmt") {
                    cmt = xml.readElementText(QXmlStreamReader::SkipChildElements);
                }

            }  else if (xml.isEndElement() and xmlTag == tag) {
                break;
            }
        }

        if (name.length() == 0) {
            if (desc.length() != 0) {
                name = desc;
            } else if (cmt.length() != 0) {
                name = cmt;
            }
        }

        // If a GeoMapProvider is available, check if there's a known waypoint like for example an airfield nearby.
        // If we find a waypoint within the distance of 1/100° we use it instead
        // of the coordinate which was just imported from gpx.
        QObject* nearest = nullptr;
        if (geoMapProvider != nullptr) {
            QGeoCoordinate distant_pos(lat + 0.01 /* about 1.11 km */, lon);
            nearest = geoMapProvider->closestWaypoint(pos, distant_pos);

            if (nearest != nullptr) {
                Waypoint* wpt = dynamic_cast<Waypoint*>(nearest);
                if (wpt->get("TYP") == "WP" && wpt->get("CAT") == "WP" && name.length() > 0)
                    wpt->setProperty("NAM", name);
            }
        }
        target.append(nearest == nullptr ? new Waypoint(pos, this) : new Waypoint(*dynamic_cast<Waypoint*>(nearest), this));

        return;
    }; // <<< lambda function to read a single gpx rtept, trkpt or wpt

    while (!xml.atEnd() && !xml.hasError())
    {
        auto token = xml.readNext();
        if (!token)
            break;

        auto name = xml.name().toString();

        if (xml.isStartElement()) {
            if (name == "rtept") {
                addPoint("rtept", rtept);
            } else if (name == "trkpt") {
                addPoint("trkpt", trkpt);
            } else if (name == "wpt") {
                addPoint("wpt", wpt);
            }
        }
    }

    // prefer rte over trk over wpt
    // this is a bit arbitrary but seems reasonable to me.
    // Could be made configurable.
    //
    QList<Waypoint*> &source = (rtept.length() > 0) ? rtept :
                               (trkpt.length() > 0) ? trkpt :
                                                      wpt;
    if (source.length() == 0) {
        // don't have to delete lists rtept, trkpt, wpt as they're empty
        return tr("Error interpreting GPX file: no valid route found.");
    }

    _waypoints.clear();

    // make a deep copy
    //
    for(auto & wp : source) {
        _waypoints.append(new Waypoint(*wp, this));
    }

    qDeleteAll(rtept);
    qDeleteAll(trkpt);
    qDeleteAll(wpt);

    updateLegs();
    emit waypointsChanged();
    return QString();
}
