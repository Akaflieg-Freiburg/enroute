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

using namespace Qt::Literals::StringLiterals;


namespace GeoMaps
{

/*! \brief Visual approach chart
 *
 *  This class represents visual approach charts. It stores the following data
 *  items.
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

    friend QDataStream& operator<<(QDataStream& stream, const GeoMaps::VAC& vac);
    friend QDataStream& operator>>(QDataStream& stream, GeoMaps::VAC& vac);

public:
    /*! \brief Default constructor, creates an invalid VAC */
    VAC() = default;

    /*! \brief Constructor
     *
     *  This class reads a georeferenced image file, where geographic data is
     *  encoded in one of the following two forms.
     *
     *  - The image file is a GeoTIFF file with embedded georeferencing
     *    information.
     *
     *  - The file name is of the form
     *    "EDTF-geo_7.739665_48.076416_7.9063883_47.96452.jpg"
     *
     *  It attempt to extract the map name from the image file (if the image
     *  file is a GeoTIFF), or else from the file name.  The raster data is not
     *  read, so that this constructor is rather lightweight.
     *
     *  \param fName File name of a georeferenced raster image file
     */
    VAC(const QString& fName);


    //
    // Properties
    //

    /*! \brief Geographic coordinate of raster image corner
     *
     * This coordinate might be invalid.
     */
    Q_PROPERTY(QGeoCoordinate bottomLeft MEMBER bottomLeft)

    /*! \brief Geographic coordinate of raster image corner
     *
     * This coordinate might be invalid.
     */
    Q_PROPERTY(QGeoCoordinate bottomRight MEMBER bottomRight)

    /*! \brief Center coordinate
     *
     * This property holds the geographic coordinate of the raster image center,
     * or an invalid coordinate if no valid corner coordinates are available.
     */
    Q_PROPERTY(QGeoCoordinate center READ center)

    /*! \brief Describe installed file(s)
     *
     * This property contains a description of the locally installed file(s),
     * localized and in HTML format. If no description is available, then the
     * property contains an empty string.
     */
    Q_PROPERTY(QString description READ description)

    /*! \brief Short info text
     *
     * The text is typically one lines "manually installed • 203 kB", translated
     * to the local language.
     */
    Q_PROPERTY(QString infoText READ infoText)

    /*! \brief Validity
     *
     * The VAC is considered valid if all corner coordinate are valid, the file
     * 'fileName' exists and the name is not empty.
     */
    Q_PROPERTY(bool isValid READ isValid)

    /*! \brief Name of the VAC. */
    Q_PROPERTY(QString name MEMBER name)

    /*! \brief Geographic coordinate of raster image corner
     *
     * This coordinate might be invalid.
     */
    Q_PROPERTY(QGeoCoordinate topLeft MEMBER topLeft)

    /*! \brief Geographic coordinate of raster image corner
     *
     * This coordinate might be invalid.
     */
    Q_PROPERTY(QGeoCoordinate topRight MEMBER topRight)

    /*! \brief Name of raster image file */
    Q_PROPERTY(QString fileName MEMBER fileName)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property of the same name
     *
     * @returns Property center
     */
    [[nodiscard]] QGeoCoordinate center() const;

    /*! \brief Getter function for property of the same name
     *
     * @returns Property description
     */
    [[nodiscard]] QString description() const;

    /*! \brief Getter function for property of the same name
     *
     * @returns Property infoText
     */
    [[nodiscard]] QString infoText() const;

    /*! \brief Getter function for property of the same name
     *
     * @returns Property isValid
     */
    [[nodiscard]] bool isValid() const;



    //
    // Methods
    //

    /*! \brief Comparison
     *
     * @param other VAC to compare *this with
     *
     * @returns True on equality
     */
    [[nodiscard]] bool operator==(const VAC& other) const = default;

    /*! \brief Mime type for files that can be opened by this class
     *
     *  @returns Name of mime type
     */
    [[nodiscard]] static QStringList mimeTypes()
    {
        return {u"image/jpg"_s,
                u"image/jpeg"_s,
                u"image/png"_s,
                u"image/tif"_s,
                u"image/tiff"_s,
                u"image/webp"_s};
    }



    //
    // Member variable
    //

    /*! \brief Member variable for property of the same name */
    QGeoCoordinate bottomLeft;

    /*! \brief Member variable for property of the same name */
    QGeoCoordinate bottomRight;

    /*! \brief Member variable for property of the same name */
    QString fileName;

    /*! \brief Member variable for property of the same name */
    QString name;

    /*! \brief Member variable for property of the same name */
    QGeoCoordinate topLeft;

    /*! \brief Member variable for property of the same name */
    QGeoCoordinate topRight;

private:
    // Obtain values for topLeft etc by looking at the file name
    void getCoordsFromFileName();

    // Obtain value name by looking at the file name
    void getNameFromFileName();

    // Check if all geo coordinates are valid
    [[nodiscard]] bool hasValidCoordinates() const;
};

/*! \brief Serialization */
QDataStream& operator<<(QDataStream& stream, const GeoMaps::VAC& vac);

/*! \brief Deserialization */
QDataStream& operator>>(QDataStream& stream, GeoMaps::VAC& vac);

} // namespace FileFormats


