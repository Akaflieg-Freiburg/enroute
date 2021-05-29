/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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
#include <QQmlEngine>
#include <QStandardPaths>

#include "FlightRoute.h"

// Static instance of this class. Do not analyze, because of many unwanted warnings.
#ifndef __clang_analyzer__
QPointer<FlightRoute> flightRouteStatic {};
#endif

FlightRoute::FlightRoute(QObject *parent)
    : QObject(parent)
{

    stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flight route.geojson";

    // Load last flightRoute
    loadFromGeoJSON(stdFileName);

    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::saveToStdLocation);
    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::summaryChanged);
    connect(Aircraft::globalInstance(), &Aircraft::valChanged, this, &FlightRoute::summaryChanged);
    connect(Weather::Wind::globalInstance(), &Weather::Wind::valChanged, this, &FlightRoute::summaryChanged);

}


void FlightRoute::append(const GeoMaps::SimpleWaypoint &waypoint)
{
    _waypoints.append(waypoint);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::append(const QGeoCoordinate& position)
{
    append( GeoMaps::SimpleWaypoint(position) );
}


auto FlightRoute::boundingRectangle() const -> QGeoRectangle
{
    QGeoRectangle bbox;

    for(const auto &_waypoint : _waypoints) {
        if (!_waypoint.isValid()) {
            continue;
        }

        QGeoCoordinate position = _waypoint.coordinate();
        if (!bbox.isValid()) {
            bbox.setTopLeft(position);
            bbox.setBottomRight(position);
        } else {
            bbox.extendRectangle(position);
        }
    }

    return bbox;
}


auto FlightRoute::canAppend(const GeoMaps::SimpleWaypoint &other) const -> bool
{
    if (_waypoints.isEmpty() ) {
        return true;
    }

    return !_waypoints.last().isNear(other);
}


void FlightRoute::clear()
{
    _waypoints.clear();

    updateLegs();
    emit waypointsChanged();
}


auto FlightRoute::contains(const GeoMaps::SimpleWaypoint& waypoint) const -> bool
{
    foreach(auto _waypoint, _waypoints) {
        if (!_waypoint.isValid()) {
            continue;
        }
        if (_waypoint.isNear(waypoint)) {
            return true;
        }
    }
    return false;
}


auto FlightRoute::firstWaypointObject() const -> GeoMaps::SimpleWaypoint
{
    if (_waypoints.isEmpty()) {
        return {};
    }
    return _waypoints.first();
}


auto FlightRoute::geoPath() const -> QVariantList
{
    // Paranoid safety checks
    if (_waypoints.size() < 2) {
        return QVariantList();
    }

    QVariantList result;
    for(const auto& _waypoint : _waypoints) {
        if (!_waypoint.isValid()) {
            return QVariantList();
        }
        result.append(QVariant::fromValue(_waypoint.coordinate()));
    }

    return result;
}


auto FlightRoute::globalInstance() -> FlightRoute*
{

#ifndef __clang_analyzer__
    if (flightRouteStatic.isNull()) {
        flightRouteStatic = new FlightRoute();
    }
    return flightRouteStatic;
#else
    return nullptr;
#endif

}


auto FlightRoute::lastWaypointObject() const -> GeoMaps::SimpleWaypoint
{
    if (_waypoints.isEmpty()) {
        return {};
    }
    return _waypoints.last();
}


auto FlightRoute::loadFromGeoJSON(QString fileName) -> QString
{
    if (fileName.isEmpty()) {
        fileName = stdFileName;
    }

    QFile file(fileName);
    auto success = file.open(QIODevice::ReadOnly);
    if (!success) {
        return tr("Cannot open file '%1' for reading.").arg(fileName);
    }
    auto fileContent = file.readAll();
    if (fileContent.isEmpty()) {
        return tr("Cannot read data from file '%1'.").arg(fileName);
    }
    file.close();

    QJsonParseError parseError{};
    auto document = QJsonDocument::fromJson(fileContent, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return tr("Cannot parse file '%1'. Reason: %2.").arg(fileName, parseError.errorString());
    }

    QVector<GeoMaps::SimpleWaypoint> newWaypoints;
    foreach(auto value, document.object()["features"].toArray()) {
        auto wp = GeoMaps::SimpleWaypoint(value.toObject());
        if (!wp.isValid()) {
            return tr("Cannot parse content of file '%1'.").arg(fileName);
        }
#warning
        // connect(wp, &GeoMaps::Waypoint::extendedNameChanged, this, &FlightRoute::waypointsChanged);
        newWaypoints.append(wp);
    }

    _waypoints = newWaypoints;
    updateLegs();
    emit waypointsChanged();

    return QString();
}


auto FlightRoute::makeSummary(bool inMetricUnits) const -> QString
{
    if (_legs.empty()) {
        return {};
    }

    QString result;

    auto dist = AviationUnits::Distance::fromM(0.0);
    auto time = AviationUnits::Time::fromS(0.0);
    double fuelInL = 0.0;

    for(auto *_leg : _legs) {
        dist += _leg->distance();
        if (dist.toM() > 100) {
            time += _leg->Time();
            fuelInL += _leg->Fuel();
        }
    }

    if (inMetricUnits) {
        result += tr("Total: %1&nbsp;km").arg(dist.toKM(), 0, 'f', 1);
    } else {
        result += tr("Total: %1&nbsp;nm").arg(dist.toNM(), 0, 'f', 1);
    }
    if (time.isFinite()) {
        result += QStringLiteral(" • %1&nbsp;h").arg(time.toHoursAndMinutes());
    }
    if (qIsFinite(fuelInL)) {
        result += QStringLiteral(" • %1&nbsp;L").arg(qRound(fuelInL));
    }


    QStringList complaints;
    if (!qIsFinite(Aircraft::globalInstance()->cruiseSpeedInKT())) {
        complaints += tr("Cruise speed not specified.");
    }
    if (!qIsFinite(Aircraft::globalInstance()->fuelConsumptionInLPH())) {
        complaints += tr("Fuel consumption not specified.");
    }
    if (!qIsFinite(Weather::Wind::globalInstance()->windSpeedInKT())) {
        complaints += tr("Wind speed not specified.");
    }
    if (!qIsFinite(Weather::Wind::globalInstance()->windDirectionInDEG())) {
        if (!qIsFinite(Weather::Wind::globalInstance()->windDirectionInDEG())) {
            complaints += tr("Wind direction not specified.");
        }
    }

    if (!complaints.isEmpty()) {
        result += tr("<p><font color='red'>Computation incomplete. %1</font></p>").arg(complaints.join(QStringLiteral(" ")));
    }

    return result;
}


auto FlightRoute::midFieldWaypoints() const -> QVariantList
{
    QVariantList result;

    if (_waypoints.isEmpty()) {
        return result;
    }

    foreach(auto wpt, _waypoints) {
        if (wpt.category() == "WP") {
            result << QVariant::fromValue(wpt);
        }
    }

    return result;
}


void FlightRoute::moveDown(const GeoMaps::SimpleWaypoint& waypoint)
{
    // Paranoid safety checks
    if (waypoint == lastWaypointObject()) {
        return;
    }

    auto idx = _waypoints.indexOf(waypoint);
    _waypoints.move(idx, idx+1);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::moveUp(const GeoMaps::SimpleWaypoint& waypoint)
{
    // Paranoid safety checks
    if (waypoint == firstWaypointObject()) {
        return;
    }

    auto idx = _waypoints.indexOf(waypoint);
    if (idx > 0) {
        _waypoints.move(idx, idx-1);
    }

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::removeWaypoint(const GeoMaps::SimpleWaypoint& waypoint)
{

    foreach(const auto &_waypoint, _waypoints) {
        if (!_waypoint.isValid()) {
            continue;
        }

        if (_waypoint.isNear(waypoint)) {
            _waypoints.removeOne(_waypoint);
            updateLegs();
            emit waypointsChanged();
            return;
        }
    }

}


void FlightRoute::renameWaypoint(const GeoMaps::SimpleWaypoint& waypoint, const QString& newName)
{
    for(auto& wp : _waypoints) {
        if (wp == waypoint) {
            wp.setExtendedName(newName);
        }
    }
    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::reverse()
{
    std::reverse(_waypoints.begin(), _waypoints.end());
    updateLegs();
    emit waypointsChanged();
}


auto FlightRoute::legs() const -> QList<QObject*>
{
    QList<QObject*> result;
    result.reserve(_legs.size());

    for(int i=0; i<_legs.size(); i++) {
        result.append(_legs[i]);
    }

    return result;
}


auto FlightRoute::save(const QString& fileName) const -> QString
{
    QFile file(fileName);
    auto success = file.open(QIODevice::WriteOnly);
    if (!success) {
        return tr("Unable to open the file '%1' for writing.").arg(fileName);
    }
    auto numBytesWritten = file.write(toGeoJSON());
    if (numBytesWritten == -1) {
        file.close();
        QFile::remove(fileName);
        return tr("Unable to write to file '%1'.").arg(fileName);
    }
    file.close();
    return QString();
}


auto FlightRoute::suggestedFilename() const -> QString
{
    if (_waypoints.size() < 2) {
        return tr("Flight Route");
    }

    // NEU (1): ICAO-Code UND Namen nennen, soweit Code vorhanden:
    QStringList resultList;

    //
    // Get name for start point (e.g. "EDTL (LAHR)")
    //
    QString start = _waypoints.constFirst().ICAOCode(); // ICAO code of start point
    QString name = _waypoints.constFirst().name(); // Name of start point
    name.replace("(", "");
    name.replace(")", "");
    if (name.length() > 11) {  // Shorten name
        name = name.left(10)+"_";
    }
    if (!name.isEmpty()) {
        if (start.isEmpty()) {
            start = name;
        } else {
            start += " (" + name + ")";
        }
    }

    //
    // Get name for end point (e.g. "EDTG (BREMGARTEN)")
    //
    QString end = _waypoints.constLast().ICAOCode(); // ICAO code of end point
    name = _waypoints.constLast().name(); // Name of end point
    name.replace("(", "");
    name.replace(")", "");
    if (name.length() > 11) {  // Shorten name
        name = name.left(10)+"_";
    }
    if (!name.isEmpty()) {
        if (end.isEmpty()) {
            end = name;
        } else {
            end += " (" + name + ")";
        }
    }

    // Remove some problematic characters
    start.replace("/", "-");
    end.replace("/", "-");

    // Compile final result

    if (start.isEmpty() && end.isEmpty()) {
        return tr("Flight Route");
    }

    if (start.isEmpty()) {
        return end;
    }

    if (end.isEmpty()) {
        return start;
    }


    return start + " - " + end;
}


auto FlightRoute::summary() const -> QString {
    return makeSummary(false);
}


auto FlightRoute::summaryMetric() const -> QString {
    return makeSummary(true);
}


auto FlightRoute::toGeoJSON() const -> QByteArray
{
    QJsonArray waypointArray;
    foreach(auto waypoint, _waypoints) {
        waypointArray.append(waypoint.toJSON());
    }
    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("type"), "FeatureCollection");
    jsonObj.insert(QStringLiteral("features"), waypointArray);
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}


void FlightRoute::updateLegs()
{
    foreach(auto _leg, _legs)
        _leg->deleteLater();
    _legs.clear();

    for(int i=0; i<_waypoints.size()-1; i++) {
        _legs.append(new Leg(_waypoints.at(i), _waypoints.at(i+1), Aircraft::globalInstance(), Weather::Wind::globalInstance(), this));
    }
}
