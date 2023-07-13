/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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
#include "units/Pressure.h"
#include "units/Temperature.h"


namespace Navigation {

/*! \brief Atmospherical data
 *
 *  This class provides the standard computation routines for the ICAO atmosphere, for the height range of 0-11km
 *
 */

class Atmosphere
{
public:
    /*! \brief Computation of height as a function of pressure
     *
     *  @param pressure Air pressure
     *
     *  @returns Barometric height above the 1013.25 hPa level (which equals 0 Meter)
     *
     */
    static Units::Distance height(Units::Pressure pressure);

    /*! \brief Computation of pressure as a function of altitude
     *
     *  @param height Barometric height above the 1013.25 hPa level (which equals 0 Meter)
     *
     *  @returns Air pressure
     *
     */
    static Units::Pressure pressure(Units::Distance height);
};

} // namespace Navigation
