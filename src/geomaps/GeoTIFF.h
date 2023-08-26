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

#include <QGeoRectangle>

namespace GeoMaps
{
/*! \brief GeoTIFF support class
 *
 *  This class reads the bounding box off a GeoTIFF file.
 */

class GeoTIFF
{

public:
    /*! \brief Get bounding box from a GeoTIFF file
     *
     *  @param fileName Name of the GeoTIFF file.
     *
     *  @returns The bounding box of the GeoTIFF file. In case of an error, an invalid
     *  bounding box is returned.
     */
    static QGeoRectangle bBox(const QString& fileName);
};

} // namespace GeoMaps
