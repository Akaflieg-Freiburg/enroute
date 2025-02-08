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

#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficObserver.h"


Traffic::TrafficObserver::TrafficObserver(QObject* parent)
    : QObject(parent)
{
    m_traffic.setBinding([this]() {
        QList<Traffic::TrafficFactor_Abstract*> result;
        const auto list = GlobalObject::trafficDataProvider()->trafficObjects();
        for(const auto* factor : list)
        {
            if (factor == nullptr)
            {
                continue;
            }
            if (!factor->valid())
            {
                continue;
            }
            result << (Traffic::TrafficFactor_Abstract*)factor;
        }
        const auto* factor = GlobalObject::trafficDataProvider()->trafficObjectWithoutPosition();
        if ((factor != nullptr) && factor->valid())
        {
            result << (Traffic::TrafficFactor_Abstract*)factor;
        }

        std::sort(result.begin(), result.end(),
                  [](const Traffic::TrafficFactor_Abstract* first, const Traffic::TrafficFactor_Abstract* second)
                  { return first->hasHigherPriorityThan(*second); });
        return result;
    });
}
