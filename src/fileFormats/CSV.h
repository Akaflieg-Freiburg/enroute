/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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

#include "DataFileAbstract.h"

using namespace Qt::Literals::StringLiterals;


namespace FileFormats
{

    /*! \brief CSV file support class
     *
     *  The methods of this class read CSV files.
     */

    class CSV : public DataFileAbstract
    {

    public:
        /*! \brief Constructor
         *
         *  This method reads a CSV file.
         *
         *  @param fileName Name of a CSV file
         */
        CSV(const QString& fileName);



        //
        // Getter Methods
        //

        /*! \brief Waypoints specified in the CUP file
         *
         *  @returns Waypoints specified in the CUP file
         */
        [[nodiscard]] QVector<QStringList> lines() const { return m_lines; }


        //
        // Static methods
        //

        /*! \brief Mime type for files that can be opened by this class
         *
         *  @returns Name of mime type
         */
        [[nodiscard]] static QStringList mimeTypes() { return {u"text/csv"_s, u"text/plain"_s}; }

    private:
        // Private helper functions
        static QStringList parseCSV(const QString& string);

        QVector<QStringList> m_lines;
    };

} // namespace FileFormats
