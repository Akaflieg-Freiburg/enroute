/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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


#include <Settings.h>

#include "traffic/TrafficFactor_DistanceOnly.h"


Traffic::TrafficFactor_DistanceOnly::TrafficFactor_DistanceOnly(QObject *parent) : Traffic::TrafficFactor_Abstract(parent)
{  

    // Bindings for property valid
    connect(this, &Traffic::TrafficFactor_DistanceOnly::coordinateChanged, this, &Traffic::TrafficFactor_DistanceOnly::dispatchUpdateValid);

}


void Traffic::TrafficFactor_DistanceOnly::updateValid()
{

    if (!coordinate().isValid()) {
        if (m_valid) {
            m_valid = false;
            emit validChanged();
        }
        return;
    }

    TrafficFactor_Abstract::updateValid();

}
