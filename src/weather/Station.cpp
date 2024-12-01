/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include "geomaps/GeoMapProvider.h"
#include "weather/Station.h"
#include <utility>


Weather::Station::Station(QObject *parent)
    : QObject(parent)
{
}


Weather::Station::Station(QString id, GeoMaps::GeoMapProvider *geoMapProvider, QObject *parent)
    : QObject(parent),
    m_ICAOCode(std::move(id)),
    m_twoLineTitle(m_ICAOCode),
    m_geoMapProvider(geoMapProvider)
{
    m_extendedName = m_ICAOCode;

    // Wire up with GeoMapProvider, in order to learn about future changes in waypoints
    connect(m_geoMapProvider, &GeoMaps::GeoMapProvider::waypointsChanged, this, &Weather::Station::readDataFromWaypoint);
    readDataFromWaypoint();
}


void Weather::Station::readDataFromWaypoint()
{
    // Paranoid safety checks
    if (m_geoMapProvider.isNull()) {
        return;
    }
    auto waypoint = m_geoMapProvider->findByID(m_ICAOCode);
    if (!waypoint.isValid())
    {
        return;
    }

    // Update data
    m_coordinate = waypoint.coordinate();
    m_extendedName = waypoint.extendedName();
    m_icon = waypoint.icon();
    m_twoLineTitle = waypoint.twoLineTitle();

    disconnect(m_geoMapProvider, nullptr, this, nullptr);
}


void Weather::Station::setMETAR(const Weather::METAR& metar)
{
    // Ignore invalid and expired METARs. Also ignore METARs whose ICAO code does not match with this weather station
    if (!metar.isValid() || (QDateTime::currentDateTime() > metar.expiration()) || (metar.ICAOCode() != m_ICAOCode))
    {
        return;
    }

    // Overwrite metar pointer
    m_metar = metar;

    // Update coordinate
    if (!m_coordinate.value().isValid())
    {
        m_coordinate = m_metar.value().coordinate();
    }
}


void Weather::Station::setTAF(const Weather::TAF& taf)
{
    // Ignore invalid and expired TAFs. Also ignore TAFs whose ICAO code does not match with this weather station
    if (!taf.isValid() || (QDateTime::currentDateTime() > taf.expiration()) || (taf.ICAOCode() != m_ICAOCode))
    {
        return;
    }

    // Overwrite TAF pointer
    m_taf = taf;

    // Update coordinate
    if (!m_coordinate.value().isValid())
    {
        m_coordinate = m_taf.value().coordinate();
    }
}
