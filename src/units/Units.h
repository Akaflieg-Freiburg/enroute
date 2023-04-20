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
#include "units/Timespan.h"
#include "units/Volume.h"
#include "units/VolumeFlow.h"

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
inline auto operator/(Units::Distance dist, Units::Timespan time) -> Units::Speed
{
    if ((!dist.isFinite()) || (!time.isFinite()) || (qFuzzyIsNull(time.toS()))) {
        return {};
}

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
inline auto operator/(Units::Distance dist, Units::Speed speed) -> Units::Timespan
{
    if ((!dist.isFinite()) || (!speed.isFinite()) || (qFuzzyIsNull(speed.toMPS()))) {
        return {};
}

    return Units::Timespan::fromS(dist.toM() / speed.toMPS());
}


/*! \brief Compute volume as product of volumeFlow and time
 *
 * @param volumeFlow Volume Flow
 *
 * @param time Time
 *
 * @returns Volume
 */
inline auto operator*(Units::VolumeFlow volumeFlow, Units::Timespan time) -> Units::Volume
{
    if (!volumeFlow.isFinite() || !time.isFinite()) {
        return {};
}

    return Units::Volume::fromL( volumeFlow.toLPH()*time.toH() );
}
