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


// Format a duration in seconds as "H:MM"; returns {} for negative values
static auto secsToHHMM(qint64 secs) -> QString
{
    if (secs < 0) {
        return {};
    }
    auto hours = secs / 3600;
    auto minutes = (secs % 3600) / 60;
    return u"%1:%2"_s.arg(hours).arg(minutes, 2, 10, QChar(u'0'));
}


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


auto Flightlog::Flight::blockTime() const -> QString
{
    if (!m_offBlockTime.isValid() || !m_onBlockTime.isValid()) {
        return {};
    }
    return secsToHHMM(m_offBlockTime.secsTo(m_onBlockTime));
}


auto Flightlog::Flight::flightTime() const -> QString
{
    return secsToHHMM(flightTimeSeconds());
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
        && m_landingCount == other.m_landingCount
        && m_trackFile == other.m_trackFile
        && m_departureCoordinate == other.m_departureCoordinate
        && m_arrivalCoordinate == other.m_arrivalCoordinate;
}



