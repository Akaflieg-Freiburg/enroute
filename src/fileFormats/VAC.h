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

#include <QGeoCoordinate>
#include <QImage>
#include <QQmlEngine>

#include "fileFormats/DataFileAbstract.h"

namespace FileFormats
{

/*! \brief Visual approach chart
 *
 *  This class reads a georeferenced image file.
 *
 */
class VAC : public DataFileAbstract
{
    Q_GADGET
    QML_VALUE_TYPE(vac)

public:
    /*! \brief Default constructor
     */
    VAC();

    /*! \brief Constructor
     *
     *  This class reads a georeferenced image file, where georeferencing data is
     *  encoded in of the two following ways.
     *
     *  * The image file is a GeoTIFF file with embedded georeferencing information
     *
     *  * The file name is of the form
     *    "EDTF-geo_7.739665_48.076416_7.9063883_47.96452.jpg"
     *
     *  This constructor reads the boundary box of the georeferenced image file
     *  and guesses a good base name.  If no good name can be guessed, the base name
     *  might well be empty.
     *
     *  This constructor reads the raster data. It is therefore not lightweight
     *  on memory.
     *
     *  \param fileName File name of a georeferenced image file.
     */
    VAC(const QString& fileName);

    /*! \brief Constructor
     *
     *  This reads raster data and geoCoordinates for the edges of the raster image.
     *  The image is rotated to "north up" and the bounding box is computed accordingly.
     *  A base name needs to be set before the VAC can be saved.
     *
     *  This constructor reads the raster data and performs rotation. It is absolutely not lightweight.
     *
     *  \param data Raster data in a format the QImage can read
     *
     *  \param topLeft GeoCoordinate for image edge
     *
     *  \param topRight GeoCoordinate for image edge
     *
     *  \param bottomLeft GeoCoordinate for image edge
     *
     *  \param bottomRight GeoCoordinate for image edge
     */
    VAC(const QString &fileName,
        QGeoCoordinate topLeft,
        QGeoCoordinate topRight,
        QGeoCoordinate bottomLeft,
        QGeoCoordinate bottomRight);

#warning unfinished
    Q_PROPERTY(QGeoCoordinate center READ center CONSTANT)
    Q_PROPERTY(QString baseName READ baseName CONSTANT)
    Q_PROPERTY(bool isValid READ isValid CONSTANT)
    Q_PROPERTY(QGeoCoordinate topLeft READ topLeft CONSTANT)
    Q_PROPERTY(QGeoCoordinate topRight READ topRight CONSTANT)
    Q_PROPERTY(QGeoCoordinate bottomLeft READ bottomLeft CONSTANT)
    Q_PROPERTY(QGeoCoordinate bottomRight READ bottomRight CONSTANT)
    Q_PROPERTY(QString fileName READ fileName CONSTANT)


    //
    // Getter Methods
    //

    /*! \brief Base name
     *
     *  The base name is a suggested name for this visual approach chart, to be
     *  used in the GUI and as a file name. It can be invalid or empty.
     *
     *  @returns A QString with the base name
     */
    [[nodiscard]] auto baseName() const -> QString { return m_baseName; }

    [[nodiscard]] QGeoCoordinate center() const;

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

    /*! \brief File name
     *
     *  @returns A QString with the file name
     */
    [[nodiscard]] auto fileName() const -> QString { return m_fileName; }

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
    // Setter Methods
    //

    /*! \brief Setter function for the base name
     *
     *  @param newBaseName New base name
     */
    void setBaseName(const QString& newBaseName) { m_baseName = newBaseName; }



    //
    // Methods
    //

    /*! \brief Read base name from file name
     *
     *  For file names of the form path/EDFR-geo_10.1535_49.4293_10.2822_49.3453.webp or path/EDFR_10.1535_49.4293_10.2822_49.3453.webp,
     *  this method returns the string "EDFR".
     *
     *  @param fileName file name
     *
     *  @return Base name, or an empty string in case of an error
     */
    [[nodiscard]] static QString baseNameFromFileName(const QString& fileName);

    /*! \brief Mime type for files that can be opened by this class
     *
     *  @returns Name of mime type
     */
    [[nodiscard]] static QStringList mimeTypes() { return {u"image/jpg"_qs,
                                                           u"image/jpeg"_qs,
                                                           u"image/png"_qs,
                                                           u"image/tif"_qs,
                                                           u"image/tiff"_qs,
                                                           u"image/webp"_qs}; }

private:
#warning document
    bool coordsFromFileName();

#warning document
    void generateErrorsAndWarnings();

    QString m_baseName {};
    QString m_fileName {};

    // Geographic coordinates for corner of raster image
    QGeoCoordinate m_topLeft {};
    QGeoCoordinate m_topRight {};
    QGeoCoordinate m_bottomLeft {};
    QGeoCoordinate m_bottomRight {};
};

} // namespace FileFormats
