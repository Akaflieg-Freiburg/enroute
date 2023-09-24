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

#include <QObject>

namespace FileFormats
{

/*! \brief Base class for file reading classes
 *
 *  This is the base class for all classes that read data files, such as CUP,
 *  GeoJSON, GeoTIFF, …
 */

class DataFileAbstract
{

public:
    /*! \brief Constructor
     *
     *  The constructor reads the file, which is usually quite expensive. Use
     *  the static method hasCorrectMimeType() for a quick check.
     *
     *  @param fileName Name of file that is to be read.
     */
    DataFileAbstract(const QString& fileName) = 0;



    //
    // Getter methods
    //

    /*! \brief Validity check
     *
     *  Use this method to check if the file is valid. If not, then all the
     *  other getter methods will typically return undefined results. Since all
     *  the heavy lifting is done in the constructor, this method is extremely
     *  lightweight.
     *
     *  This method is equivalent to !error().isEmpty()
     *
     *  @returns True if the data is valid.
     */
    bool isValid() const { return !m_error.isEmpty(); }

    /*! \brief Error string
     *
     *  If the constructor found the file to be invalid, this method returns a
     *  human-readable, transated error string.
     *
     *  @returns Error string, or an empty string if the file is valid.
     */
    QString error() const { return m_error; }

    /*! \brief Warnings
     *
     *  If the constructor found non-critical issues in the file, this method
     *  returns a list of human-readable, transated warnings.
     *
     *  @returns List of warning, or an empty list if there were no warnings.
     */
    QStringList warnings() const { return m_warnings; }



    //
    // Static methods
    //

    /*! \brief Quick check if the file has the correct mime type.
     *
     *  @param fileName Name of file that is to be read.
     *
     *  @returns True if the file could potentially be valid data file
     */
    static bool hasCorrectMimeType(const QString& fileName) = 0;

protected:
    QString m_error;
    QStringList m_warnings;

private:
    Q_DISABLE_COPY_MOVE(DataFileAbstract)

};

} // namespace FileFormats
