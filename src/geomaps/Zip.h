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

#include <QStringList>


namespace GeoMaps
{
/*! \brief ZIP Archive
 *
 *  This class reads a ZIP file and allows extracting individual files.
 */

class Zip {
public:
    /*! \brief Constructor
     *
     *  The constructor opens the ZIP file. The class assumes that the ZIP file
     *  remains untouched throughout the life cycle of this instance.
     *
     *  \param fileName File name of a zip file.
     */
    Zip(const QString& fileName);

    /*! \brief Destructor */
    ~Zip();

    /*! \brief Test for validity
     *
     *  @returns True if the file named in the constructor appears to be a valid zip file.
     */
    [[nodiscard]] auto isValid() const -> bool { return (m_zip != nullptr); }

    /*! \brief List of files in the zip archive
     *
     *  @returns List of file named.
     */
    [[nodiscard]] auto fileNames() const -> QStringList { return m_fileNames; }

    /*! \brief Content of file in the zip archive
     *
     *  @param index Index of the file in the list returned by fileNames
     *
     *  @returns Content of file, or a Null array in case of error
     */
    auto extract(qsizetype index) -> QByteArray;

    /*! \brief Content of file in the zip archive
     *
     *  @param fileName File name
     *
     *  @returns Content of file, or a Null array in case of error
     */
    auto extract(const QString &fileName) -> QByteArray;

private:
    Q_DISABLE_COPY_MOVE(Zip)

    void* m_zip {nullptr};
    QStringList m_fileNames;
    QList<qsizetype> m_fileSizes;
};

} // namespace GeoMaps
