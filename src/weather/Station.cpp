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
#include "weather/WeatherDataProvider.h"
#include <utility>


Weather::Station::Station()
{
}


Weather::Station::Station(QString id, GeoMaps::GeoMapProvider *geoMapProvider)
    : m_ICAOCode(std::move(id)),
    m_twoLineTitle(m_ICAOCode),
    m_geoMapProvider(geoMapProvider)
{
    m_extendedName = m_ICAOCode;

    // Setup Bindings
    m_metar.setBinding([this]() {return GlobalObject::weatherDataProvider()->METARs()[m_ICAOCode];});
    m_taf.setBinding([this]() {return GlobalObject::weatherDataProvider()->TAFs()[m_ICAOCode];});

    // Wire up with GeoMapProvider, in order to learn about future changes in waypoints
    //connect(m_geoMapProvider, &GeoMaps::GeoMapProvider::waypointsChanged, this, &Weather::Station::readDataFromWaypoint);
    readDataFromWaypoint();
}


Weather::Station::Station(const Weather::Station& other)
{
    m_coordinate = other.m_coordinate.value();
    m_extendedName = other.m_extendedName.value();
    m_ICAOCode = other.m_ICAOCode;
    m_icon = other.m_icon.value();
    m_twoLineTitle = other.m_twoLineTitle.value();
    m_geoMapProvider = other.m_geoMapProvider;

    // Setup Bindings
    m_metar.setBinding([this]() {return GlobalObject::weatherDataProvider()->METARs()[m_ICAOCode];});
    m_taf.setBinding([this]() {return GlobalObject::weatherDataProvider()->TAFs()[m_ICAOCode];});
}


Weather::Station& Weather::Station::operator=(const Weather::Station& other)
{
    m_coordinate = other.m_coordinate.value();
    m_extendedName = other.m_extendedName.value();
    m_ICAOCode = other.m_ICAOCode;
    m_icon = other.m_icon.value();
    m_twoLineTitle = other.m_twoLineTitle.value();
    m_geoMapProvider = other.m_geoMapProvider;

    // Setup Bindings
    m_metar.setBinding([this]() {return GlobalObject::weatherDataProvider()->METARs()[m_ICAOCode];});
    m_taf.setBinding([this]() {return GlobalObject::weatherDataProvider()->TAFs()[m_ICAOCode];});

    return *this;
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

    //disconnect(m_geoMapProvider, nullptr, this, nullptr);
}
