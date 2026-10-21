/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

#include "geomaps/Waypoint.h"

namespace Ui {

/*! \brief Item model over all waypoints known to the app, for search lists
 *
 *  Rows are the waypoints of the WaypointLibrary (the user's own waypoints,
 *  first), followed by the waypoints of the installed aviation maps. Both
 *  groups are sorted by name. Library changes are forwarded row by row; a
 *  change of the aviation data replaces the map rows in a single reset.
 *
 *  The role 'searchKey' holds Librarian::simplify(name) + " " +
 *  Librarian::simplify(ICAO code). For the map rows it is precomputed on the
 *  worker thread that reads the aviation data, so that a NameFilterProxyModel
 *  with filterRoleIsSimplified can filter tens of thousands of rows on every
 *  keystroke without normalising a single name:
 *
 *  model: NameFilterProxyModel {
 *      sourceModel: WaypointSearchModel {}
 *      filterRoleName: "searchKey"
 *      filterRoleIsSimplified: true
 *      filter: textInput.filter
 *  }
 */

class WaypointSearchModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    /*! \brief Model roles */
    enum Role : int {
        /*! \brief The waypoint, of type GeoMaps::Waypoint */
        WaypointRole = Qt::UserRole + 1,

        /*! \brief Name of the waypoint, a QString */
        NameRole,

        /*! \brief Pre-simplified search key, a QString */
        SearchKeyRole
    };

    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit WaypointSearchModel(QObject* parent = nullptr);

    // Default destructor
    ~WaypointSearchModel() override = default;


    //
    // Methods
    //

    /*! \brief Implementation of QAbstractListModel::rowCount
     *
     *  @param parent Parent index, ignored for a list
     *
     *  @returns Number of library waypoints plus number of map waypoints
     */
    [[nodiscard]] int rowCount(const QModelIndex& parent = {}) const override;

    /*! \brief Implementation of QAbstractListModel::data
     *
     *  @param index Model index
     *
     *  @param role Role, see enum Role
     *
     *  @returns Data for the role
     */
    [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;

    /*! \brief Implementation of QAbstractListModel::roleNames
     *
     *  @returns Role names "waypoint", "name" and "searchKey"
     */
    [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

private:
    Q_DISABLE_COPY_MOVE(WaypointSearchModel)

    // Replaces the map rows with the current aviation data
    void resetMapWaypoints();

    // Number of rows taken by the waypoint library
    [[nodiscard]] static int libraryRowCount();

    // Snapshots of GeoMapProvider::waypoints() and waypointSearchKeys(),
    // implicitly shared with the provider
    QList<GeoMaps::Waypoint> m_mapWaypoints;
    QStringList m_mapSearchKeys;
};

} // namespace Ui
