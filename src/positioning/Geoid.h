/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#pragma once

#include <QGeoCoordinate>

#include "units/Distance.h"

namespace Positioning {

/*! \brief Provide geoidal separation according to https://en.wikipedia.org/wiki/Geoid
 *
 * In maps and common use (like VFR) the height over the mean sea level (such as
 * orthometric height) is used to indicate the height of elevations while the
 * ellipsoidal height results from the GPS system and similar GNSS.
 *
 * The deviation between the ellipsoidal height and the orthometric height is
 * provided here.
 *
 * The implementation uses the data from
 * https://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/binary/binarygeoid.html
 * and does a bilinear interpolation between the four neighboring data points of
 * the requested location.
 *
 * The implementation was _very_ carefully tested and compared with both a
 * bilinear and a bicubic spline interpolation in python.  The comparison of the
 * method here with the python bilinear interpolation verified that both
 * implementations yield the same numbers (within numerical precision).  The
 * comparison of the bilinear implementation here with the python's bicubic
 * interpolation showed a worldwide max deviation of about 1 m.
 */

class Geoid
{
public:
    Geoid() = delete;

    //
    // Methods
    //

    /*! \brief return geoidal separation -- the difference between AMSL and ellipsoidal height.
     *
     * @param coord location for which the geoidal separation should be calculated.
     *
     * @returns Geoidal separation. In case that the method fails, NAN is returned
     */
    static Units::Distance separation(const QGeoCoordinate& coord);

private:
    // Reads data into the vector egm
    static void readEGM();

    static QVector<qint16> egm; // holds the data read from the binaray data file WW15MGH.DAC

    // https://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/binary/readme.txt
    // https://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/binary/binarygeoid.html

    const static qint32 egm96_rows = 721;
    const static qint32 egm96_cols = 1440;
    const static qint32 egm96_size = egm96_rows * egm96_cols;
};

}
