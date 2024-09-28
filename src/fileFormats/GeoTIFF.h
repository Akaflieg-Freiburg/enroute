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

    /*! \brief Geographic coordinate for corner of raster image
     *
     *  @returns Coordinate, or an invalid coordinate in case of error.
     */
    [[nodiscard]] QGeoCoordinate bottomLeft() const { return m_bottomLeft; }

    /*! \brief Geographic coordinate for corner of raster image
     *
     *  @returns Coordinate, or an invalid coordinate in case of error.
     */
    [[nodiscard]] QGeoCoordinate bottomRight() const { return m_bottomRight; }

    /*! \brief Name, as specified in the GeoTIFF file
     *
     *  @returns The name or an empty string if no name is specified.
     */
    [[nodiscard]] QString name() const { return m_name; }

    /*! \brief Geographic coordinate for corner of raster image
     *
     *  @returns Coordinate, or an invalid coordinate in case of error.
     */
    [[nodiscard]] QGeoCoordinate topLeft() const { return m_topLeft; }

    /*! \brief Geographic coordinate for corner of raster image
     *
     *  @returns Coordinate, or an invalid coordinate in case of error.
     */
    [[nodiscard]] QGeoCoordinate topRight() const { return m_topRight; }


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

    /* This method computes a 4x4 tranformation matrix that maps pixel coordinates to
     * geographic coordinates.
     *
     * 1. First, the method uses readTransformation() to check if a matrix is specified
     * within the GeoTIFF file. If so, that matrix is returned.
     *
     * 2. Second, the method uses readTiepoints() and readPixelSize() to generate a matrix.
     * It assume at this point that the transformation is a simple scaling/translation,
     * and does not involve any rotation orr shearing. If sufficient data is present to
     * generate a transformation matrix, that matrix is returned.
     *
     * 3. An empty list is returned.
     */
    [[nodiscard]] static QList<double> getTransformation(const QMap<quint16, QVariantList> &TIFFFields);

    /* This method interprets the TIFFFields and looks for the tag 33922, which is used to
     * specify the tiepoints.  Returns a list with the data retrieved, or an empty
     * list on failure.
     *
     * An expection might be thrown if the tag exists, but contains invalid data.
     */
    [[nodiscard]] static QList<Tiepoint> readTiepoints(const QMap<quint16, QVariantList> &TIFFFields);

    /* This method interprets the TIFFFields and looks for the tag 270, which is used to
     * specify the image name.  Returns a QString with the data retrieved, or an empty
     * QString on failure.
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
     * specify a 4x4 transformation matrix. Returns a list of 16 doubles on success, in order
     * (a00 a01 a02 a03 a10 ...). Returns an empty list on failure.
     *
     * An expection might be thrown if the tag exists, but contains invalid data.
     */
    [[nodiscard]] static QList<double> readTransformation(const QMap<quint16, QVariantList> &TIFFFields);

    /* This methods interprets the data found in m_TIFFFields and writes to
     * m_name and m_topLeft etc. On failure, it throws a QString with a human-readable,
     * translated error message.
     */
    void interpretGeoData();

    // Geographic coordinates for corner of raster image
    QGeoCoordinate m_topLeft;
    QGeoCoordinate m_topRight;
    QGeoCoordinate m_bottomLeft;
    QGeoCoordinate m_bottomRight;

    // Name
    QString m_name;
};

} // namespace FileFormats
