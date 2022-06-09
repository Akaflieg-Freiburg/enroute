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

#pragma once

#include "geomaps/Waypoint.h"
#include "GlobalObject.h"


namespace GeoMaps {

#warning Documentation
/*! \brief Library of user-defined waypoints

 */

class WaypointLibrary : public GlobalObject
{
    Q_OBJECT

public:
    /*! \brief Creates a new GeoMap provider
     *
     * This constructor creates a new GeoMapProvider instance.
     *
     * @param parent The standard QObject parent
     */
    explicit WaypointLibrary(QObject *parent = nullptr);

    // deferred initialization
    void deferredInitialization() override;

    /*! \brief Destructor */
    ~WaypointLibrary() override = default;


    //
    // Properties
    //

    Q_PROPERTY(QVector<GeoMaps::Waypoint> waypoints READ waypoints NOTIFY waypointsChanged)


    //
    // Getter Methods
    //

    [[nodiscard]] QVector<GeoMaps::Waypoint> waypoints() const
    {
        return m_waypoints;
    }


    //
    // Methods
    //

    [[nodiscard]] Q_INVOKABLE QVector<GeoMaps::Waypoint> filteredWaypoints(const QString& filter) const;

    Q_INVOKABLE void remove(const GeoMaps::Waypoint& waypoint);
    Q_INVOKABLE void replace(const GeoMaps::Waypoint& oldWaypoint, const GeoMaps::Waypoint& newWaypoint);

    [[nodiscard]] Q_INVOKABLE bool contains(const GeoMaps::Waypoint& waypoint) const
    {
        return m_waypoints.contains(waypoint);
    }

    [[nodiscard]] Q_INVOKABLE bool hasNearbyEntry(const GeoMaps::Waypoint& waypoint) const
    {
        for(const auto wp : qAsConst(m_waypoints))
        {
            if (wp.isNear(waypoint))
            {
                return true;
            }
        }
        return false;
    }

    Q_INVOKABLE void add(const GeoMaps::Waypoint& waypoint)
    {
        m_waypoints.append(waypoint);
        emit waypointsChanged();
    }

signals:
    void waypointsChanged();


private:
    Q_DISABLE_COPY_MOVE(WaypointLibrary)

    QVector<GeoMaps::Waypoint> m_waypoints;
};

};
