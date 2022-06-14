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

namespace GeoMaps
{

    /*! \brief Library of user-defined waypoints
     *
     *  This simple class that is little more than a list of waypoints, together
     *  with some auxiliary methods. The list is automatically loaded on startup,
     *  and saved every time that a change is made.
     */

    class WaypointLibrary : public GlobalObject
    {
        Q_OBJECT

    public:
        /*! \brief Creates a new waypoin library
         *
         * This constructor creates a new WaypointLibrary instance. The library
         * is loaded from a GeoJSON file whose name is found in the private
         * member stdFileName.
         *
         * @param parent The standard QObject parent
         */
        explicit WaypointLibrary(QObject *parent = nullptr);

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

        /*! \brief Adds a waypoint to the library
         *
         *  @param waypoint Waypoint to be added. If that waypoint is invalid,
         *  this method will not do anything
         */
        Q_INVOKABLE void add(const GeoMaps::Waypoint &waypoint);

        /*! \brief Clears the waypoint library */
        Q_INVOKABLE void clear();

        /*! \brief Checks if library contains an given waypoint
         *
         * @param waypoint Waypoint
         *
         * @returns True if an exact copy of the waypoint is found in the
         * library
         */
        [[nodiscard]] Q_INVOKABLE bool contains(const GeoMaps::Waypoint &waypoint) const
        {
            return m_waypoints.contains(waypoint);
        }

        /*! \brief Lists all entries in the waypoint library whose name contains
         * the string 'filter'
         *
         * The check for string containment is done in a fuzzy way.
         *
         * @param filter String used to filter the list
         *
         * @returns A filtered list with of waypoint, in alphabetical order
         */
        [[nodiscard]] Q_INVOKABLE QVector<GeoMaps::Waypoint> filteredWaypoints(const QString &filter) const;

        /*! \brief Check if the library contains a waypoint near to a given one
         *
         *  The method checks proximity with the method GeoMaps::Waypoint::isNear
         *
         *  @param waypoint Waypoint
         *
         *  @returns True if yes
         */
        [[nodiscard]] Q_INVOKABLE bool hasNearbyEntry(const GeoMaps::Waypoint &waypoint) const;

        /*! \brief Read from file
         *
         * Reads the library from a file in GeoJSON format. On sucess, the
         * current library is replaced in full. On error, the current library is
         * not touched at all.
         *
         * @param fileName File name. If emty, a standard file name will be
         * used, in QStandardPaths::AppDataLocation. See the private member
         * stdFileName for details.
         *
         * @returns An empty string on success and a human-readable tranlated
         * error message otherwise.
         */
        [[nodiscard]] Q_INVOKABLE QString loadFromGeoJSON(QString fileName = {});

        /*! \brief Remove waypoint
         *
         * Removes the first waypoint from the list that matches the given
         * waypoint exactly. If no waypoint matches, this method does nothing.
         *
         * @param waypoint Waypoint to be removed
         *
         * @returns True if a waypoint has indeed been removed.
         */
        [[nodiscard]] Q_INVOKABLE bool remove(const GeoMaps::Waypoint &waypoint);

        /*! \brief Replace waypoint
         *
         * Replaces the first waypoint from the list that matches the given
         * oldWaypoint exactly. If no waypoint matches, this method does
         * nothing.
         *
         * @param oldWaypoint Waypoint that shall be replaced
         *
         * @param newWaypoint Waypoint replacement. If this waypoint is invalid,
         * the method returns immediately and does nothing.
         *
         * @returns True if a waypoint has indeed been replaced.
         */
        [[nodiscard]] Q_INVOKABLE bool replace(const GeoMaps::Waypoint &oldWaypoint, const GeoMaps::Waypoint &newWaypoint);

        /*! \brief Save to file
         *
         * Saves the library in GeoJSON format.
         *
         * @param fileName File name. If emty, a standard file name will be
         * used, in QStandardPaths::AppDataLocation. See the private member
         * stdFileName for details.
         *
         * @returns An empty string on success and a human-readable tranlated
         * error message otherwise.
         */
        [[nodiscard]] Q_INVOKABLE QString save(QString fileName = {}) const;

        /*! \brief Serialize into GeoJSON document
         *
         * Serializes the library to as GeoJSON file. The resulting file contains a
         * FeatureArray, where each member is of the form described in the
         * documentation of the method GeoMaps::Waypoint::toJSON()
         *
         * @returns GeoJSON data
         */
        [[nodiscard]] Q_INVOKABLE QByteArray toGeoJSON() const;

        /*! \brief Serialize into GPX document
         *
         * This method serialises the current library as a GPX document. The
         * document conforms to the specification outlined
         * [here](https://www.topografix.com/gpx.asp)
         *
         * @returns QByteArray containing GPX data describing the flight route
         */
        [[nodiscard]] Q_INVOKABLE QByteArray toGpx() const;

    signals:
        /*! \brief Notification signal for the property with the same name */
        void waypointsChanged();

    private:
        Q_DISABLE_COPY_MOVE(WaypointLibrary)

        // Standard file name for save() and loadFromGeoJGON() methods
        QString stdFileName{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/waypoint library.geojson"};

        // Acutual list of waypoints.
        QVector<GeoMaps::Waypoint> m_waypoints;
    };

};
