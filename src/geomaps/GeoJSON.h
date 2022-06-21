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

    /*! \brief GeoJSON file support class
     *
     *  The methods of this class read GeoJSON waypoint files, as specified here:
     *  https://geojson.org/
     */

    class GeoJSON
    {

    public:
        /*! File type */
        enum fileContent {
            flightRoute, /*< File is valid and contains a flight route */
            invalid, /*< File is invalid */
            valid, /*< File is valid, content unspecified */
            waypointLibrary /*< File is valid and contains a waypoint library */
        };

        /*! Indicator string for flight routes
         *
         *  A GeoJSON file is expected to contain a flight route if this string appears
         *  as 'enroute' in the top-level JSON objects.
         *
         *  @returns Indicator string
         */
        static QString indicatorFlightRoute()
        {
            return QStringLiteral("flight route");
        }

        /*! Indicator string for flight routes
         *
         *  A GeoJSON file is expected to contain a waypoint library if this string appears
         *  as 'enroute' in the top-level JSON objects.
         *
         *  @returns Indicator string
         */
        static QString indicatorWaypointLibrary()
        {
            return QStringLiteral("waypoint library");
        }

        /*! \brief Inspect file
         *
         *  This method reads a file, to check if it contains GeoJSON data. If so, it
         *  tries to determine the type of the datat
         *
         *  @param fileName Name of a file
         *
         *  @returns Most probably file type
         */
        static fileContent inspect(const QString& fileName);

        /*! \brief Read a GeoJSON file
         *
         *  This method reads a GeoJSON file and generates a vector of waypoints.
         *
         *  @param fileName Name of a CUP file
         *
         *  @returns QVector with waypoints. The vector is empty in case of an error.
         */
        static QVector<GeoMaps::Waypoint> read(const QString &fileName);
    };

};
