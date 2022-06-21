/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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

#include "geomaps/Waypoint.h"

namespace GeoMaps
{

    /*! \brief GPX file support class
     *
     *  The methods of this class read GPX waypoint files, as specified here:
     */

    class GPX
    {

    public:
        /*! \brief Check if file contains valid GPX data
         *
         *  @param fileName Name of a file
         *
         *  @returns True if the file is likely to contain valid GPX data.
         */
        static bool isValid(const QString &fileName);

        /*! \brief Read a GPX file
         *
         *  This method reads a GPX file and generates a vector of waypoints.
         *
         *  @param fileName Name of a CUP file
         *
         *  @returns QVector with waypoints. The vector is empty in case of an error.
         */
        static QVector<GeoMaps::Waypoint> read(const QString &fileName);
    };

};
