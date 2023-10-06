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
#include <QImage>

namespace GeoMaps
{

/*! \brief Visual approach chart
 *
 *  This class reads a georeferenced image file, where georeferencing data is
 *  encoded in of the two following ways.
 *
 *  * The image file is a GeoTIFF file with embedded georeferencing information
 *
 *  * The file name is of the form
 *    "EDTF-geo_7.739665_48.076416_7.9063883_47.96452.jpg"
 *
 */
class VAC
{

public:
    /*! \brief Constructor
     *
     *  The constructor reads the boundary box of the georeferenced image file
     *  and guesses a good base name.
     *
     *  If the fileName presented to the constructor is "EDTF.tif", this method
     *  sets "EDTF" for a base name. If the fileName is "EDTF
     *  Freiburg-geo_7.739665_48.076416_7.9063883_47.96452.jpg", it set "EDTF
     *  Freiburg". In other cases, the result is undefined, and the base name
     *  might well be empty.
     *
     *  This constructor reads the raster data. It is therefore not lightweight
     *  on memory.
     *
     *  \param fileName File name of a georeferenced image file.
     */
    VAC(const QString& fileName);


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

    /*! \brief Bounding box
     *
     *  This method returns the bounding box of the georeferenced image.
     *
     *  @returns The bounding box. In case of error, an invalid QGeoRectangle is
     *  returned.
     */
    [[nodiscard]] auto bBox() const -> QGeoRectangle { return m_bBox; }

    /*! \brief Error message
     *
     *  If the visual approach chart is invalid, this method contains a short
     *  explanation.
     *
     *  @returns A human-readable, translated warning or an empty string if no
     *  error.
     */
    [[nodiscard]] auto error() const -> QString { return m_error; }

    /*! \brief Test for validity
     *
     *  A visual approach chart is considered valid if the bounding box is valid
     *  and if the raster data can be loaded successfully.
     *
     *  @returns True if this visual approach chart is valid
     */
    [[nodiscard]] auto isValid() const -> bool;

    /*! \brief Warning
     *
     *  If the visual approach chart is technically valid, but unlikely to be
     *  correct, this method returns a short explanation of the problem.
     *
     *  @returns A human-readable, translated warning or an empty string if no
     *  warning.
     */
    [[nodiscard]] auto warning() const -> QString { return m_warning; }



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

    /*! \brief Save visual approach chart
     *
     *  This method copies the visual approach chart to a new directory, chooses
     *  an appropriate file name of the form
     *  "baseName-geo_7.739665_48.076416_7.9063883_47.96452.webp". If the image
     *  is not in webp format already, it will be converted to webp using a
     *  lossy encoder.
     *
     *  @param directoryName Name of a directory where the VAC will be stored.
     *  The directory and its parents are created if necessary
     *
     *  @returns Path of the newly created file, or an empty string in case of
     *  error.
     */
    [[nodiscard]] auto save(const QString &directoryName) -> QString;

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

    /*! \brief Read bounding box from file name
     *
     *  This method checks if the file name is of the form EDFR-geo_10.1535_49.4293_10.2822_49.3453.webp or EDFR_10.1535_49.4293_10.2822_49.3453.webp.
     *  If yes, it returns the bounding box for that file.
     *
     *  @param fileName file name
     *
     *  @return Bounding box, or an invalid bounding box in case of an error
     */
    [[nodiscard]] static QGeoRectangle bBoxFromFileName(const QString& fileName);

private:
    void generateErrorsAndWarnings();

    QGeoRectangle m_bBox;
    QString m_baseName;
    QString m_fileName;
    QImage m_image;
    QString m_warning;
    QString m_error;
};

} // namespace GeoMaps
