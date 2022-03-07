/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include <QSettings>

#include "weather/Wind.h"


//
// Setter Methods
//

void Weather::Wind::setSpeed(Units::Speed newSpeed)
{

    if ((newSpeed < minWindSpeed) || (newSpeed > maxWindSpeed)) {
        newSpeed = Units::Speed();
    }

    if (newSpeed == m_speed) {
        return;
    }

    m_speed = newSpeed;
}


void Weather::Wind::setDirectionFrom(Units::Angle newDirectionFrom)
{

    if (newDirectionFrom == m_directionFrom) {
        return;
    }

    m_directionFrom = newDirectionFrom;
}


//
// Methods
//

bool Weather::Wind::operator==(const Weather::Wind& other) const
{
    return (m_speed == other.m_speed) &&
            (m_directionFrom == other.m_directionFrom);
}
