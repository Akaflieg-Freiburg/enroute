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

#include "DataFileAbstract.h"
#include "geomaps/Waypoint.h"


namespace FileFormats
{

    /*! \brief CUP file support class
     *
     *  The methods of this class read CUP waypoint files, as specified here:
     *  http://download.naviter.com/docs/CUP-file-format-description.pdf
     */

    class CUP : public DataFileAbstract
    {

    public:
        /*! \brief Constructor
         *
         *  This method reads a CUP file and generates a vector of waypoints.
         *
         *  @param fileName Name of a CUP file
         */
        CUP(const QString& fileName);



        //
        // Getter Methods
        //

        /*! \brief Waypoints specified in the CUP file
         *
         *  @returns Waypoints specified in the CUP file
         */
        [[nodiscard]] QVector<GeoMaps::Waypoint> waypoints() const { return m_waypoints; }

    private:
        // Private helper functions
        static QStringList parseCSV(const QString& string);
        static GeoMaps::Waypoint readWaypoint(const QString &line);

        QVector<GeoMaps::Waypoint> m_waypoints;
    };

} // namespace GeoMaps
