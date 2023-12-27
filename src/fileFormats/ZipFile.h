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

#include "fileFormats/DataFileAbstract.h"


namespace FileFormats
{

/*! \brief ZIP Archive
 *
 *  This class reads a ZIP file and allows extracting individual files.
 */
class ZipFile : public DataFileAbstract
{
public:
    /*! \brief Constructor
     *
     *  The constructor opens the ZIP file. The class assumes that the ZIP file
     *  remains untouched throughout the life cycle of this instance.
     *
     *  \param fileName File name of a zip file.
     */
    ZipFile(const QString& fileName);

    /*! \brief Destructor */
    ~ZipFile();



    //
    // Getter Methods
    //

    /*! \brief List of files in the zip archive
     *
     *  @returns List of files in the zip archive.
     */
    [[nodiscard]] QStringList fileNames() const { return m_fileNames; }

    /*! \brief Content of file in the zip archive
     *
     *  @param index Index of the file in the list returned by fileNames
     *
     *  @returns Content of file, or a Null array in case of error.
     */
    [[nodiscard]] QByteArray extract(qsizetype index);

    /*! \brief Content of file in the zip archive
     *
     *  If a file with the given name does not exist in the archive, then
     *  the method tries to replace the path separator characters '/' in the
     *  file name with windows-style separators '\' and checks if a file
     *  with that name exists.
     *
     *  @param fileName File name
     *
     *  @returns Content of file, or a Null array in case of error
     */
    [[nodiscard]] QByteArray extract(const QString& fileName);


    //
    // Static methods
    //

    /*! \brief Mime type for files that can be opened by this class
     *
     *  @returns Name of mime type
     */
    [[nodiscard]] static QStringList mimeTypes() { return {u"application/zip"_qs}; }

private:
    Q_DISABLE_COPY_MOVE(ZipFile)

    void* m_zip {nullptr};
    QSharedPointer<QFile> m_file;
    QStringList m_fileNames;
    QList<qsizetype> m_fileSizes;
};

} // namespace FileFormats
