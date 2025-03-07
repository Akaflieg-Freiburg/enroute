/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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

#include "weather/Observer.h"
#include "weather/WeatherDataProvider.h"


Weather::Observer::Observer(QObject* parent)
    : QObject(parent)
{
    // Setup Bindings
    m_metar.setBinding([this]() {
        auto id = m_waypoint.value().ICAOCode();
        return GlobalObject::weatherDataProvider()->METARs().value(id);
    });
    m_taf.setBinding([this]() {
        auto id = m_waypoint.value().ICAOCode();
        return GlobalObject::weatherDataProvider()->TAFs().value(id);
    });
}

