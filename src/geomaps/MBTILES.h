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

#include <QObject>

namespace GeoMaps {

/*! \brief Utility class for databases in MBTILES format
 *
 *  MBTILES are SQLite databases whose schema is specified here:
 *  https://github.com/mapbox/mbtiles-spec
 */

class MBTILES {

public:
    /*! \brief Format of data tiles */
    enum Format {
        /*! \brief Unknown format */
        Unknown,

        /*! \brief Vector data in PBF format */
        Vector,

        /*! \brief Raster data in JPG, PNG or WEBP format */
        Raster,
    };

    /*! \brief Attribution of MBTILES file
     *
     *  @param fileName Name of the file
     *
     *  @returns A human-readable HTML-String with attribution, or an empty string on error.
     */
    [[nodiscard]] static QString attribution(const QString& fileName);

    /*! \brief Determine type of data contain in an MBTILES file
     *
     *  @param fileName Name of the file
     *
     *  @returns Type of data, or Unknown on error.
     */
    [[nodiscard]] static GeoMaps::MBTILES::Format format(const QString& fileName);

    /*! \brief Information about an MBTILES file
     *
     *  @param fileName Name of the file
     *
     *  @returns A human-readable HTML-String with information about the file, or an empty string on error.
     */
    [[nodiscard]] static QString info(const QString& fileName);

};

}
