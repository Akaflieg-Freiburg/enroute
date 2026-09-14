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

#include <QObject>
#include <QSqlQuery>
#include <QStandardPaths>

#include "flightlog/Flight.h"

using namespace Qt::Literals::StringLiterals;

namespace Flightlog {

/*! \brief SQLite-backed storage for the flight log
 *
 *  Owns the app's user-data SQLite database (userdata.db, shared with other
 *  user data that may be added later, e.g. an aircraft list) and persists
 *  the 'flights' table within it.
 *
 *  Per-operation persistence: each method here writes only the row(s) it
 *  names, rather than reconciling the whole table against an in-memory list.
 *  FlightLog still keeps its own in-memory QList<Flight> (the app's dataset
 *  is small enough that this costs nothing, and most of FlightLog's logic —
 *  sorting, uuid lookups, export selection — is naturally list-shaped); this
 *  class exists purely to keep the on-disk copy in sync with each individual
 *  change as it happens.
 */
class FlightLogStorage : public QObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     *  Opens (creating if necessary) the userdata.db SQLite database and
     *  ensures the 'flights' table schema exists.
     *
     *  @param parent The standard QObject parent pointer
     */
    explicit FlightLogStorage(QObject* parent);

    /*! \brief Standard destructor */
    ~FlightLogStorage() override;

    /*! \brief Load all flights from the database
     *
     *  @returns All flights currently stored, in unspecified order
     */
    [[nodiscard]] auto loadAll() -> QList<Flight>;

    /*! \brief Insert a flight, or update it if its uuid already exists
     *
     *  @param flight The flight to persist
     *  @returns True on success
     */
    bool upsert(const Flight& flight);

    /*! \brief Insert or update multiple flights, in one transaction
     *
     *  @param flights The flights to persist
     *  @returns True on success
     */
    bool upsertMany(const QList<Flight>& flights);

    /*! \brief Delete a flight by uuid
     *
     *  Does nothing (and still returns true) if no row with this uuid exists.
     *
     *  @param uuid The uuid of the flight to delete
     *  @returns True on success
     */
    bool remove(const QUuid& uuid);

    /*! \brief Delete multiple flights by uuid, in one transaction
     *
     *  @param uuids The uuids of the flights to delete
     *  @returns True on success
     */
    bool removeMany(const QList<QUuid>& uuids);

    /*! \brief Delete every flight
     *
     *  @returns True on success
     */
    bool removeAll();

signals:
    /*! \brief Emitted when a database operation fails
     *
     *  Always delivered via a queued connection, so it is safe to emit
     *  from the constructor (before any signal connections exist yet).
     *
     *  @param message Human-readable error description
     */
    void saveError(const QString& message);

private:
    Q_DISABLE_COPY_MOVE(FlightLogStorage)

    // Open the database connection and ensure the schema exists
    void openDatabase();

    // Rename a corrupt file aside with a timestamp suffix and report the error
    void quarantineFile(const QString& fileName, const QString& reason);

    // Emit saveError() via a queued connection, safe to call from the constructor.
    // Const so it can also be called from loadAll(); emitting a signal doesn't
    // change any observable state of this object.
    void reportError(const QString& message);

    // Bind all fields of a flight to a prepared INSERT query, in column order
    static void bindFlight(QSqlQuery& query, const Flight& flight);

    QString m_databaseConnectionName;

    const QString m_dbFileName {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/userdata.db"_s};
};

} // namespace Flightlog
