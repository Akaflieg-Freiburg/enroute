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
#include <QUuid>

#include "flightlog/Flight.h"

namespace Flightlog {

/*! \brief List of the flights in the flight log
 *
 *  This model holds the flights of the flight log, sorted by start time, with
 *  the newest flight first. Flights without a start time come last. Changes
 *  are announced with row-granular model signals, so that a view keeps its
 *  delegates and its scroll position when a flight is added, edited or
 *  removed.
 *
 *  The instance is owned by FlightLog, see FlightLog::flightsModel. FlightLog
 *  is the only writer; it persists every change before it changes the model.
 *  In QML, bind a view to the model through a NameFilterProxyModel:
 *
 *  @code
 *  ListView {
 *      model: NameFilterProxyModel {
 *          sourceModel: FlightLog.flightsModel
 *          filterRoleName: "searchKey"
 *          filterRoleIsSimplified: true
 *          filter: filterField.filter
 *      }
 *  }
 *  @endcode
 */

class FlightLogModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Use FlightLog.flightsModel")

public:
    /*! \brief Model roles */
    enum Role : int {
        /*! \brief The flight, a Flightlog::Flight */
        FlightRole = Qt::UserRole + 1,

        /*! \brief Text that a filter is matched against, a QString
         *
         *  Holds departure, arrival, pilot name and aircraft callsign, each
         *  simplified with Librarian::simplify() and separated by spaces.
         */
        SearchKeyRole,

        /*! \brief Identity of the flight, a QString as in Flight::uuidString */
        UuidRole
    };
    Q_ENUM(Role)

    /*! \brief Standard constructor
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit FlightLogModel(QObject* parent = nullptr);

    // Default destructor
    ~FlightLogModel() override = default;


    //
    // Getter Methods
    //

    /*! \brief Flights in the log
     *
     *  @returns Flights, in model order
     */
    [[nodiscard]] auto flights() const -> const QList<Flight>& { return m_flights; }

    /*! \brief Row of a flight
     *
     *  @param uuid Identity of the flight
     *
     *  @returns Row of the flight with the given identity, or -1 if the log
     *  does not contain it
     */
    [[nodiscard]] auto row(const QUuid& uuid) const -> int;


    //
    // Model API
    //

    /*! \brief Re-implemented from QAbstractListModel
     *
     *  @param parent Parent index, invalid for the list itself
     *
     *  @returns Number of flights, or zero for a valid parent
     */
    [[nodiscard]] auto rowCount(const QModelIndex& parent = QModelIndex()) const -> int override;

    /*! \brief Re-implemented from QAbstractListModel
     *
     *  @param index Model index
     *
     *  @param role One of the roles in FlightLogModel::Role
     *
     *  @returns Data for the role, or an invalid QVariant
     */
    [[nodiscard]] auto data(const QModelIndex& index, int role = Qt::DisplayRole) const -> QVariant override;

    /*! \brief Re-implemented from QAbstractListModel
     *
     *  @returns Names of the roles in FlightLogModel::Role
     */
    [[nodiscard]] auto roleNames() const -> QHash<int, QByteArray> override;


    //
    // Methods
    //

    /*! \brief Add a flight
     *
     *  The flight is inserted at its sorted position, before the flights with
     *  the same start time. The caller ensures that the log does not contain
     *  a flight with the same identity.
     *
     *  @param flight Flight to add
     */
    void add(const Flight& flight);

    /*! \brief Replace the flight in a row
     *
     *  If the start time is unchanged, the flight keeps its row and the model
     *  emits dataChanged(). Otherwise, the row is removed and the flight is
     *  inserted at its new sorted position. The model does not announce this
     *  as a move of the row: a QSortFilterProxyModel forwards a move as a
     *  layout change, and a QML view answers that by rebuilding itself.
     *
     *  @param row Row of the flight to replace. Does nothing if the row does
     *  not exist.
     *
     *  @param flight New content of the row
     */
    void replace(int row, const Flight& flight);

    /*! \brief Remove the flight in a row
     *
     *  @param row Row of the flight to remove. Does nothing if the row does
     *  not exist.
     */
    void remove(int row);

    /*! \brief Replace all flights
     *
     *  The flights are sorted and the model is reset.
     *
     *  @param flights New content of the model, in any order
     */
    void reset(QList<Flight> flights);

private:
    Q_DISABLE_COPY_MOVE(FlightLogModel)

    // Sort order of the model: start time descending. An invalid start time
    // is earlier than every valid one, so that flights without start time
    // come last.
    static auto newerThan(const Flight& first, const Flight& second) -> bool;

    // Model content, sorted with newerThan()
    QList<Flight> m_flights;
};

} // namespace Flightlog
