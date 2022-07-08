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

#include <QMap>

namespace GeoMaps {

/*! \brief Utility class for databases in MBTILES format
 *
 *  MBTILES contain tiled map data. Internally, MBTILES are SQLite databases whose schema is specified here:
 *  https://github.com/mapbox/mbtiles-spec
 *  This class handles MBTILES and allows easy access to the data.
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

    /*! \brief Standard constructor
     *
     * Constructs an object from an MBTILES file. The file is supposed to exist and remain intact throughout the existence of this class instance.
     */
    MBTILES(const QString& fileName);

    /*! \brief Standard destructor
     *
     * Constructs an object from an MBTILES file.
     */
    ~MBTILES();

    /*! \brief Attribution of MBTILES file
     *
     *  @returns A human-readable HTML-String with attribution, or an empty string on error.
     */
    [[nodiscard]] QString attribution();

    /*! \brief Determine type of data contain in an MBTILES file
     *
     *  @returns Type of data, or Unknown on error.
     */
    [[nodiscard]] GeoMaps::MBTILES::Format format();

    /*! \brief Information about an MBTILES file
     *
     *  @returns A human-readable HTML-String with information about the MBTILES file, or an empty string on error.
     */
    [[nodiscard]] QString info();

    /*! \brief Retrieve tile from an MBTILES file
     *
     *  @param zoom Zoom level of the tile
     *
     *  @param x x-Coordinate of the tile
     *
     *  @param y y-Coordinate of the tile
     *
     *  @returns A QByteArray with the tile data, or an empty QByteArray on error.
     */
    [[nodiscard]] QByteArray tile(int zoom, int x, int y);

    /*! \brief Retrieve metadata of the MBTILES file
     *
     *  MBTILES files contain metadata, in the form of a list of key/value pairs.
     *
     *  @returns A QMap containing the metadata, or an empty QMap on error.
     */
    [[nodiscard]] QMap<QString, QString> metaData() const
    {
        return m_metadata;
    }

    /*! \brief Retrieve name of the MBTILES file
     *
     *  @returns Filename, as given in the constructor
     */
    [[nodiscard]] QString fileName() const
    {
        return m_fileName;
    }
private:
    //
    Q_DISABLE_COPY_MOVE(MBTILES)

    // Name of the MBTILES file
    QString m_fileName;

    // Name of the data base connection. This name is unique to each instance of this class, and should therefore not be copied.
    QString m_databaseConnectionName;
    QMap<QString, QString> m_metadata;
};

}
