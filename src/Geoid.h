/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef GEOID_H
#define GEOID_H

#include <QObject>

/*!
 * \brief provide geoid correction 
 *
 * according to https://en.wikipedia.org/wiki/Geoid
 *
 * In maps and common use (like VFR) the height over the mean sea level
 * (such as orthometric height) is used to indicate the height of elevations
 * while the ellipsoidal height results from the GPS system and similar GNSS.
 *
 * The deviation between the ellipsoidal height and the orthometric height
 * is returned here.
 */
class Geoid
{
public:
    /*! \brief return geoid correction -- the difference between AMSL and ellipsoidal height.
     *
     * this operator will basically get the geoid correction by calling
     * de.akaflieg_freiburg.enroute.MobileAdapter.geoid().
     * Getting the geoid correction by parsing NMEA Messages is done there.
     *
     * @returns geoid correction
     */
    float operator()() const;
};

#endif // GEOID_H
