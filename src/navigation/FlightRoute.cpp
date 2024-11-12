/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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
#include "fileFormats/FPL.h"
#include "fileFormats/PLN.h"
#include "geomaps/GeoJSON.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/GPX.h"
#include "navigation/Navigator.h"

using namespace Qt::Literals::StringLiterals;


//
// Constructors and destructors
//

Navigation::FlightRoute::FlightRoute(QObject *parent)
    : QObject(parent)
{
    connect(this, &FlightRoute::waypointsChanged, this, &Navigation::FlightRoute::summaryChanged);
    connect(GlobalObject::navigator(), &Navigation::Navigator::aircraftChanged, this, &Navigation::FlightRoute::summaryChanged);
    connect(GlobalObject::navigator(), &Navigation::Navigator::windChanged, this, &Navigation::FlightRoute::summaryChanged);

    // Setup Bindings
    m_geoPath.setBinding([this]() {return this->computeGeoPath();});

}


//
// Getter Methods
//

auto Navigation::FlightRoute::boundingRectangle() const -> QGeoRectangle
{
    QGeoRectangle bbox;

    for(const auto &_waypoint : m_waypoints.value())
    {
        if (!_waypoint.isValid())
        {
            continue;
        }

        QGeoCoordinate const position = _waypoint.coordinate();
        if (!bbox.isValid())
        {
            bbox.setTopLeft(position);
            bbox.setBottomRight(position);
        }
        else
        {
            bbox.extendRectangle(position);
        }
    }

    return bbox;
}

QList<QGeoCoordinate> Navigation::FlightRoute::computeGeoPath()
{
    QList<QGeoCoordinate> result;
    for(const auto& _waypoint : m_waypoints.value())
    {
        if (!_waypoint.isValid())
        {
            return {};
        }
        result.append(_waypoint.coordinate());
    }

    return result;
}

auto Navigation::FlightRoute::midFieldWaypoints() const -> QList<GeoMaps::Waypoint>
{
    QList<GeoMaps::Waypoint> result;

    foreach(auto wpt, m_waypoints.value())
    {
        if (wpt.category() == u"WP")
        {
            result << wpt;
        }
    }

    return result;
}

auto Navigation::FlightRoute::summary() const -> QString
{

    if (m_legs.empty())
    {
        return {};
    }

    QString result;

    const auto aircraft = GlobalObject::navigator()->aircraft();
    const auto wind = GlobalObject::navigator()->wind();
    auto dist = Units::Distance::fromM(0.0);
    auto time = Units::Timespan::fromS(0.0);
    auto fuel = Units::Volume::fromL(0.0);

    for(const auto& _leg : m_legs)
    {
        dist += _leg.distance();
        if (dist.toM() > 100)
        {
            time += _leg.ETE(wind, aircraft);
            fuel += _leg.Fuel(wind, aircraft);
        }
    }
    if (!dist.isFinite()) {
        return {};
    }

    result += tr("Total: %1").arg( aircraft.horizontalDistanceToString(dist) );

    if (time.isFinite())
    {
        result += QStringLiteral(" • ETE %1 h").arg(time.toHoursAndMinutes());
    }
    if (fuel.isFinite())
    {
        result += QStringLiteral(" • %1").arg(aircraft.volumeToString(fuel));
    }


    QStringList complaints;
    if ( !aircraft.cruiseSpeed().isFinite() )
    {
        complaints += tr("Cruise speed not specified.");
    }
    if (!aircraft.fuelConsumption().isFinite())
    {
        complaints += tr("Fuel consumption not specified.");
    }
    if (!wind.speed().isFinite())
    {
        complaints += tr("Wind speed not specified.");
    }
    if (!wind.directionFrom().isFinite())
    {
        complaints += tr("Wind direction not specified.");
    }

    if (!complaints.isEmpty())
    {
        result += tr("<p><font color='red'>Computation incomplete. %1</font></p>").arg(complaints.join(QStringLiteral(" ")));
    }

    return result;

}



//
// METHODS
//

void Navigation::FlightRoute::append(const GeoMaps::Waypoint& waypoint)
{
    auto newWaypoints = m_waypoints.value();
    newWaypoints.append(waypoint);
    m_waypoints = newWaypoints;

    updateLegs();
    emit waypointsChanged();
}

void Navigation::FlightRoute::append(const QGeoCoordinate& position)
{
    append( GeoMaps::Waypoint(position) );
}

auto Navigation::FlightRoute::canAppend(const GeoMaps::Waypoint &other) const -> bool
{
    if (m_waypoints.value().isEmpty())
    {
        return true;
    }

    return !m_waypoints.value().last().isNear(other);
}

auto Navigation::FlightRoute::canInsert(const GeoMaps::Waypoint &other) const -> bool
{
    if (m_waypoints.value().size() < 2)
    {
        return false;
    }
    foreach(const auto& waypoint, m_waypoints.value())
    {
        if (waypoint.isNear(other))
        {
            return false;
        }
    }
    return true;
}

void Navigation::FlightRoute::clear()
{
    m_waypoints = QVector<GeoMaps::Waypoint>();

    updateLegs();
    emit waypointsChanged();
}

auto Navigation::FlightRoute::contains(const GeoMaps::Waypoint& waypoint) const -> bool
{
    foreach(auto _waypoint, m_waypoints.value())
    {
        if (!_waypoint.isValid())
        {
            continue;
        }
        if (_waypoint.isNear(waypoint))
        {
            return true;
        }
    }
    return false;
}

void Navigation::FlightRoute::insert(const GeoMaps::Waypoint& waypoint)
{
    if (!canInsert(waypoint))
    {
        return;
    }

    int shortestIndex = 0;
    double shortestRoute = 10e9;

    auto newWaypoints = m_waypoints.value();
    for(int idx=0; idx<newWaypoints.size()-1;  idx++)
    {
        double routeSize = 0.0;
        for(int i=0; i<newWaypoints.size()-1; i++)
        {
            if (i == idx)
            {
                routeSize += newWaypoints[i].coordinate().distanceTo(waypoint.coordinate());
                routeSize += waypoint.coordinate().distanceTo(newWaypoints[i+1].coordinate());
            }
            else
            {
                routeSize += newWaypoints[i].coordinate().distanceTo(newWaypoints[i+1].coordinate());
            }
        }

        if (routeSize < shortestRoute)
        {
            shortestRoute = routeSize;
            shortestIndex = idx;
        }
    }

    newWaypoints.insert(shortestIndex+1, waypoint);
    m_waypoints = newWaypoints;
    updateLegs();
    emit waypointsChanged();
}

auto Navigation::FlightRoute::lastIndexOf(const GeoMaps::Waypoint& waypoint) const -> qsizetype
{
    for(auto i=m_waypoints.value().size()-1; i>=0; i--)
    {
        auto _waypoint = m_waypoints.value().at(i);
        if (!_waypoint.isValid())
        {
            continue;
        }
        if (_waypoint.isNear(waypoint))
        {
            return i;
        }
    }
    return -1;
}

auto Navigation::FlightRoute::load(const QString& fileName) -> QString
{
    QString myFileName = fileName;
    if (fileName.startsWith(u"file://"_s))
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
        auto pln = FileFormats::PLN(myFileName);
        result.reserve(pln.waypoints().size());
        for(const auto& coordinate : pln.waypoints())
        {
            result += coordinate;
        }
    }
    if (result.isEmpty())
    {
        auto fpl = FileFormats::FPL(myFileName);
        result.reserve(fpl.waypoints().size());
        for(const auto& coordinate : fpl.waypoints())
        {
            result += coordinate;
        }
    }
    if (result.isEmpty())
    {
        return tr("Error reading file '%1'").arg(myFileName);
    }
    if (result.length() > 100)
    {
        return tr("The file '%1' contains too many waypoints. Flight routes with more than 100 waypoints are not supported.").arg(myFileName);
    }

    QVector<GeoMaps::Waypoint> newWaypoints;
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
            newWaypoints << waypoint;
        }
        else
        {
            newWaypoints << nearest;
        }
    }
    m_waypoints = newWaypoints;

    updateLegs();
    emit waypointsChanged();
    return {};
}

void Navigation::FlightRoute::moveDown(int idx)
{
    QVector<GeoMaps::Waypoint> newWaypoints = m_waypoints.value();

    // Paranoid safety checks
    if ((idx < 0) || (idx > newWaypoints.size()-2))
    {
        return;
    }

    newWaypoints.move(idx, idx+1);
    m_waypoints = newWaypoints;

    updateLegs();
    emit waypointsChanged();
}

void Navigation::FlightRoute::moveUp(int idx)
{
    QVector<GeoMaps::Waypoint> newWaypoints = m_waypoints.value();

    // Paranoid safety checks
    if ((idx < 1) || (idx >= newWaypoints.size()))
    {
        return;
    }

    newWaypoints.move(idx, idx-1);
    m_waypoints = newWaypoints;

    updateLegs();
    emit waypointsChanged();
}

void Navigation::FlightRoute::removeWaypoint(int idx)
{
    QVector<GeoMaps::Waypoint> newWaypoints = m_waypoints.value();

    // Paranoid safety checks
    if ((idx < 0) || (idx >= newWaypoints.size()))
    {
        return;
    }

    newWaypoints.removeAt(idx);
    m_waypoints = newWaypoints;
    updateLegs();
    emit waypointsChanged();
}

void Navigation::FlightRoute::replaceWaypoint(int idx, const GeoMaps::Waypoint& newWaypoint)
{
    QVector<GeoMaps::Waypoint> newWaypoints = m_waypoints.value();

    // Paranoid safety checks
    if ((idx < 0) || (idx >= newWaypoints.size()))
    {
        return;
    }

    // If no change is necessary, then return
    if (newWaypoints[idx] == newWaypoint)
    {
        return;
    }

    newWaypoints[idx] = newWaypoint;
    m_waypoints = newWaypoints;
    updateLegs();
    emit waypointsChanged();
}

void Navigation::FlightRoute::reverse()
{
    QVector<GeoMaps::Waypoint> newWaypoints = m_waypoints.value();
    std::reverse(newWaypoints.begin(), newWaypoints.end());
    m_waypoints = newWaypoints;
    updateLegs();
    emit waypointsChanged();
}

auto Navigation::FlightRoute::save(const QString& fileName) const -> QString
{
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

auto Navigation::FlightRoute::suggestedFilename() const -> QString
{
    if (m_waypoints.value().size() < 2)
    {
        return tr("Flight Route");
    }

    //
    // Get name for start point (e.g. "EDTL (LAHR)")
    //
    auto start = m_waypoints.value().constFirst().ICAOCode(); // ICAO code of start point
    auto name = m_waypoints.value().constFirst().name(); // Name of start point
    name.replace(u'(', u""_s);
    name.replace(u')', u""_s);
    if (name.length() > 11)
    {  // Shorten name
        name = name.left(10)+"_";
    }
    if (!name.isEmpty())
    {
        if (start.isEmpty())
        {
            start = name;
        }
        else
        {
            start += " (" + name + ")";
        }
    }

    //
    // Get name for end point (e.g. "EDTG (BREMGARTEN)")
    //
    QString end = m_waypoints.value().constLast().ICAOCode(); // ICAO code of end point
    name = m_waypoints.value().constLast().name(); // Name of end point
    name.replace(u"("_s, u""_s);
    name.replace(u")"_s, u""_s);
    if (name.length() > 11)
    {  // Shorten name
        name = name.left(10)+"_";
    }
    if (!name.isEmpty())
    {
        if (end.isEmpty())
        {
            end = name;
        }
        else
        {
            end += " (" + name + ")";
        }
    }

    // Remove some problematic characters
    start.replace(u"/"_s, u"-"_s);
    end.replace(u"/"_s, u"-"_s);

    // Compile final result

    if (start.isEmpty() && end.isEmpty())
    {
        return tr("Flight Route");
    }

    if (start.isEmpty())
    {
        return end;
    }

    if (end.isEmpty())
    {
        return start;
    }


    return start + " - " + end;
}

auto Navigation::FlightRoute::toGeoJSON() const -> QByteArray
{
    QJsonArray waypointArray;
    foreach(const auto& waypoint, m_waypoints.value())
    {
        if (waypoint.isValid())
        {
            waypointArray.append(waypoint.toJSON());
        }
    }

    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("type"), "FeatureCollection");
    jsonObj.insert(QStringLiteral("enroute"), GeoMaps::GeoJSON::indicatorFlightRoute());
    jsonObj.insert(QStringLiteral("features"), waypointArray);

    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}

void Navigation::FlightRoute::updateLegs()
{
    m_legs.clear();

    for(int i=0; i<m_waypoints.value().size()-1; i++)
    {
        m_legs.append(Leg(m_waypoints.value().at(i), m_waypoints.value().at(i+1)));
    }
}

