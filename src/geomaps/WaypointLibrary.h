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

#include <QStandardPaths>

#include "geomaps/Waypoint.h"
#include "GlobalObject.h"


namespace GeoMaps {

#warning Documentation
/*! \brief Library of user-defined waypoints
 *
 *  This simple class that is little more than a list of waypoints, together with some auxiliary methods.
 *  The list is automatically loaded on startup, and saved every time that a change is made.
 */

class WaypointLibrary : public GlobalObject
{
    Q_OBJECT

public:
    /*! \brief Creates a new waypoin library
     *
     * This constructor creates a new WaypointLibrary instance. The library is loaded from a GeoJSON file whose name is found in the private member stdFileName.
     *
     * @param parent The standard QObject parent
     */
    explicit WaypointLibrary(QObject* parent = nullptr);


    //
    // Properties
    //

    /*! \brief List of waypoints
     *
     *  This property holds the list of waypoints, in alphabetical order
     */
    Q_PROPERTY(QVector<GeoMaps::Waypoint> waypoints READ waypoints NOTIFY waypointsChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property with the same name
     *
     * @returns Property waypoints
     */
    [[nodiscard]] QVector<GeoMaps::Waypoint> waypoints() const
    {
        return m_waypoints;
    }


    //
    // Methods
    //

    /*! \brief Checks if library contains an given waypoint
     *
     * @param waypoint Waypoint
     *
     * @returns True if an exact copy of the waypoint is found in the library
     */
    [[nodiscard]] Q_INVOKABLE bool contains(const GeoMaps::Waypoint& waypoint) const
    {
        return m_waypoints.contains(waypoint);
    }

    /*! \brief Lists all entries in the waypoint library whose name contains the string 'filter'
     *
     * The check for string containment is done in a fuzzy way.
     *
     * @param filter String used to filter the list
     *
     * @returns A filtered list with of waypoint, in alphabetical order
     */
    [[nodiscard]] Q_INVOKABLE QVector<GeoMaps::Waypoint> filteredWaypoints(const QString& filter) const;

    /*! \brief Remove waypoint
     *
     * Removes the first waypoint from the list that matches the given waypoint exactly. If no waypoint
     * matches, this method does nothing.
     *
     * @param waypoint Waypoint to be removed
     *
     * @returns True if a waypoint has indeed been removed.
     */
    [[nodiscard]] Q_INVOKABLE bool remove(const GeoMaps::Waypoint& waypoint);

    /*! \brief Replace waypoint
     *
     * Replaces the first waypoint from the list that matches the given oldWaypoint exactly. If no waypoint
     * matches, this method does nothing.
     *
     * @param oldWaypoint Waypoint that shall be replaced
     *
     * @param newWaypoint Waypoint replacement
     *
     * @returns True if a waypoint has indeed been replaced.
     */
    [[nodiscard]] Q_INVOKABLE bool replace(const GeoMaps::Waypoint& oldWaypoint, const GeoMaps::Waypoint& newWaypoint);

    // -----------------------------------


    [[nodiscard]] Q_INVOKABLE bool hasNearbyEntry(const GeoMaps::Waypoint& waypoint) const
    {
        for(const auto& wp : qAsConst(m_waypoints))
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
#warning need to sort
    }

    QString stdFileName {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/waypoint library.geojson"};

    [[nodiscard]] QString save(QString fileName={}) const;

    [[nodiscard]] QByteArray toGeoJSON() const;

    [[nodiscard]] QString loadFromGeoJSON(QString fileName={});

signals:
    void waypointsChanged();


private:
    Q_DISABLE_COPY_MOVE(WaypointLibrary)

    QVector<GeoMaps::Waypoint> m_waypoints;
};

};
