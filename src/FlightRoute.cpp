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

#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QQmlEngine>
#include <QStandardPaths>

#include "AviationUnits.h"
#include "FlightRoute.h"
#include "Waypoint.h"

#warning need to go through code and make it safe for aircraft or wind to be nullptr

FlightRoute::FlightRoute(Aircraft *aircraft, Wind *wind, QObject *parent)
    : QObject(parent), _aircraft(aircraft), _wind(wind)
{
    stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flight route.geojson";

    // Load last flightRoute
    load(stdFileName);

    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::saveToStdLocation);
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
    for(const auto& _waypoint : _waypoints) {
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


QString FlightRoute::save(const QString& fileName) const
{
    QFile file(fileName);
    auto success = file.open(QIODevice::WriteOnly);
    if (!success)
        return tr("Unable to open the file '%1' for writing.").arg(fileName);
    auto numBytesWritten = file.write(toGeoJSON());
    if (numBytesWritten == -1) {
        file.close();
        QFile::remove(fileName);
        return tr("Unable to write to file '%1' for writing.").arg(fileName);
    }
    file.close();
    return QString();
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


QString FlightRoute::suggestedFilename() const
{
    if (_waypoints.size() < 2)
        return QString();

    QString start = _waypoints.constFirst()->get("COD").toString();
    if (start.isEmpty())
        return QString();
    QString end = _waypoints.constLast()->get("COD").toString();
    if (end.isEmpty())
        return QString();

    return start+" - "+end;
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


QByteArray FlightRoute::toGeoJSON() const
{
    QJsonArray waypointArray;
    foreach(auto waypoint, _waypoints)
        waypointArray.append(waypoint->toJSON());
    QJsonObject jsonObj;
    jsonObj.insert("type", "FeatureCollection");
    jsonObj.insert("features", waypointArray);
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}


QString FlightRoute::load(QString fileName)
{
    if (fileName.isEmpty())
        fileName = stdFileName;

    QFile file(fileName);
    auto success = file.open(QIODevice::ReadOnly);
    if (!success)
        return tr("Cannot open file '%1' for reading.").arg(fileName);
    auto fileContent = file.readAll();
    if (fileContent.isEmpty())
        return tr("Cannot read data from file '%1'.").arg(fileName);
    file.close();

    QJsonParseError parseError{};
    auto document = QJsonDocument::fromJson(fileContent, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return tr("Cannot parse file '%1'. Reason: %2.").arg(fileName, parseError.errorString());

    QList<QPointer<Waypoint>> newWaypoints;
    foreach(auto value, document.object()["features"].toArray()) {
        auto wp = new Waypoint(value.toObject());
        if (!wp->isValid()) {
            qDeleteAll(newWaypoints);
            return tr("Cannot parse content of file '%1'.").arg(fileName);
        }
        QQmlEngine::setObjectOwnership(wp, QQmlEngine::CppOwnership);
        newWaypoints.append(wp);
    }

    qDeleteAll(_waypoints);
    _waypoints = newWaypoints;
    updateLegs();
    emit waypointsChanged();

    return QString();
}


// export to gpx waypoints (wpt) and route (rte).
// We currently don't export as track, but see below.
//
QByteArray FlightRoute::toGpx()
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

    return gpx.toUtf8();
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


QString FlightRoute::gpxElements(QString indent, QString tag)
{
    QString gpx = "";

    // waypoints
    //
    for(auto _waypoint : _waypoints) {

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
