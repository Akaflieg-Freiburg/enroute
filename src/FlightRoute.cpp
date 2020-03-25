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

#include <QDataStream>
#include <QFile>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

#include "AviationUnits.h"
#include "FlightRoute.h"
#include "Waypoint.h"


FlightRoute::FlightRoute(Aircraft *aircraft, Wind *wind, GeoMapProvider *geoMapProvider_in, QObject *parent)
    : QObject(parent), _aircraft(aircraft), _wind(wind), geoMapProvider(geoMapProvider_in)
{
    load();
    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::save);
    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::summaryChanged);
    if (!_aircraft.isNull())
        connect(_aircraft, &Aircraft::valChanged, this, &FlightRoute::summaryChanged);
    if (!_wind.isNull())
        connect(_wind, &Wind::valChanged, this, &FlightRoute::summaryChanged);
}


void FlightRoute::append(QObject *waypoint)
{
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;

    auto* wp = dynamic_cast<Waypoint*>(waypoint);
    _waypoints.append(new Waypoint(*wp, this));

    updateLegs();
    emit waypointsChanged();
}


QObject* FlightRoute::firstWaypointObject() const
{
    if (_waypoints.isEmpty())
        return nullptr;
    return _waypoints.first();
}


QVariantList FlightRoute::geoPath() const
{
    // Paranoid safety checks
    if (_waypoints.size() < 2)
        return QVariantList();

    QVariantList result;
    for(auto _waypoint : _waypoints) {
        if (!_waypoint->isValid())
            return QVariantList();
        result.append(QVariant::fromValue(_waypoint->coordinate()));
    }

    return result;
}


QObject* FlightRoute::lastWaypointObject() const
{
    if (_waypoints.isEmpty())
        return nullptr;
    return _waypoints.last();
}


void FlightRoute::clear()
{
    qDeleteAll(_waypoints);
    _waypoints.clear();

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::moveDown(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;
    if (waypoint == lastWaypointObject())
        return;

    auto swp = dynamic_cast<Waypoint*>(waypoint);

    auto idx = _waypoints.indexOf(swp);
    _waypoints.move(idx, idx+1);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::moveUp(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;
    if (waypoint == firstWaypointObject())
        return;

    auto swp = dynamic_cast<Waypoint*>(waypoint);

    auto idx = _waypoints.indexOf(swp);
    if (idx > 0)
        _waypoints.move(idx, idx-1);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::removeWaypoint(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;

    auto swp = dynamic_cast<Waypoint*>(waypoint);

    _waypoints.removeOne(swp);
    delete swp;

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::reverse()
{
    std::reverse(_waypoints.begin(), _waypoints.end());
    updateLegs();
    emit waypointsChanged();
}


QString FlightRoute::gpxElements(QString indent, QString tag)
{
    QString gpx = "";

    // waypoints
    //
    for(auto _waypoint : _waypoints) {

        if (!_waypoint->isValid())
            continue; // skip silently

        QGeoCoordinate position = _waypoint->coordinate();
        auto code = _waypoint->codeName();
        auto name = _waypoint->extendedName();

        if (code.isEmpty()) {
            code = name;
        }

        auto lat = QString::number(position.latitude(), 'f', 8);
        auto lon = QString::number(position.longitude(), 'f', 8);
        gpx += indent + "<" + tag + " lat='" + lat + "' lon='" + lon + "'>\n";

        if (_waypoint->hasElevation()) {

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


bool FlightRoute::routeBounds(double &minlat, double &minlon, double &maxlat, double &maxlon) const
{
    bool valid = false;

    for(auto _waypoint : _waypoints) {

        if (!_waypoint->isValid())
            continue; // skip silently

        QGeoCoordinate position = _waypoint->coordinate();
        auto lat = position.latitude();
        auto lon = position.longitude();

        if (!valid)
        {
            valid = true;
            minlat = maxlat = lat;
            minlon = maxlon = lon;
        } else {
            if (lat < minlat)
                minlat = lat;
            if (lon < minlon)
                minlon = lon;
            if (lat > maxlat)
                maxlat = lat;
            if (lon > maxlon)
                maxlon = lon;
        }
    }

    return valid;
}

// export to gpx waypoints (wpt) and route (rte).
// We currently don't export as track, but see below.
//
QString FlightRoute::toGpx()
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

    double minlat, minlon, maxlat, maxlon;
    if (routeBounds(minlat, minlon, maxlat, maxlon))
    {
        gpx += "    <bounds"
                " minlat='" + QString::number(minlat, 'f', 8) + "'"
                " minlon='" + QString::number(minlon, 'f', 8) + "'"
                " maxlat='" + QString::number(maxlat, 'f', 8) + "'"
                " maxlon='" + QString::number(maxlon, 'f', 8) + "'/>\n";
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

    return gpx;
}

void FlightRoute::fromGpx(QString fileUrl)
{
    if (fileUrl.startsWith("file://"))
    {
        fileUrl = QUrl(fileUrl).toLocalFile();
    }

    QFile file(fileUrl);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << tr(QString("File open error:" + file.errorString()).toUtf8());
        return;
    }

    QXmlStreamReader xml(&file);
    fromGpx(xml);
}

void FlightRoute::fromGpx(const QByteArray& data)
{
    QXmlStreamReader xml(data);
    fromGpx(xml);
}

void FlightRoute::fromGpx(QXmlStreamReader& xml)
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
            if (!xml.readNext()) {
                break;
            }

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

        // check if there's a known waypoint like for example an airfield nearby.
        // If we find a waypoint within the distance of 1/100° we use it instead
        // of the coordinate which was just imported from gpx.
        //
        QGeoCoordinate distant_pos(lat + 0.01 /* about 1.11 km */, lon);
        QObject* nearest = geoMapProvider->closestWaypoint(pos, distant_pos);

        if (nearest != nullptr) {
            Waypoint* wpt = dynamic_cast<Waypoint*>(nearest);

            if (wpt->get("TYP") == "WP" && wpt->get("CAT") == "WP" && name.length() > 0) {
                wpt->setName(name);
            }
        }

        target.append(nearest == nullptr ? new Waypoint(pos, this) : new Waypoint(*dynamic_cast<Waypoint*>(nearest), this));

        return;
    }; // <<< lambda function to read a single gpx rtept, trkpt or wpt

    while (!xml.atEnd() && !xml.hasError())
    {
        auto token = xml.readNext();
        if (!token) {
            qDebug() << "can't read next element";
            break;
        }

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
        QString err = tr(QString("no valid route found").toUtf8());
        qDebug() << err;
        return;
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
}


void FlightRoute::save()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flightPlan.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);

    out << streamVersion; // Stream version
    for(auto & _waypoint : _waypoints)
        out << *_waypoint;
}


void FlightRoute::load()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flightPlan.dat");
    if (!file.exists())
        return;

    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    quint16 version;
    in >> version;
    if (version != streamVersion)
        return;

    _waypoints.clear();
    while(!in.atEnd()) {
        auto wp = new Waypoint(in, this);
        if (!wp->isValid()) {
            delete wp;
            _waypoints.clear();
            return;
        }
        _waypoints.append(wp);
    }

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::updateLegs()
{
    foreach(auto _leg, _legs)
        _leg->deleteLater();
    _legs.clear();

    for(int i=0; i<_waypoints.size()-1; i++)
        _legs.append(new Leg(_waypoints[i], _waypoints[i+1], _aircraft, _wind, this));
}


QList<QObject*> FlightRoute::routeObjects() const
{
    QList<QObject*> result;

    if (_waypoints.isEmpty())
        return result;

    for(int i=0; i<_waypoints.size()-1; i++) {
        result.append(_waypoints[i]);
        result.append(_legs[i]);
    }
    result.append(_waypoints.last());

    return result;
}


QString FlightRoute::summary() const
{
    if (_legs.empty())
        return {};

    QString result;

    auto dist = AviationUnits::Distance::fromM(0.0);
    auto time = AviationUnits::Time::fromS(0.0);
    double fuelInL = 0.0;

    for(auto _leg : _legs) {
        dist += _leg->distance();
        if (dist.toM() > 100) {
            time += _leg->Time();
            fuelInL += _leg->Fuel();
        }
    }

    result += QString("Total: %1&nbsp;NM").arg(dist.toNM(), 0, 'f', 1);
    if (time.isFinite())
        result += QString(" • %1&nbsp;h").arg(time.toHoursAndMinutes());
    if (qIsFinite(fuelInL))
        result += QString(" • %1&nbsp;L").arg(qRound(fuelInL));


    QStringList complaints;
    if (!_aircraft.isNull()) {
        if (!qIsFinite(_aircraft->cruiseSpeedInKT()))
            complaints += tr("Cruise speed not specified.");
        if (!qIsFinite(_aircraft->fuelConsumptionInLPH()))
            complaints += tr("Fuel consumption not specified.");
    }
    if (!_wind.isNull()) {
        if (!qIsFinite(_wind->windSpeedInKT()))
            complaints += tr("Wind speed not specified.");
        if (!qIsFinite(_wind->windDirectionInDEG()))
            if (!qIsFinite(_wind->windDirectionInDEG()))
                complaints += tr("Wind direction not specified.");

    }

    if (!complaints.isEmpty())
        result += QString("<p><font color='red'>Computation incomplete. %1</font></p>").arg(complaints.join(" "));

    return result;
}
