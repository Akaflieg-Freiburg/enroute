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

#include <algorithm>

#include "Librarian.h"
#include "flightlog/FlightLogModel.h"


Flightlog::FlightLogModel::FlightLogModel(QObject* parent)
    : QAbstractListModel(parent)
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
}


//
// Getter Methods
//

auto Flightlog::FlightLogModel::row(const QUuid& uuid) const -> int
{
    const auto it = std::ranges::find_if(m_flights, [&](const Flight& flight) {
        return flight.uuid() == uuid;
    });
    if (it == m_flights.end()) {
        return -1;
    }
    return static_cast<int>(std::distance(m_flights.begin(), it));
}


//
// Model API
//

auto Flightlog::FlightLogModel::rowCount(const QModelIndex& parent) const -> int
{
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(m_flights.size());
}


auto Flightlog::FlightLogModel::data(const QModelIndex& index, int role) const -> QVariant
{
    if (!index.isValid() || (index.row() < 0) || (index.row() >= m_flights.size())) {
        return {};
    }

    const auto& flight = m_flights.at(index.row());
    switch (role) {
    case FlightRole:
        return QVariant::fromValue(flight);
    case SearchKeyRole:
        return Librarian::simplify(flight.departureICAO()) + u' '
            + Librarian::simplify(flight.arrivalICAO()) + u' '
            + Librarian::simplify(flight.pilotName()) + u' '
            + Librarian::simplify(flight.aircraftCallsign());
    case UuidRole:
        return flight.uuidString();
    default:
        return {};
    }
}


auto Flightlog::FlightLogModel::roleNames() const -> QHash<int, QByteArray>
{
    return {{FlightRole, "flight"}, {SearchKeyRole, "searchKey"}, {UuidRole, "uuid"}};
}


//
// Methods
//

void Flightlog::FlightLogModel::add(const Flight& flight)
{
    // First row whose flight is not newer than the given one
    const auto it = std::ranges::lower_bound(m_flights, flight, &FlightLogModel::newerThan);
    const auto newRow = static_cast<int>(std::distance(m_flights.begin(), it));

    beginInsertRows({}, newRow, newRow);
    m_flights.insert(newRow, flight);
    endInsertRows();
}


void Flightlog::FlightLogModel::replace(int row, const Flight& flight)
{
    if ((row < 0) || (row >= m_flights.size())) {
        return;
    }

    if (m_flights.at(row).startTime() == flight.startTime()) {
        // The sorted position does not change
        m_flights[row] = flight;
        const auto idx = index(row);
        emit dataChanged(idx, idx);
        return;
    }

    remove(row);
    add(flight);
}


void Flightlog::FlightLogModel::remove(int row)
{
    if ((row < 0) || (row >= m_flights.size())) {
        return;
    }

    beginRemoveRows({}, row, row);
    m_flights.removeAt(row);
    endRemoveRows();
}


void Flightlog::FlightLogModel::reset(QList<Flight> flights)
{
    std::ranges::stable_sort(flights, &FlightLogModel::newerThan);

    beginResetModel();
    m_flights = std::move(flights);
    endResetModel();
}


//
// Private Methods
//

auto Flightlog::FlightLogModel::newerThan(const Flight& first, const Flight& second) -> bool
{
    return first.startTime() > second.startTime();
}
