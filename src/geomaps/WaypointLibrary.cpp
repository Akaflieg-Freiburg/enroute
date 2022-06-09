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

#include <QtConcurrent/QtConcurrentRun>
#include <QJsonArray>
#include <QLockFile>
#include <QRandomGenerator>

#include "Librarian.h"
#include "geomaps/WaypointLibrary.h"
#include "geomaps/MBTILES.h"
#include "navigation/Navigator.h"


GeoMaps::WaypointLibrary::WaypointLibrary(QObject *parent)
    : GlobalObject(parent)
{

#warning Demo only
    Waypoint wp( QGeoCoordinate(48.1259185, 8.0229407) );
    wp = wp.renamed("Hörnlisberg");
    m_waypoints.append(wp);
}

void GeoMaps::WaypointLibrary::deferredInitialization()
{
}


//
// Getter Methods
//



//
// Private Methods and Slots
//

QVector<GeoMaps::Waypoint> GeoMaps::WaypointLibrary::filteredWaypoints(const QString& filter) const
{
    QVector<GeoMaps::Waypoint> result;

    QString simplifiedFilter = GlobalObject::librarian()->simplifySpecialChars(filter);
    foreach(auto waypoint, m_waypoints) {
        auto simplifiedName = GlobalObject::librarian()->simplifySpecialChars(waypoint.name());
        if (simplifiedName.contains(simplifiedFilter, Qt::CaseInsensitive)) {
            result.append(waypoint);
        }
    }
#warning need to sort
    return result;
}

void GeoMaps::WaypointLibrary::remove(const GeoMaps::Waypoint& waypoint)
{
    auto oldLen = m_waypoints.size();
#warning use return value
    m_waypoints.removeOne(waypoint);
    if (oldLen != m_waypoints.size())
    {
        emit waypointsChanged();
    }
#warning should have return value
}

void GeoMaps::WaypointLibrary::replace(const GeoMaps::Waypoint& oldWaypoint, const GeoMaps::Waypoint& newWaypoint)
{
    auto oldLen = m_waypoints.size();
    m_waypoints.removeOne(oldWaypoint);
#warning use return value
    if (oldLen == m_waypoints.size())
    {
        return;
    }

    m_waypoints.append(newWaypoint);
    emit waypointsChanged();
#warning should have return value
}

