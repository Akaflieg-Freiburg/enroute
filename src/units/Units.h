/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#pragma once

#include "units/Distance.h"
#include "units/Time.h"

//
// Operations
//


/*! \brief Compute speed as quotient of distance by time
 *
 *  @param dist Distance
 *
 *  @param time Time
 *
 *  @return Speed
 */
inline Units::Speed operator/(Units::Distance dist, Units::Time time)
{
    if ((!dist.isFinite()) || (!time.isFinite()) || (qFuzzyIsNull(time.toS())))
        return {};

    return Units::Speed::fromMPS(dist.toM()/time.toS());
}


/*! \brief Compute time as quotient of distance and speed
 *
 * @param dist Distance
 *
 * @param speed Speed
 *
 * @returns Time
 */
inline Units::Time operator/(Units::Distance dist, Units::Speed speed)
{
    if ((!dist.isFinite()) || (!speed.isFinite()) || (qFuzzyIsNull(speed.toMPS())))
        return {};

    return Units::Time::fromS(dist.toM() / speed.toMPS());
}
