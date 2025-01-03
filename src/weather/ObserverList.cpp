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

#include "weather/ObserverList.h"
#include "weather/WeatherDataProvider.h"
#include "geomaps/GeoMapProvider.h"
#include "positioning/PositionProvider.h"


Weather::ObserverList::ObserverList(QObject* parent)
    : QObject(parent)
{
    m_unsortedObservers.setBinding([this]() {
        QSet<QString> ids;
        QList<Weather::Observer*> result;

        for(const auto& key : GlobalObject::weatherDataProvider()->METARs().keys() + GlobalObject::weatherDataProvider()->TAFs().keys())
        {
            if (ids.contains(key))
            {
                continue;
            }
            ids << key;
            if (m_observersByID.contains(key))
            {
                result << m_observersByID[key];
                continue;
            }
            auto wp = GlobalObject::geoMapProvider()->findByID(key);
            if (!wp.isValid())
            {
                continue;
            }
            auto* obs = new Observer(this);
            obs->setWaypoint(wp);
            m_observersByID[key] = obs;
            result << obs;
        }
        return result;
    });

    m_observers.setBinding([this]() {
        auto tmp = m_unsortedObservers.value();
        auto here = Positioning::PositionProvider::lastValidCoordinate();
        auto compare = [here](const Weather::Observer* a, const Weather::Observer* b) {
            return here.distanceTo(a->waypoint().coordinate()) < here.distanceTo(b->waypoint().coordinate());
        };
        std::sort(tmp.begin(), tmp.end(), compare);
        return tmp;
    });
}
