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

#include "GlobalObject.h"
#include "Librarian.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/WaypointLibrary.h"
#include "ui/WaypointSearchModel.h"


Ui::WaypointSearchModel::WaypointSearchModel(QObject* parent)
    : QAbstractListModel(parent)
{
    // Library rows come first, so library row numbers are our row numbers and
    // its row-granular signals can be forwarded without an offset.
    auto* library = GlobalObject::waypointLibrary();
    connect(library, &QAbstractItemModel::rowsAboutToBeInserted, this, [this](const QModelIndex&, int first, int last) { beginInsertRows({}, first, last); });
    connect(library, &QAbstractItemModel::rowsInserted, this, [this]() { endInsertRows(); });
    connect(library, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex&, int first, int last) { beginRemoveRows({}, first, last); });
    connect(library, &QAbstractItemModel::rowsRemoved, this, [this]() { endRemoveRows(); });
    connect(library, &QAbstractItemModel::dataChanged, this, [this](const QModelIndex& topLeft, const QModelIndex& bottomRight) {
        emit dataChanged(index(topLeft.row()), index(bottomRight.row()), {WaypointRole, NameRole, SearchKeyRole});
    });
    connect(library, &QAbstractItemModel::modelAboutToBeReset, this, [this]() { beginResetModel(); });
    connect(library, &QAbstractItemModel::modelReset, this, [this]() { endResetModel(); });

    // The aviation data is replaced as a whole, so the map rows are reset.
    auto* provider = GlobalObject::geoMapProvider();
    connect(provider, &GeoMaps::GeoMapProvider::waypointsChanged, this, &WaypointSearchModel::resetMapWaypoints);
    m_mapWaypoints = provider->waypoints();
    m_mapSearchKeys = provider->waypointSearchKeys();
}


//
// Methods
//

int Ui::WaypointSearchModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return libraryRowCount() + static_cast<int>(m_mapWaypoints.size());
}


QVariant Ui::WaypointSearchModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto row = index.row();
    const auto libraryRows = libraryRowCount();
    if (row < libraryRows)
    {
        auto* library = GlobalObject::waypointLibrary();
        const auto waypoint = library->data(library->index(row), GeoMaps::WaypointLibrary::WaypointRole).value<GeoMaps::Waypoint>();
        switch (role)
        {
        case WaypointRole:
            return QVariant::fromValue(waypoint);
        case NameRole:
        case Qt::DisplayRole:
            return waypoint.name();
        case SearchKeyRole:
            return Librarian::simplify(waypoint.name()) + u' ' + Librarian::simplify(waypoint.ICAOCode());
        default:
            return {};
        }
    }

    const auto mapRow = row - libraryRows;
    if (mapRow >= m_mapWaypoints.size())
    {
        return {};
    }
    switch (role)
    {
    case WaypointRole:
        return QVariant::fromValue(m_mapWaypoints.at(mapRow));
    case NameRole:
    case Qt::DisplayRole:
        return m_mapWaypoints.at(mapRow).name();
    case SearchKeyRole:
        return m_mapSearchKeys.value(mapRow);
    default:
        return {};
    }
}


QHash<int, QByteArray> Ui::WaypointSearchModel::roleNames() const
{
    return {{WaypointRole, "waypoint"}, {NameRole, "name"}, {SearchKeyRole, "searchKey"}};
}


//
// Private Methods
//

void Ui::WaypointSearchModel::resetMapWaypoints()
{
    beginResetModel();
    auto* provider = GlobalObject::geoMapProvider();
    m_mapWaypoints = provider->waypoints();
    m_mapSearchKeys = provider->waypointSearchKeys();
    endResetModel();
}


int Ui::WaypointSearchModel::libraryRowCount()
{
    return GlobalObject::waypointLibrary()->rowCount();
}
