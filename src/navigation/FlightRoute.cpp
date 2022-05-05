/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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
#include <QStandardPaths>

#include "FlightRoute.h"
#include "GlobalObject.h"
#include "navigation/Navigator.h"


//
// Constructors and destructors
//

Navigation::FlightRoute::FlightRoute(QObject *parent)
    : QObject(parent)
{

    stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flight route.geojson";

    // Load last flightRoute
    loadFromGeoJSON(stdFileName);

    connect(this, &FlightRoute::waypointsChanged, this, &Navigation::FlightRoute::saveToStdLocation);
    connect(this, &FlightRoute::waypointsChanged, this, &Navigation::FlightRoute::summaryChanged);
    connect(GlobalObject::navigator(), &Navigation::Navigator::aircraftChanged, this, &Navigation::FlightRoute::summaryChanged);
    connect(GlobalObject::navigator(), &Navigation::Navigator::windChanged, this, &Navigation::FlightRoute::summaryChanged);
}


//
// Getter Methods
//

auto Navigation::FlightRoute::boundingRectangle() const -> QGeoRectangle
{
    QGeoRectangle bbox;

    for(const auto &_waypoint : m_waypoints) {
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


auto Navigation::FlightRoute::geoPath() const -> QVariantList
{
    QVariantList result;
    for(const auto& _waypoint : m_waypoints) {
        if (!_waypoint.isValid()) {
            return {};
        }
        result.append(QVariant::fromValue(_waypoint.coordinate()));
    }

    return result;
}


auto Navigation::FlightRoute::midFieldWaypoints() const -> QVariantList
{
    QVariantList result;

    if (m_waypoints.isEmpty()) {
        return result;
    }

    foreach(auto wpt, m_waypoints) {
        if (wpt.category() == QLatin1String("WP")) {
            result << QVariant::fromValue(wpt);
        }
    }

    return result;
}


auto Navigation::FlightRoute::summary() const -> QString
{

    if (m_legs.empty()) {
        return {};
    }

    QString result;

    const auto aircraft = GlobalObject::navigator()->aircraft();
    const auto wind = GlobalObject::navigator()->wind();
    auto dist = Units::Distance::fromM(0.0);
    auto time = Units::Time::fromS(0.0);
    auto fuel = Units::Volume::fromL(0.0);

    for(const auto& _leg : m_legs) {
        dist += _leg.distance();
        if (dist.toM() > 100) {
            time += _leg.ETE(wind, aircraft);
            fuel += _leg.Fuel(wind, aircraft);
        }
    }
    if (!dist.isFinite()) {
        return {};
    }

    result += tr("Total: %1").arg( aircraft.horizontalDistanceToString(dist) );

    if (time.isFinite()) {
        result += QStringLiteral(" • ETE %1 h").arg(time.toHoursAndMinutes());
    }
    if (fuel.isFinite()) {
        result += QStringLiteral(" • %1").arg(aircraft.volumeToString(fuel));
    }


    QStringList complaints;
    if ( !aircraft.cruiseSpeed().isFinite() ) {
        complaints += tr("Cruise speed not specified.");
    }
    if (!aircraft.fuelConsumption().isFinite()) {
        complaints += tr("Fuel consumption not specified.");
    }
    if (!wind.speed().isFinite()) {
        complaints += tr("Wind speed not specified.");
    }
    if (!wind.directionFrom().isFinite()) {
        complaints += tr("Wind direction not specified.");
    }

    if (!complaints.isEmpty()) {
        result += tr("<p><font color='red'>Computation incomplete. %1</font></p>").arg(complaints.join(QStringLiteral(" ")));
    }

    return result;

}


auto Navigation::FlightRoute::waypoints() const -> QVariantList
{
    QVariantList result;

    foreach(auto wpt, m_waypoints) {
        result << QVariant::fromValue(wpt);
    }

    return result;
}


//
// METHODS
//

void Navigation::FlightRoute::append(const GeoMaps::Waypoint &waypoint)
{
    m_waypoints.append(waypoint);

    updateLegs();
    emit waypointsChanged();
}


void Navigation::FlightRoute::append(const QGeoCoordinate& position)
{
    append( GeoMaps::Waypoint(position) );
}


auto Navigation::FlightRoute::canAppend(const GeoMaps::Waypoint &other) const -> bool
{
    if (m_waypoints.isEmpty() ) {
        return true;
    }

    return !m_waypoints.last().isNear(other);
}


void Navigation::FlightRoute::clear()
{
    m_waypoints.clear();

    updateLegs();
    emit waypointsChanged();
}


auto Navigation::FlightRoute::contains(const GeoMaps::Waypoint& waypoint) const -> bool
{
    foreach(auto _waypoint, m_waypoints) {
        if (!_waypoint.isValid()) {
            continue;
        }
        if (_waypoint.isNear(waypoint)) {
            return true;
        }
    }
    return false;
}


auto Navigation::FlightRoute::lastIndexOf(const GeoMaps::Waypoint& waypoint) const -> int
{

    for(int i=m_waypoints.size()-1; i>=0; i--) {
        auto _waypoint = m_waypoints.at(i);
        if (!_waypoint.isValid()) {
            continue;
        }
        if (_waypoint.isNear(waypoint)) {
            return i;
        }
    }
    return -1;

}


auto Navigation::FlightRoute::loadFromGeoJSON(QString fileName) -> QString
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

    QVector<GeoMaps::Waypoint> newWaypoints;
    foreach(auto value, document.object()[QStringLiteral("features")].toArray()) {
        auto wp = GeoMaps::Waypoint(value.toObject());
        if (!wp.isValid()) {
            return tr("Cannot parse content of file '%1'.").arg(fileName);
        }

        newWaypoints.append(wp);
    }

    m_waypoints = newWaypoints;
    updateLegs();
    emit waypointsChanged();

    return {};
}


void Navigation::FlightRoute::moveDown(int idx)
{
    // Paranoid safety checks
    if ((idx < 0) || (idx > m_waypoints.size()-2)) {
        return;
    }

    m_waypoints.move(idx, idx+1);

    updateLegs();
    emit waypointsChanged();
}


void Navigation::FlightRoute::moveUp(int idx)
{
    // Paranoid safety checks
    if ((idx < 1) || (idx >= m_waypoints.size())) {
        return;
    }

    m_waypoints.move(idx, idx-1);

    updateLegs();
    emit waypointsChanged();
}


void Navigation::FlightRoute::removeWaypoint(int idx)
{
    // Paranoid safety checks
    if ((idx < 0) || (idx >= m_waypoints.size())) {
        return;
    }

    m_waypoints.removeAt(idx);
    updateLegs();
    emit waypointsChanged();
}


void Navigation::FlightRoute::relocateWaypoint(int idx, double latitude, double longitude)
{
    // Paranoid safety checks
    if ((idx < 0) || (idx >= m_waypoints.size())) {
        return;
    }


    // If the new coordinate is invalid of closer than 100m to the old coordinate, then do nothing.
    QGeoCoordinate newCoordinate(latitude, longitude);
    if (!newCoordinate.isValid()) {
        return;
    }
    if (m_waypoints[idx].coordinate().isValid() && (m_waypoints[idx].coordinate().distanceTo(newCoordinate) < 10.0)) {
        return;
    }


    m_waypoints[idx] = m_waypoints[idx].relocated(newCoordinate);
    updateLegs();
    emit waypointsChanged();
}


void Navigation::FlightRoute::renameWaypoint(int idx, const QString& newName)
{
    // Paranoid safety checks
    if ((idx < 0) || (idx >= m_waypoints.size())) {
        return;
    }
    // If name did not
    if (m_waypoints[idx].name() == newName) {
        return;
    }


    m_waypoints[idx] = m_waypoints[idx].renamed(newName);
    updateLegs();
    emit waypointsChanged();
}


void Navigation::FlightRoute::reverse()
{
    std::reverse(m_waypoints.begin(), m_waypoints.end());
    updateLegs();
    emit waypointsChanged();
}


auto Navigation::FlightRoute::save(const QString& fileName) const -> QString
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
    return {};
}


auto Navigation::FlightRoute::suggestedFilename() const -> QString
{
    if (m_waypoints.size() < 2) {
        return tr("Flight Route");
    }

    // NEU (1): ICAO-Code UND Namen nennen, soweit Code vorhanden:
    QStringList resultList;

    //
    // Get name for start point (e.g. "EDTL (LAHR)")
    //
    QString start = m_waypoints.constFirst().ICAOCode(); // ICAO code of start point
    QString name = m_waypoints.constFirst().name(); // Name of start point
    name.replace(QLatin1String("("), QLatin1String(""));
    name.replace(QLatin1String(")"), QLatin1String(""));
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
    QString end = m_waypoints.constLast().ICAOCode(); // ICAO code of end point
    name = m_waypoints.constLast().name(); // Name of end point
    name.replace(QLatin1String("("), QLatin1String(""));
    name.replace(QLatin1String(")"), QLatin1String(""));
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
    start.replace(QLatin1String("/"), QLatin1String("-"));
    end.replace(QLatin1String("/"), QLatin1String("-"));

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


auto Navigation::FlightRoute::toGeoJSON() const -> QByteArray
{
    QJsonArray waypointArray;
    foreach(auto waypoint, m_waypoints) {
        waypointArray.append(waypoint.toJSON());
    }
    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("type"), "FeatureCollection");
    jsonObj.insert(QStringLiteral("features"), waypointArray);
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}


void Navigation::FlightRoute::updateLegs()
{
    m_legs.clear();

    for(int i=0; i<m_waypoints.size()-1; i++) {
        m_legs.append(Leg(m_waypoints.at(i), m_waypoints.at(i+1)));
    }
}

