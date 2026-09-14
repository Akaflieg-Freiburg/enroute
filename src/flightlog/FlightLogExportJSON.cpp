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

#include <QDateTime>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>

#include "flightlog/FlightLogExportJSON.h"

using namespace Qt::Literals::StringLiterals;


auto Flightlog::FlightLogExportJSON::toJSON(const Flight& flight) -> QJsonObject
{
    QJsonObject json;
    json[u"departureICAO"_s] = flight.departureICAO();
    json[u"arrivalICAO"_s] = flight.arrivalICAO();

    if (flight.offBlockTime().isValid()) {
        json[u"offBlockTime"_s] = flight.offBlockTime().toUTC().toString(Qt::ISODate);
    }
    if (flight.startTime().isValid()) {
        json[u"startTime"_s] = flight.startTime().toUTC().toString(Qt::ISODate);
    }
    if (flight.landingTime().isValid()) {
        json[u"landingTime"_s] = flight.landingTime().toUTC().toString(Qt::ISODate);
    }
    if (flight.onBlockTime().isValid()) {
        json[u"onBlockTime"_s] = flight.onBlockTime().toUTC().toString(Qt::ISODate);
    }
    json[u"pilotName"_s] = flight.pilotName();
    json[u"aircraftCallsign"_s] = flight.aircraftCallsign();
    json[u"comments"_s] = flight.comments();
    if (flight.landingCount() > 0) {
        json[u"landingCount"_s] = flight.landingCount();
    }

    if (flight.departureCoordinate().isValid()) {
        json[u"departureLat"_s] = flight.departureCoordinate().latitude();
        json[u"departureLon"_s] = flight.departureCoordinate().longitude();
    }
    if (flight.arrivalCoordinate().isValid()) {
        json[u"arrivalLat"_s] = flight.arrivalCoordinate().latitude();
        json[u"arrivalLon"_s] = flight.arrivalCoordinate().longitude();
    }

    if (!flight.trackFile().isEmpty()) {
        json[u"trackFile"_s] = flight.trackFile();
    }

    json[u"uuid"_s] = flight.uuid().toString(QUuid::WithoutBraces);

    return json;
}


auto Flightlog::FlightLogExportJSON::fromJSON(const QJsonObject& json) -> Flight
{
    // Restore UUID; generate a fresh one for entries created before this
    // field was added, or if it's malformed.
    auto uuidStr = json.value(u"uuid"_s).toString();
    auto uuid = uuidStr.isEmpty() ? QUuid::createUuid() : QUuid::fromString(uuidStr);
    Flight f(uuid);

    f.setDepartureICAO(json.value(u"departureICAO"_s).toString());
    f.setArrivalICAO(json.value(u"arrivalICAO"_s).toString());

    if (json.contains(u"offBlockTime"_s)) {
        f.setOffBlockTime(QDateTime::fromString(json.value(u"offBlockTime"_s).toString(), Qt::ISODate));
    }
    f.setStartTime(QDateTime::fromString(json.value(u"startTime"_s).toString(), Qt::ISODate));
    f.setLandingTime(QDateTime::fromString(json.value(u"landingTime"_s).toString(), Qt::ISODate));
    if (json.contains(u"onBlockTime"_s)) {
        f.setOnBlockTime(QDateTime::fromString(json.value(u"onBlockTime"_s).toString(), Qt::ISODate));
    }
    f.setPilotName(json.value(u"pilotName"_s).toString());
    f.setAircraftCallsign(json.value(u"aircraftCallsign"_s).toString());
    f.setComments(json.value(u"comments"_s).toString());
    f.setLandingCount(json.value(u"landingCount"_s).toInt(0));

    if (json.contains(u"departureLat"_s) && json.contains(u"departureLon"_s)) {
        f.setDepartureCoordinate(QGeoCoordinate(
            json.value(u"departureLat"_s).toDouble(),
            json.value(u"departureLon"_s).toDouble()));
    }
    if (json.contains(u"arrivalLat"_s) && json.contains(u"arrivalLon"_s)) {
        f.setArrivalCoordinate(QGeoCoordinate(
            json.value(u"arrivalLat"_s).toDouble(),
            json.value(u"arrivalLon"_s).toDouble()));
    }

    // Path-traversal guard against untrusted trackFile values lives in
    // Flight::setTrackFile() itself.
    f.setTrackFile(json.value(u"trackFile"_s).toString());

    return f;
}


auto Flightlog::FlightLogExportJSON::toJSON(const QList<Flight>& flights) -> QByteArray
{
    if (flights.isEmpty()) {
        return {};
    }

    QJsonArray array;
    for (const auto& flight : flights) {
        array.append(toJSON(flight));
    }

    QJsonObject root;
    root[u"content"_s] = u"flightLog"_s;
    root[u"version"_s] = 1;
    root[u"exportDate"_s] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    root[u"flights"_s] = array;

    return QJsonDocument(root).toJson();
}


auto Flightlog::FlightLogExportJSON::fromJSON(const QByteArray& content) -> QList<Flight>
{
    const auto doc = QJsonDocument::fromJson(content);
    if (!doc.isObject() || doc.object().value(u"content"_s).toString() != u"flightLog"_s) {
        return {};
    }

    QList<Flight> flights;
    const auto array = doc.object().value(u"flights"_s).toArray();
    for (const auto& val : array) {
        if (val.isObject()) {
            flights.append(fromJSON(val.toObject()));
        }
    }
    return flights;
}


auto Flightlog::FlightLogExportJSON::isValid(const QString& fileName, QString* info) -> bool
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        if (info != nullptr) {
            *info = file.errorString();
        }
        return false;
    }
    const auto doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isObject() || doc.object().value(u"content"_s).toString() != u"flightLog"_s) {
        if (info != nullptr) {
            *info = QObject::tr("Not a flight log file.", "Flightlog::FlightLogExportJSON");
        }
        return false;
    }
    return true;
}
