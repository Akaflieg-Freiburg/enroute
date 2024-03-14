/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

#include "fileFormats/DataFileAbstract.h"
#include "geomaps/Waypoint.h"


namespace FileFormats
{

    /*! \brief MapURL file support class
     *
     *  The methods of this class read MapURL waypoint files, as specified here:
     *  http://download.naviter.com/docs/MapURL-file-format-description.pdf
     */

    class MapURL : public DataFileAbstract
    {

    public:
        /*! \brief Constructor
         *
         *  This method reads a URL and tries to extract geographic coordinates.
         *  The following URL types are currently supported:
         *
         *  - Google Maps: https://www.google.com/maps/place/79418+Schliengen/@47.7732771,7.6012134,13z/data=!4m6!3m5!1s0x4791a680c9f694c1:0x1c1f6bbb0c9264b0!8m2!3d47.7596149!4d7.5975301!16s%2Fg%2F1tglxxnn?entry=ttu
         *
         *  @param urlName URL
         */
        MapURL(const QString& urlName);



        //
        // Getter Methods
        //

        /*! \brief Waypoints specified in the MapURL file
         *
         *  @returns Waypoints specified in the MapURL file
         */
        [[nodiscard]] GeoMaps::Waypoint waypoint() const { return m_waypoint; }


    private:
        GeoMaps::Waypoint m_waypoint;
    };

    } // namespace FileFormats
