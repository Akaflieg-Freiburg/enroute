/***************************************************************************
 *   Copyright (C) 2022-2026 by Stefan Kebekus                             *
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

#include <QAbstractListModel>
#include <QQmlEngine>
#include <QStandardPaths>

#include "GlobalObject.h"
#include "geomaps/Waypoint.h"

namespace GeoMaps
{

    /*! \brief Library of user-defined waypoints
     *
     *  This class holds the list of user-defined waypoints, sorted by name, and
     *  exposes it as a list model with the roles 'waypoint' and 'name'. The
     *  library is loaded from a GeoJSON file on construction and saved every
     *  time that a change is made.
     *
     *  Changes are announced with row-granular model signals, so that views
     *  bound to this model update incrementally, and with the coarse signal
     *  waypointsChanged() for consumers of the property 'waypoints'. In QML,
     *  bind a view to this singleton through a NameFilterProxyModel.
     */

    class WaypointLibrary : public QAbstractListModel
    {
        Q_OBJECT
        QML_ELEMENT
        QML_SINGLETON

    public:
        /*! \brief Creates a new waypoint library
         *
         * This constructor creates a new WaypointLibrary instance. The library
         * is loaded from a GeoJSON file whose name is found in the private
         * member stdFileName.
         *
         * @param parent The standard QObject parent
         */
        explicit WaypointLibrary(QObject *parent = nullptr);

        // No default constructor, important for QML singleton
        explicit WaypointLibrary() = delete;

        // factory function for QML singleton
        static GeoMaps::WaypointLibrary* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
        {
            return GlobalObject::waypointLibrary();
        }

        ~WaypointLibrary() override = default;

        /*! \brief Model roles */
        enum Role : int {
            /*! \brief The waypoint, of type GeoMaps::Waypoint */
            WaypointRole = Qt::UserRole + 1,

            /*! \brief Name of the waypoint, a QString */
            NameRole
        };


        //
        // Properties
        //

        /*! \brief List of waypoints
         *
         *  This property holds the list of waypoints, in alphabetical order
         */
        Q_PROPERTY(QList<GeoMaps::Waypoint> waypoints READ waypoints NOTIFY waypointsChanged)

        /*! \brief List of waypoints
         *
         *  This property holds the list of waypoints, in alphabetical order.
         */
        Q_PROPERTY(QByteArray GeoJSON READ GeoJSON NOTIFY waypointsChanged)


        //
        // Getter Methods
        //

        /*! \brief Getter function for property with the same name
         *
         * @returns Property waypoints
         */
        [[nodiscard]] QList<GeoMaps::Waypoint> waypoints() const
        {
            return m_waypoints;
        }

        /*! \brief Getter function for property with the same name
         *
         * @returns Property GeoJSON
         */
        [[nodiscard]] QByteArray GeoJSON() const;


        //
        // Model API
        //

        /*! \brief Re-implemented from QAbstractListModel
         *
         *  @param parent Parent index, invalid for the list itself
         *
         *  @returns Number of waypoints, or zero for a valid parent
         */
        [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;

        /*! \brief Re-implemented from QAbstractListModel
         *
         *  @param index Model index
         *
         *  @param role One of the roles in WaypointLibrary::Role
         *
         *  @returns Data for the role, or an invalid QVariant
         */
        [[nodiscard]] QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

        /*! \brief Re-implemented from QAbstractListModel
         *
         *  @returns Names of the roles in WaypointLibrary::Role
         */
        [[nodiscard]] QHash<int, QByteArray> roleNames() const override;


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

        /*! \brief Check if the library contains a waypoint near to a given one
         *
         *  The method checks proximity with the method GeoMaps::Waypoint::isNear
         *
         *  @param waypoint Waypoint
         *
         *  @returns True if yes
         */
        [[nodiscard]] Q_INVOKABLE bool hasNearbyEntry(const GeoMaps::Waypoint& waypoint) const;

        /*! \brief Import waypoints into the library
         *
         *  This method reads waypoints from a file and adds them to the library.
         *
         *  @param fileName Name of file to import. Must be in CUP, GPX or GeoJSON format.
         *
         *  @param skip If true, skip over waypoints that already exist in the library
         *
         *  @return Human-readable error message, or an empty string on success
         */
        [[nodiscard]] Q_INVOKABLE QString import(const QString& fileName, bool skip);

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
        Q_INVOKABLE bool remove(const GeoMaps::Waypoint &waypoint);

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
        Q_INVOKABLE bool replace(const GeoMaps::Waypoint &oldWaypoint, const GeoMaps::Waypoint &newWaypoint);

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
        /*! \brief Notification signal for the property with the same name
         *
         *  This coarse signal is emitted after every change, following the
         *  row-granular model signals.
         */
        void waypointsChanged();

    private:
        Q_DISABLE_COPY_MOVE(WaypointLibrary)

        // Row at which a waypoint with the given name is to be inserted, so
        // that m_waypoints stays sorted by name
        [[nodiscard]] int insertionRow(const QString& name) const;

        // Sorts a list of waypoints by name
        static void sortByName(QList<GeoMaps::Waypoint>& waypoints);

        // Standard file name for save() and loadFromGeoJGON() methods
        QString stdFileName{QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/waypoint library.geojson"};

        // Acutual list of waypoints, sorted by name
        QList<GeoMaps::Waypoint> m_waypoints;
    };

} // namespace GeoMaps
