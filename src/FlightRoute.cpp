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

FlightRoute::FlightRoute(Aircraft *aircraft, Weather::Wind *wind, QObject *parent)
    : QObject(parent), _aircraft(aircraft), _wind(wind)
{
    stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flight route.geojson";

    // Load last flightRoute
    loadFromGeoJSON(stdFileName);

    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::saveToStdLocation);
    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::summaryChanged);
    if (!_aircraft.isNull()) {
        connect(_aircraft, &Aircraft::valChanged, this, &FlightRoute::summaryChanged);
    }
    if (!_wind.isNull()) {
        connect(_wind, &Weather::Wind::valChanged, this, &FlightRoute::summaryChanged);
    }
}


void FlightRoute::append(QObject *waypoint)
{
    if (waypoint == nullptr) {
        return;
    }
    if (!waypoint->inherits("GeoMaps::Waypoint")) {
        return;
    }

    auto* wp =  qobject_cast<GeoMaps::Waypoint*>(waypoint);
    auto* myWp = new GeoMaps::Waypoint(*wp, this);
    QQmlEngine::setObjectOwnership(myWp, QQmlEngine::CppOwnership);
    connect(myWp, &GeoMaps::Waypoint::extendedNameChanged, this, &FlightRoute::waypointsChanged);
    _waypoints.append(myWp);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::append(const QGeoCoordinate& position)
{
    append(new GeoMaps::Waypoint(position, this));
}


auto FlightRoute::boundingRectangle() const -> QGeoRectangle
{
    QGeoRectangle bbox;

    for(const auto &_waypoint : _waypoints) {
        if (_waypoint.isNull()) {
            continue;
        }
        if (!_waypoint->isValid()) {
            continue;
        }

        QGeoCoordinate position = _waypoint->coordinate();
        if (!bbox.isValid()) {
            bbox.setTopLeft(position);
            bbox.setBottomRight(position);
        } else {
            bbox.extendRectangle(position);
        }
    }

    return bbox;
}


auto FlightRoute::canAppend(GeoMaps::Waypoint *other) const -> bool
{
    if (other == nullptr) {
        return true;
    }
    if (_waypoints.isEmpty() ) {
        return true;
    }

    return !_waypoints.last()->isNear(other);
}


void FlightRoute::clear()
{
    qDeleteAll(_waypoints);
    _waypoints.clear();

    updateLegs();
    emit waypointsChanged();
}


auto FlightRoute::contains(QObject * waypoint) const -> bool
{
    if (waypoint == nullptr) {
        return false;
    }
    if (!waypoint->inherits("GeoMaps::Waypoint")) {
        return false;
    }

    auto *testWaypoint = qobject_cast<GeoMaps::Waypoint *>(waypoint);
    // must loop over list in order to compare values (not pointers)
    foreach(auto _waypoint, _waypoints) {
        if (_waypoint.isNull()) {
            continue;
        }
        if (!_waypoint->isValid()) {
            continue;
        }
        if (*_waypoint == *testWaypoint) {
            return true;
        }
    }
    return false;
}


auto FlightRoute::firstWaypointObject() const -> QObject*
{
    if (_waypoints.isEmpty()) {
        return nullptr;
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
        if (!_waypoint->isValid()) {
            return QVariantList();
        }
        result.append(QVariant::fromValue(_waypoint->coordinate()));
    }

    return result;
}


auto FlightRoute::lastWaypointObject() const -> QObject*
{
    if (_waypoints.isEmpty()) {
        return nullptr;
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

    QVector<QPointer<GeoMaps::Waypoint>> newWaypoints;
    foreach(auto value, document.object()["features"].toArray()) {
        auto *wp = new GeoMaps::Waypoint(value.toObject(), this);
        if (!wp->isValid()) {
            qDeleteAll(newWaypoints);
            return tr("Cannot parse content of file '%1'.").arg(fileName);
        }
        QQmlEngine::setObjectOwnership(wp, QQmlEngine::CppOwnership);
        connect(wp, &GeoMaps::Waypoint::extendedNameChanged, this, &FlightRoute::waypointsChanged);
        newWaypoints.append(wp);
    }

    qDeleteAll(_waypoints);
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
        result += tr("Total: %1&nbsp;NM").arg(dist.toNM(), 0, 'f', 1);
    }
    if (time.isFinite()) {
        result += QStringLiteral(" • %1&nbsp;h").arg(time.toHoursAndMinutes());
    }
    if (qIsFinite(fuelInL)) {
        result += QStringLiteral(" • %1&nbsp;L").arg(qRound(fuelInL));
    }


    QStringList complaints;
    if (!_aircraft.isNull()) {
        if (!qIsFinite(_aircraft->cruiseSpeedInKT())) {
            complaints += tr("Cruise speed not specified.");
        }
        if (!qIsFinite(_aircraft->fuelConsumptionInLPH())) {
            complaints += tr("Fuel consumption not specified.");
        }
    }
    if (!_wind.isNull()) {
        if (!qIsFinite(_wind->windSpeedInKT())) {
            complaints += tr("Wind speed not specified.");
        }
        if (!qIsFinite(_wind->windDirectionInDEG())) {
            if (!qIsFinite(_wind->windDirectionInDEG())) {
                complaints += tr("Wind direction not specified.");
            }
        }
    }

    if (!complaints.isEmpty()) {
        result += tr("<p><font color='red'>Computation incomplete. %1</font></p>").arg(complaints.join(QStringLiteral(" ")));
    }

    return result;
}


auto FlightRoute::midFieldWaypoints() const -> QList<QObject*>
{
    QList<QObject*> result;

    if (_waypoints.isEmpty()) {
        return result;
    }

    foreach(auto wpt, _waypoints) {
        if (wpt == nullptr) {
            continue;
        }
        if (wpt->getPropery("CAT") == "WP") {
            result << wpt;
        }
    }

    return result;
}


void FlightRoute::moveDown(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr) {
        return;
    }
    if (!waypoint->inherits("GeoMaps::Waypoint")) {
        return;
    }
    if (waypoint == lastWaypointObject()) {
        return;
    }

    auto *swp = qobject_cast<GeoMaps::Waypoint*>(waypoint);

    auto idx = _waypoints.indexOf(swp);
    _waypoints.move(idx, idx+1);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::moveUp(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr) {
        return;
    }
    if (!waypoint->inherits("GeoMaps::Waypoint")) {
        return;
    }
    if (waypoint == firstWaypointObject()) {
        return;
    }

    auto *swp = qobject_cast<GeoMaps::Waypoint*>(waypoint);

    auto idx = _waypoints.indexOf(swp);
    if (idx > 0) {
        _waypoints.move(idx, idx-1);
    }

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::removeWaypoint(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr) {
        return;
    }
    if (!waypoint->inherits("GeoMaps::Waypoint")) {
        return;
    }

    auto *waypointPtr = qobject_cast<GeoMaps::Waypoint*>(waypoint);

    // if called from the waypoint dialog, the waypoint is not the
    // same instance as the one in `_waypoints`
    if (!_waypoints.contains(waypointPtr)) {
        foreach(const auto &_waypoint, _waypoints) {
            if (_waypoint.isNull()) {
                continue;
            }
            if (!_waypoint->isValid()) {
                continue;
            }
            if (*_waypoint == *waypointPtr) {
                waypointPtr = _waypoint;
                break;
            }
        }
    }

    _waypoints.removeOne(waypointPtr);
    delete waypointPtr;

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::reverse()
{
    std::reverse(_waypoints.begin(), _waypoints.end());
    updateLegs();
    emit waypointsChanged();
}


auto FlightRoute::routeObjects() const -> QList<QObject*>
{
    QList<QObject*> result;

    if (_waypoints.isEmpty()) {
        return result;
    }

    result.reserve(2*_waypoints.size()-1);
    for(int i=0; i<_waypoints.size()-1; i++) {
        result.append(_waypoints[i]);
        result.append(_legs[i]);
    }
    result.append(_waypoints.last());

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
    QString start = _waypoints.constFirst()->getPropery(QStringLiteral("COD")).toString(); // ICAO code of start point
    QString name = _waypoints.constFirst()->getPropery(QStringLiteral("NAM")).toString(); // Name of start point
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
    QString end = _waypoints.constLast()->getPropery(QStringLiteral("COD")).toString(); // ICAO code of end point
    name = _waypoints.constLast()->getPropery(QStringLiteral("NAM")).toString(); // Name of end point
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
        waypointArray.append(waypoint->toJSON());
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
        _legs.append(new Leg(_waypoints.at(i), _waypoints.at(i+1), _aircraft, _wind, this));
    }
}
