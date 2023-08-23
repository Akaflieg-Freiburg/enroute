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
/*! \brief VAC file support class
 *
 *  This class reads a georeferenced image file, extracts the bounding box and suggests a base name for the map.
 */

class VAC
{

public:
    /*! \brief Standard constructor
     *
     *  The constructor reads the boundary box of the georeferenced image file, but not the raster data.
     *  It is therefore lightweight on memory.
     *
     * \param fileName File name of a georeferenced image file. If this file is not a GeoTIFF, then the file name
     * must be of the form "EDTF Freiburg-geo_7.739665_48.076416_7.9063883_47.96452.jpg".
     */
    VAC(const QString& fileName);


    //
    // Methods
    //

    /*! \brief Base name
     *
     *  This method guesses a good name for the image. If the fileName presented to the constructor is "EDTF.tif", this method
     *  returns "EDTF". If the fileName is "EDTF Freiburg-geo_7.739665_48.076416_7.9063883_47.96452.jpg", it will return
     *  "EDTF Freiburg". In other cases, the result is undefined, and the base name might well be empty.
     *
     *  @returns A QString with the base name
     */
    [[nodiscard]] QString baseName() const { return m_baseName; }

    /*! \brief Bounding box
     *
     *  This method returns the bounding box of the georeferenced image.
     *
     *  @returns A QGeoRectangle for the four corners of the image. In case of error, an invalid QGeoRectangle is returned.
     */
    [[nodiscard]] QGeoRectangle bBox() const { return m_bBox; }

    /*! \brief File name
     *
     *  @returns A QString with the file name, as set in the constructor
     */
    [[nodiscard]] QString fileName() const { return m_fileName; }

    /*! \brief Test for validity
     *
     *  A VAC is considered valid if the bounding box is valid and if the raster data can be loaded
     *  successfully. This method will test-load the raster data and is therefore expensive.
     *
     *  @returns True if this VAC is valid
     */
    [[nodiscard]] bool isValid() const;

private:
    QGeoRectangle m_bBox;
    QString m_baseName;
    QString m_fileName;
};

} // namespace GeoMaps
