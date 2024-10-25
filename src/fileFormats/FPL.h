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

#include <QGeoCoordinate>

#include "DataFileAbstract.h"


namespace FileFormats
{

/*! \brief FPL file support class
     *
     *  The methods of this class read FPL files.
     */

class FPL : public DataFileAbstract
{

public:
    /*! \brief Constructor
         *
         *  This method reads a FPL file.
         *
         *  @param fileName Name of a FPL file
         */
    FPL(const QString& fileName);



    //
    // Getter Methods
    //

    /*! \brief Waypoints specified in the FPL file
         *
         *  The QGeoCoordinates returned here are guaranteed to be valid.
         *
         *  @returns Waypoints specified in the FPL file
         */
    [[nodiscard]] QVector<QGeoCoordinate> waypoints() const { return m_waypoints; }


    //
    // Static methods
    //

    /*! \brief Mime type for files that can be opened by this class
         *
         *  @returns Name of mime type
         */
#warning Needs to be implemented. The type "text/FPL" does probably not exist.
    [[nodiscard]] static QStringList mimeTypes() { return {u"text/xml"_qs, u"text/plain"_qs}; }

private:
    QVector<QGeoCoordinate> m_waypoints;
};

} // namespace FileFormats
