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

#include "flightlog/Flight.h"

using namespace Qt::Literals::StringLiterals;


auto Flightlog::Flight::distance() const -> Units::Distance
{
    if (!m_departureCoordinate.isValid() || !m_arrivalCoordinate.isValid()) {
        return {};
    }
    return Units::Distance::fromM(m_departureCoordinate.distanceTo(m_arrivalCoordinate));
}


auto Flightlog::Flight::flightTimeSeconds() const -> qint64
{
    if (!m_startTime.isValid() || !m_landingTime.isValid()) {
        return -1;
    }
    return m_startTime.secsTo(m_landingTime);
}


auto Flightlog::Flight::flightTime() const -> QString
{
    auto secs = flightTimeSeconds();
    if (secs < 0) {
        return {};
    }
    auto hours = secs / 3600;
    auto minutes = (secs % 3600) / 60;
    return u"%1:%2"_s.arg(hours).arg(minutes, 2, 10, QChar(u'0'));
}


auto Flightlog::Flight::toJSON() const -> QJsonObject
{
    QJsonObject json;
    json[u"departureICAO"_s] = m_departureICAO;
    json[u"arrivalICAO"_s] = m_arrivalICAO;

    if (m_offBlockTime.isValid()) {
        json[u"offBlockTime"_s] = m_offBlockTime.toString(Qt::ISODate);
    }
    if (m_startTime.isValid()) {
        json[u"startTime"_s] = m_startTime.toString(Qt::ISODate);
    }
    if (m_landingTime.isValid()) {
        json[u"landingTime"_s] = m_landingTime.toString(Qt::ISODate);
    }
    if (m_onBlockTime.isValid()) {
        json[u"onBlockTime"_s] = m_onBlockTime.toString(Qt::ISODate);
    }
    json[u"pilotName"_s] = m_pilotName;
    json[u"aircraftCallsign"_s] = m_aircraftCallsign;
    json[u"comments"_s] = m_comments;
    if (m_landingCount > 0) {
        json[u"landingCount"_s] = m_landingCount;
    }

    if (m_departureCoordinate.isValid()) {
        json[u"departureLat"_s] = m_departureCoordinate.latitude();
        json[u"departureLon"_s] = m_departureCoordinate.longitude();
    }
    if (m_arrivalCoordinate.isValid()) {
        json[u"arrivalLat"_s] = m_arrivalCoordinate.latitude();
        json[u"arrivalLon"_s] = m_arrivalCoordinate.longitude();
    }

    if (!m_trackFile.isEmpty()) {
        json[u"trackFile"_s] = m_trackFile;
    }

    return json;
}


auto Flightlog::Flight::fromJSON(const QJsonObject& json) -> Flight
{
    Flight f;
    f.m_departureICAO = json.value(u"departureICAO"_s).toString();
    f.m_arrivalICAO = json.value(u"arrivalICAO"_s).toString();

    if (json.contains(u"offBlockTime"_s)) {
        f.m_offBlockTime = QDateTime::fromString(json.value(u"offBlockTime"_s).toString(), Qt::ISODate);
    }
    f.m_startTime = QDateTime::fromString(json.value(u"startTime"_s).toString(), Qt::ISODate);
    f.m_landingTime = QDateTime::fromString(json.value(u"landingTime"_s).toString(), Qt::ISODate);
    if (json.contains(u"onBlockTime"_s)) {
        f.m_onBlockTime = QDateTime::fromString(json.value(u"onBlockTime"_s).toString(), Qt::ISODate);
    }
    f.m_pilotName = json.value(u"pilotName"_s).toString();
    f.m_aircraftCallsign = json.value(u"aircraftCallsign"_s).toString();
    f.m_comments = json.value(u"comments"_s).toString();
    f.m_landingCount = json.value(u"landingCount"_s).toInt(0);

    if (json.contains(u"departureLat"_s) && json.contains(u"departureLon"_s)) {
        f.m_departureCoordinate = QGeoCoordinate(
            json.value(u"departureLat"_s).toDouble(),
            json.value(u"departureLon"_s).toDouble());
    }
    if (json.contains(u"arrivalLat"_s) && json.contains(u"arrivalLon"_s)) {
        f.m_arrivalCoordinate = QGeoCoordinate(
            json.value(u"arrivalLat"_s).toDouble(),
            json.value(u"arrivalLon"_s).toDouble());
    }

    auto trackFile = json.value(u"trackFile"_s).toString();
    if (!trackFile.isEmpty() && !trackFile.contains(u'/') && !trackFile.contains(u'\\')) {
        f.m_trackFile = trackFile;
    }

    return f;
}


auto Flightlog::Flight::operator==(const Flightlog::Flight& other) const -> bool
{
    return m_departureICAO == other.m_departureICAO
        && m_arrivalICAO == other.m_arrivalICAO
        && m_offBlockTime == other.m_offBlockTime
        && m_startTime == other.m_startTime
        && m_landingTime == other.m_landingTime
        && m_onBlockTime == other.m_onBlockTime
        && m_pilotName == other.m_pilotName
        && m_aircraftCallsign == other.m_aircraftCallsign
        && m_comments == other.m_comments
        && m_landingCount == other.m_landingCount;
}



