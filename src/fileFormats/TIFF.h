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

#include <QSize>
#include <QVariant>

#include "DataFileAbstract.h"

namespace FileFormats
{

/*! \brief TIFF support
 *
 *  This class reads GeoTIFF files. It extracts image dimension as well
 *  as the TIFF fields. It does not read the raster data.
 *
 */

class TIFF : public DataFileAbstract
{
public:
    /*! \brief Constructor
     *
     *  The constructor opens and reads the TIFF file. It does not read
     *  the raster data and is therefore lightweight.
     *
     *  \param fileName File name of a TIFF file.
     */
    TIFF(const QString& fileName);

    /*! \brief Constructor
     *
     *  The constructor opens and rteads the TIFF file. It does not read
     *  the raster data and is therefore lightweight.
     *
     *  \param device Device from which the TIFF is read. The device must be
     *  opened and seekable. The device will not be closed by this method.
     */
    TIFF(QIODevice& device);



    //
    // Getter Methods
    //

    /*! \brief TIFF data fields
     *
     * @returns TIFF-internal data fields
     */
    [[nodiscard]] QMap<quint16, QVariantList> fields() { return m_TIFFFields; }

    /*! \brief Size of the TIFF raster image
     *
     * @returns Size of the TIFF raster image, or an invalid size in case or error.
     */
    [[nodiscard]] QSize rasterSize() { return m_rasterSize; }


    //
    // Static methods
    //

    /*! \brief Mime type for files that can be opened by this class
     *
     *  @returns Name of mime type
     */
    [[nodiscard]] static QStringList mimeTypes() { return {u"image/tiff"_qs}; }

private:
    /* This methods reads the TIFF data from the device. On success, it fills
     * the memeber m_TIFFFields with appropriate data. On failure, it throws a
     * QString with a human-readable, translated error message.
     *
     * @param device QIODevice from which the TIFF header will be read. This
     * device must be seekable.
     */
    void readTIFFData(QIODevice& device);

    /* This methods reads a single TIFF field from the device. On success, it
     * adds an entry to the member m_TIFFFields and positions the device on the
     * byte following the structure. On failure, it throws a QString with a
     * human-readable, translated error message.
     *
     * This method only reads values of type ASCII, SHORT and DOUBLE. Values of
     * other types will be ignored.
     *
     * @param device QIODevice from which the TIFF header will be read. This
     * device must be open, seekable, and positioned to the beginning of the
     * TIFF field structure.
     *
     * @param dataStream QDataStream that is connected to the device and has the
     * correct endianness set.
     */
    void readTIFFField(QIODevice& device, QDataStream& dataStream);

    /* This method interprets m_TIFFFields, extracts the size of the raster
     * image and writes the result into m_rasterSize.
     */
    void readRasterSize();

    // Size of the raster imags
    QSize m_rasterSize;

    // TIFF tags and associated data
    QMap<quint16, QVariantList> m_TIFFFields;
};

} // namespace FileFormats
