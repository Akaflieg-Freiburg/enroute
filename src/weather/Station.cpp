/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include "weather/Station.h"
#include "weather/WeatherDataProvider.h"


Weather::Station::Station()
{
    // Setup Bindings
    m_metar.setBinding([this]() {return GlobalObject::weatherDataProvider()->METARs()[m_waypoint.ICAOCode()];});
    m_taf.setBinding([this]() {return GlobalObject::weatherDataProvider()->TAFs()[m_waypoint.ICAOCode()];});
}


Weather::Station::Station(const GeoMaps::Waypoint& wp)
    : m_waypoint(wp)
{
    // Setup Bindings
    m_metar.setBinding([this]() {qWarning() << "UPDATE" << m_waypoint.ICAOCode(); return GlobalObject::weatherDataProvider()->METARs()[m_waypoint.ICAOCode()];});
    m_taf.setBinding([this]() {return GlobalObject::weatherDataProvider()->TAFs()[m_waypoint.ICAOCode()];});
}


Weather::Station::Station(const Weather::Station& other)
{
    m_waypoint = other.m_waypoint;

    // Setup Bindings
    m_metar.setBinding([this]() {return GlobalObject::weatherDataProvider()->METARs()[m_waypoint.ICAOCode()];});
    m_taf.setBinding([this]() {return GlobalObject::weatherDataProvider()->TAFs()[m_waypoint.ICAOCode()];});
}


Weather::Station& Weather::Station::operator=(const Weather::Station& other)
{
    m_waypoint = other.m_waypoint;

    // Setup Bindings
    m_metar.setBinding([this]() {return GlobalObject::weatherDataProvider()->METARs()[m_waypoint.ICAOCode()];});
    m_taf.setBinding([this]() {return GlobalObject::weatherDataProvider()->TAFs()[m_waypoint.ICAOCode()];});

    return *this;
}
