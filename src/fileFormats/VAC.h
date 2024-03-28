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
#include <QQmlEngine>


namespace FileFormats
{

/*! \brief Visual approach chart
 *
 *  This class represents visual approach charts. It stores the following data items.
 *
 *  - The name of the visual approach chart.
 *
 *  - The name of a raster image file.
 *
 *  - Geographic coordinates for the four corners of the raster image
 */
class VAC
{
    Q_GADGET
    QML_VALUE_TYPE(vac)

public:
    /*! \brief Default constructor, creates an invalid VAC */
    VAC() = default;

    /*! \brief Constructor
     *
     *  This class reads a georeferenced image file, where geographic data is encoded in one of the following two forms.
     *
     *  - The image file is a GeoTIFF file with embedded georeferencing information.
     *
     *  - The file name is of the form "EDTF-geo_7.739665_48.076416_7.9063883_47.96452.jpg"
     *
     *  It attempt to extract the map name from the image file (if the image file is a GeoTIFF), or else from the file name.  The raster data is not read, so that this constructor is rather lightweight.
     *
     *  \param fileName File name of a georeferenced raster image file
     */
    VAC(const QString& fileName);

    /*! \brief Constructor
     *
     *  This class takes the name of an image file and geographic coordinates for the four corners
     *  of the raster image. It attempt to extract the map name from the image file (if the image file is a GeoTIFF), or else from the file name.
     *
     *  The raster data is not read, so that this constructor is rather lightweight.
     *
     *  \param fileName File name of a raster image file
     *
     *  \param topLeft QGeoCoordinate for image corner
     *
     *  \param topRight QGeoCoordinate for image corner
     *
     *  \param bottomLeft QGeoCoordinate for image corner
     *
     *  \param bottomRight QGeoCoordinate for image corner
     */
    VAC(const QString& fileName,
        const QGeoCoordinate& topLeft,
        const QGeoCoordinate& topRight,
        const QGeoCoordinate& bottomLeft,
        const QGeoCoordinate& bottomRight);

#warning unfinished
    Q_PROPERTY(QGeoCoordinate center READ center CONSTANT)
    Q_PROPERTY(QString infoText READ infoText CONSTANT)
    Q_PROPERTY(QString description READ description CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
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
    [[nodiscard]] QString name() const  { return m_name; }

#warning docu
    [[nodiscard]] bool isValid() const;

    [[nodiscard]] bool hasValidCoordinates() const;


    [[nodiscard]] QString infoText() const;

    [[nodiscard]] QString description() const;


#warning docu
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
    void setBaseName(const QString& newBaseName) { m_name = newBaseName; }

    void setFileName(const QString& newFileName) { m_fileName = newFileName; }


    //
    // Methods
    //

    bool operator==(const VAC& other) const = default;

    /*! \brief Mime type for files that can be opened by this class
     *
     *  @returns Name of mime type
     */
    [[nodiscard]] static QStringList mimeTypes()
    {
        return {u"image/jpg"_qs,
                u"image/jpeg"_qs,
                u"image/png"_qs,
                u"image/tif"_qs,
                u"image/tiff"_qs,
                u"image/webp"_qs};
    }

    [[nodiscard]] static QString getNameFromFileName(const QString& fileName);

private:
#warning document
    bool getCoordsFromFileName();

    QString m_name {};
    QString m_fileName {};

    // Geographic coordinates for corner of raster image
    QGeoCoordinate m_topLeft {};
    QGeoCoordinate m_topRight {};
    QGeoCoordinate m_bottomLeft {};
    QGeoCoordinate m_bottomRight {};
};

} // namespace FileFormats
