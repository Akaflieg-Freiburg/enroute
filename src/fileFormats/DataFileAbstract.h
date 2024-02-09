/***************************************************************************
 *   Copyright (C) 2023.2024 by Stefan Kebekus                             *
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

#include <QFile>
#include <QObject>
#include <QSharedPointer>

namespace FileFormats
{

/*! \brief Base class for file reading classes
 *
 *  This is an abstract base class for all classes that read data files, such as
 *  CUP, GeoJSON, GeoTIFF, …. Implementations should adhere to the following
 *  conventions:
 *
 *  - The constructor takes a file name as an argument and reads the file, which
 *    might be quite expensive.
 *
 *  - There is a static method 'mimeTypes' that describes the mime types of the
 *    files that can be opened with this class.
 */

class DataFileAbstract
{

public:
    DataFileAbstract() = default;
    ~DataFileAbstract() = default;


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
    [[nodiscard]] bool isValid() const { return m_error.isEmpty(); }

    /*! \brief Error string
     *
     *  If the constructor found the file to be invalid, this method returns a
     *  human-readable, transated error string.
     *
     *  @returns Error string, or an empty string if the file is valid.
     */
    [[nodiscard]] QString error() const { return m_error; }

    /*! \brief Warnings
     *
     *  If the constructor found non-critical issues in the file, this method
     *  returns a list of human-readable, transated warnings.
     *
     *  @returns List of warning, or an empty list if there were no warnings.
     */
    [[nodiscard]] QStringList warnings() const { return m_warnings; }



    //
    // Methods
    //

    /*! \brief Open file, file URL or Android content URL
     *
     *  This method opens a file. It handles file URLs and Android content URLs. The
     *  latter are downloaded to a temporary file.
     *
     *  @param fileName A file name, a file URL or an Android content URL
     *
     *  @returns A shared pointer to a file, which is already open. The file might be a QTemporaryFile.
     *  The file might have an error condition set.
     */
    [[nodiscard]] static QSharedPointer<QFile> openFileURL(const QString& fileName);

protected:
    void addWarning(const QString& warning) { m_warnings += warning; }
    void setError(const QString& newError) { m_error = newError; }

private:
    Q_DISABLE_COPY_MOVE(DataFileAbstract)

    QString m_error;
    QStringList m_warnings;
};

} // namespace FileFormats
