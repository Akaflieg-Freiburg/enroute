/***************************************************************************
 *   Copyright (C) 2023-2024 by Stefan Kebekus                             *
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
#include <QPointF>
#include <QVariant>

#include "TIFF.h"

namespace FileFormats
{

/*! \brief GeoTIFF support
 *
 *  This class reads GeoTIFF files, as specified here:
 *  https://gis-lab.info/docs/geotiff-1.8.2.pdf
 *
 *  It extracts bounding box coordinates, as well as the name of the file. This
 *  class does not read the raster data. GeoTIFF is a huge and complex standard,
 *  and this class is definitively not able to read all possible valid GeoTIFF
 *  files. We restrict ourselves to files that appear in real-world aviation.
 */

class GeoTIFF : public TIFF
{
public:
    /*! \brief Constructor
     *
     *  The constructor opens and analyzes the GeoTIFF file. It does not read
     *  the raster data and is therefore lightweight.
     *
     *  \param fileName File name of a GeoTIFF file.
     */
    GeoTIFF(const QString& fileName);

    /*! \brief Constructor
     *
     *  The constructor opens and analyzes the GeoTIFF file. It does not read
     *  the raster data and is therefore lightweight.
     *
     *  \param device Device from which the GeoTIFF is read. The device must be
     *  opened and seekable. The device will not be closed by this method.
     */
    GeoTIFF(QIODevice& device);


    //
    // Getter Methods
    //

    /*! \brief Name, as specified in the GeoTIFF file
     *
     *  @returns The name or an empty string if no name is specified.
     */
    [[nodiscard]] QString name() const { return m_name; }

    /*! \brief Bounding box, as specified in the GeoTIFF file
     *
     *  @returns Bounding box, which might be invalid
     */
    [[nodiscard]] QGeoRectangle bBox() const { return m_bBox; }



    //
    // Static methods
    //

    /*! \brief Mime type for files that can be opened by this class
     *
     *  @returns Name of mime type
     */
    [[nodiscard]] static QStringList mimeTypes() { return FileFormats::TIFF::mimeTypes(); }

private:
    struct Tiepoint
    {
        QPointF rasterCoordinate;
        QGeoCoordinate geoCoordinate;
    };

    [[nodiscard]] static QList<double> getTransformation(const QMap<quint16, QVariantList> &TIFFFields);
    [[nodiscard]] static QList<Tiepoint> readTiepoints(const QMap<quint16, QVariantList> &TIFFFields);

    /* This method interprets the TIFFFields and looks for the tag 270, which is used to
     * specify the image name.  Returns a QString with the data retrieved, or an empty
     * QString F on failure.
     *
     * An expection might be thrown if the tag exists, but contains invalid data.
     */
    [[nodiscard]] static QString readName(const QMap<quint16, QVariantList>& TIFFFields);

    /* This method interprets the TIFFFields and looks for the tag 33550, which is used to
     * specify the geographic size of a pixel.  Returns a QSizeF with the data retrieved,
     * or an invalid QSizeF on failure.
     *
     * An expection might be thrown if the tag exists, but contains invalid data.
     */
    [[nodiscard]] static QSizeF readPixelSize(const QMap<quint16, QVariantList> &TIFFFields);

    /* This method interprets the TIFFFields and looks for the tag 34264, which is used to
     * specify a 4x4 translation matrix. Returns a list of 16 doubles on success, in order
     * (a00 a01 a02 a03 a10 ...). Returns an empty list on failure.
     *
     * An expection might be thrown if the tag exists, but contains invalid data.
     */
    [[nodiscard]] static QList<double> readTransformation(const QMap<quint16, QVariantList> &TIFFFields);

    void computeGeoQuadrangle(const QMap<quint16, QVariantList>& TIFFFields);

    /* This methods interprets the data found in m_TIFFFields and writes to
     * m_bBox and m_name.On failure, it throws a QString with a human-readable,
     * translated error message.
     */
    void interpretGeoData();

    // Bounding box
    QGeoRectangle m_bBox;

    // Name
    QString m_name;
};

} // namespace FileFormats
