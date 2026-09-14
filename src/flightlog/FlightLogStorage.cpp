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

#include <QFile>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>

#include "flightlog/FlightLogStorage.h"

using namespace Qt::Literals::StringLiterals;


namespace {

// Single source of truth for the 'flights' table schema. The CREATE TABLE,
// INSERT, and SELECT statements are all generated from this list, and
// bindFlight()/loadAll() bind/read by column name (not position) — so unlike
// four independently hand-maintained lists, these can no longer drift apart.
// NOTE: there is currently no migration path. CREATE TABLE IF NOT EXISTS is a
// no-op against an existing file, so adding/removing/reordering a column here
// only takes effect for brand-new databases — existing on-disk tables keep
// their old columns, and SELECT will then fail on a column that doesn't
// exist. Until a migration step (e.g. PRAGMA user_version + ALTER TABLE) is
// added, changing this list is only safe before the schema has shipped.
struct ColumnDef
{
    const char* name;
    const char* sqlType;
};

constexpr ColumnDef flightColumns[] = {
    {"uuid", "TEXT PRIMARY KEY"},
    {"departureICAO", "TEXT"},
    {"arrivalICAO", "TEXT"},
    {"offBlockTime", "TEXT"},
    {"startTime", "TEXT"},
    {"landingTime", "TEXT"},
    {"onBlockTime", "TEXT"},
    {"pilotName", "TEXT"},
    {"aircraftCallsign", "TEXT"},
    {"comments", "TEXT"},
    {"landingCount", "INTEGER"},
    {"departureLat", "REAL"},
    {"departureLon", "REAL"},
    {"arrivalLat", "REAL"},
    {"arrivalLon", "REAL"},
    {"trackFile", "TEXT"},
};

QString createTableStatement()
{
    QStringList columns;
    for (const auto& col : flightColumns) {
        columns << QString::fromLatin1(col.name) + u" "_s + QString::fromLatin1(col.sqlType);
    }
    return u"CREATE TABLE IF NOT EXISTS flights ("_s + columns.join(u", "_s) + u")"_s;
}

QString selectStatement()
{
    QStringList columns;
    for (const auto& col : flightColumns) {
        columns << QString::fromLatin1(col.name);
    }
    return u"SELECT "_s + columns.join(u", "_s) + u" FROM flights"_s;
}

// Insert a row, or overwrite it in place if its uuid already exists.
QString upsertStatement()
{
    QStringList columns;
    QStringList placeholders;
    QStringList updates;
    for (const auto& col : flightColumns) {
        const auto name = QString::fromLatin1(col.name);
        columns << name;
        placeholders << u":"_s + name;
        if (name != u"uuid"_s) {
            updates << name + u"=excluded."_s + name;
        }
    }
    return u"INSERT INTO flights ("_s + columns.join(u", "_s) + u") VALUES ("_s + placeholders.join(u", "_s) + u") "
           u"ON CONFLICT(uuid) DO UPDATE SET "_s + updates.join(u", "_s);
}

} // namespace


Flightlog::FlightLogStorage::FlightLogStorage(QObject* parent)
    : QObject(parent)
    , m_databaseConnectionName(u"Flightlog::FlightLogStorage %1"_s.arg(QRandomGenerator::global()->generate()))
{
    openDatabase();
}


Flightlog::FlightLogStorage::~FlightLogStorage()
{
    QSqlDatabase::database(m_databaseConnectionName).close();
    QSqlDatabase::removeDatabase(m_databaseConnectionName);
}


void Flightlog::FlightLogStorage::openDatabase()
{
    bool opened = false;
    QString reason;
    {
        auto db = QSqlDatabase::addDatabase(u"QSQLITE"_s, m_databaseConnectionName);
        db.setDatabaseName(m_dbFileName);
        opened = db.open();
        if (!opened) {
            reason = db.lastError().text();
        }
    } // db destroyed here, so removeDatabase() below is not called while a
      // QSqlDatabase referring to this connection is still alive.

    if (!opened) {
        // The file exists but is not a valid SQLite database. Quarantine it
        // and start fresh rather than failing permanently.
        QSqlDatabase::removeDatabase(m_databaseConnectionName);
        quarantineFile(m_dbFileName, reason);

        auto db = QSqlDatabase::addDatabase(u"QSQLITE"_s, m_databaseConnectionName);
        db.setDatabaseName(m_dbFileName);
        if (!db.open()) {
            reportError(db.lastError().text());
            return;
        }
    }

    auto db = QSqlDatabase::database(m_databaseConnectionName);
    QSqlQuery query(db);
    if (!query.exec(createTableStatement())) {
        reportError(query.lastError().text());
    }
}


void Flightlog::FlightLogStorage::quarantineFile(const QString& fileName, const QString& reason)
{
    const QString ts = QDateTime::currentDateTimeUtc().toString(u"yyyyMMddTHHmmssZ"_s);
    const QString backup = fileName + u"."_s + ts + u".corrupt"_s;
    QFile::rename(fileName, backup);
    qWarning() << "FlightLogStorage:" << reason << "- damaged file renamed to" << backup;
    reportError(tr("The flight log file could not be read and has been reset (%1). "
                   "Your previous flight log data is no longer available.")
               .arg(reason));
}


void Flightlog::FlightLogStorage::reportError(const QString& message)
{
    // Queued so it's safe to call from the constructor: FlightLog connects to
    // this signal in its own constructor body, which runs after this object
    // (a member) has been fully constructed but before the queued event is
    // dispatched.
    QMetaObject::invokeMethod(this, [this, message]() {
        emit saveError(message);
    }, Qt::QueuedConnection);
}


void Flightlog::FlightLogStorage::bindFlight(QSqlQuery& query, const Flight& flight)
{
    query.bindValue(u":uuid"_s, flight.uuid().toString(QUuid::WithoutBraces));
    query.bindValue(u":departureICAO"_s, flight.departureICAO());
    query.bindValue(u":arrivalICAO"_s, flight.arrivalICAO());
    query.bindValue(u":offBlockTime"_s, flight.offBlockTime().isValid() ? flight.offBlockTime().toUTC().toString(Qt::ISODate) : QString());
    query.bindValue(u":startTime"_s, flight.startTime().isValid() ? flight.startTime().toUTC().toString(Qt::ISODate) : QString());
    query.bindValue(u":landingTime"_s, flight.landingTime().isValid() ? flight.landingTime().toUTC().toString(Qt::ISODate) : QString());
    query.bindValue(u":onBlockTime"_s, flight.onBlockTime().isValid() ? flight.onBlockTime().toUTC().toString(Qt::ISODate) : QString());
    query.bindValue(u":pilotName"_s, flight.pilotName());
    query.bindValue(u":aircraftCallsign"_s, flight.aircraftCallsign());
    query.bindValue(u":comments"_s, flight.comments());
    query.bindValue(u":landingCount"_s, flight.landingCount());
    if (flight.departureCoordinate().isValid()) {
        query.bindValue(u":departureLat"_s, flight.departureCoordinate().latitude());
        query.bindValue(u":departureLon"_s, flight.departureCoordinate().longitude());
    } else {
        query.bindValue(u":departureLat"_s, QVariant());
        query.bindValue(u":departureLon"_s, QVariant());
    }
    if (flight.arrivalCoordinate().isValid()) {
        query.bindValue(u":arrivalLat"_s, flight.arrivalCoordinate().latitude());
        query.bindValue(u":arrivalLon"_s, flight.arrivalCoordinate().longitude());
    } else {
        query.bindValue(u":arrivalLat"_s, QVariant());
        query.bindValue(u":arrivalLon"_s, QVariant());
    }
    query.bindValue(u":trackFile"_s, flight.trackFile());
}


auto Flightlog::FlightLogStorage::loadAll() -> QList<Flight>
{
    QList<Flight> result;

    auto db = QSqlDatabase::database(m_databaseConnectionName);
    if (!db.isOpen()) {
        reportError(tr("The flight log database is not open."));
        return result;
    }

    QSqlQuery query(db);
    if (!query.exec(selectStatement())) {
        reportError(query.lastError().text());
        return result;
    }

    while (query.next()) {
        Flight f(QUuid::fromString(query.value(u"uuid"_s).toString()));
        f.setDepartureICAO(query.value(u"departureICAO"_s).toString());
        f.setArrivalICAO(query.value(u"arrivalICAO"_s).toString());

        const auto offBlockTimeStr = query.value(u"offBlockTime"_s).toString();
        if (!offBlockTimeStr.isEmpty()) {
            f.setOffBlockTime(QDateTime::fromString(offBlockTimeStr, Qt::ISODate));
        }
        const auto startTimeStr = query.value(u"startTime"_s).toString();
        if (!startTimeStr.isEmpty()) {
            f.setStartTime(QDateTime::fromString(startTimeStr, Qt::ISODate));
        }
        const auto landingTimeStr = query.value(u"landingTime"_s).toString();
        if (!landingTimeStr.isEmpty()) {
            f.setLandingTime(QDateTime::fromString(landingTimeStr, Qt::ISODate));
        }
        const auto onBlockTimeStr = query.value(u"onBlockTime"_s).toString();
        if (!onBlockTimeStr.isEmpty()) {
            f.setOnBlockTime(QDateTime::fromString(onBlockTimeStr, Qt::ISODate));
        }

        f.setPilotName(query.value(u"pilotName"_s).toString());
        f.setAircraftCallsign(query.value(u"aircraftCallsign"_s).toString());
        f.setComments(query.value(u"comments"_s).toString());
        f.setLandingCount(query.value(u"landingCount"_s).toInt());

        if (!query.value(u"departureLat"_s).isNull() && !query.value(u"departureLon"_s).isNull()) {
            f.setDepartureCoordinate(QGeoCoordinate(query.value(u"departureLat"_s).toDouble(), query.value(u"departureLon"_s).toDouble()));
        }
        if (!query.value(u"arrivalLat"_s).isNull() && !query.value(u"arrivalLon"_s).isNull()) {
            f.setArrivalCoordinate(QGeoCoordinate(query.value(u"arrivalLat"_s).toDouble(), query.value(u"arrivalLon"_s).toDouble()));
        }

        // Path-traversal guard against untrusted trackFile values lives in
        // Flight::setTrackFile() itself.
        f.setTrackFile(query.value(u"trackFile"_s).toString());

        result.append(std::move(f));
    }

    return result;
}


bool Flightlog::FlightLogStorage::upsert(const Flight& flight)
{
    auto db = QSqlDatabase::database(m_databaseConnectionName);
    if (!db.isOpen()) {
        reportError(tr("The flight log database is not open."));
        return false;
    }

    QSqlQuery query(db);
    query.prepare(upsertStatement());
    bindFlight(query, flight);
    if (!query.exec()) {
        reportError(query.lastError().text());
        return false;
    }
    return true;
}


bool Flightlog::FlightLogStorage::upsertMany(const QList<Flight>& flights)
{
    if (flights.isEmpty()) {
        return true;
    }

    auto db = QSqlDatabase::database(m_databaseConnectionName);
    if (!db.isOpen()) {
        reportError(tr("The flight log database is not open."));
        return false;
    }

    if (!db.transaction()) {
        reportError(db.lastError().text());
        return false;
    }

    QSqlQuery query(db);
    query.prepare(upsertStatement());
    for (const auto& flight : flights) {
        bindFlight(query, flight);
        if (!query.exec()) {
            db.rollback();
            reportError(query.lastError().text());
            return false;
        }
    }

    if (!db.commit()) {
        db.rollback();
        reportError(db.lastError().text());
        return false;
    }
    return true;
}


bool Flightlog::FlightLogStorage::remove(const QUuid& uuid)
{
    auto db = QSqlDatabase::database(m_databaseConnectionName);
    if (!db.isOpen()) {
        reportError(tr("The flight log database is not open."));
        return false;
    }

    QSqlQuery query(db);
    query.prepare(u"DELETE FROM flights WHERE uuid = ?"_s);
    query.addBindValue(uuid.toString(QUuid::WithoutBraces));
    if (!query.exec()) {
        reportError(query.lastError().text());
        return false;
    }
    return true;
}


bool Flightlog::FlightLogStorage::removeMany(const QList<QUuid>& uuids)
{
    if (uuids.isEmpty()) {
        return true;
    }

    auto db = QSqlDatabase::database(m_databaseConnectionName);
    if (!db.isOpen()) {
        reportError(tr("The flight log database is not open."));
        return false;
    }

    if (!db.transaction()) {
        reportError(db.lastError().text());
        return false;
    }

    QSqlQuery query(db);
    query.prepare(u"DELETE FROM flights WHERE uuid = ?"_s);
    for (const auto& uuid : uuids) {
        query.addBindValue(uuid.toString(QUuid::WithoutBraces));
        if (!query.exec()) {
            db.rollback();
            reportError(query.lastError().text());
            return false;
        }
    }

    if (!db.commit()) {
        db.rollback();
        reportError(db.lastError().text());
        return false;
    }
    return true;
}


bool Flightlog::FlightLogStorage::removeAll()
{
    auto db = QSqlDatabase::database(m_databaseConnectionName);
    if (!db.isOpen()) {
        reportError(tr("The flight log database is not open."));
        return false;
    }

    QSqlQuery query(db);
    if (!query.exec(u"DELETE FROM flights"_s)) {
        reportError(query.lastError().text());
        return false;
    }
    return true;
}
