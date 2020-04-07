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
 * \brief provide geoidal separation.
 *
 * according to https://en.wikipedia.org/wiki/Geoid
 *
 * In maps and common use (like VFR) the height over the mean sea level
 * (such as orthometric height) is used to indicate the height of elevations
 * while the ellipsoidal height results from the GPS system and similar GNSS.
 *
 * The deviation between the ellipsoidal height and the orthometric height
 * is provided here.
 */
class Geoid
{
public:
    Geoid();
    ~Geoid();

    /*! \brief return geoidal separation -- the difference between AMSL and ellipsoidal height.
     *
     * @returns geoidal separation
     */
    qreal operator()(qreal latitude, qreal longitude);

    /*! \brief returns true if geoidal separation is available or false otherwise
     *
     * @returns true or false
     */
    bool valid() const;

private:
    qint16* egm;

    // https://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/binary/readme.txt
    //
    const static qint32 egm96_rows = 721;
    const static qint32 egm96_cols = 1440;
    const static qint32 egm96_size = egm96_rows * egm96_cols;
};

#endif // GEOID_H
