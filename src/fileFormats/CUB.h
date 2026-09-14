/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

#include <QJsonDocument>

namespace FileFormats
{

/*! \brief CUB file support class
 *
 *  The methods of this class read airspace files in the binary CUB format used
 *  by Naviter and LXNav products, as specified here:
 *  https://github.com/naviter/seeyou_file_formats/blob/main/CUB_file_format.md
 *
 *  NOTAM-defined airspaces and activation times are not supported. Airspaces of
 *  type NOTAM are ignored; all other airspaces are treated as permanently
 *  active.
 */

class CUB
{
public:
    /*! \brief Check if file contains valid CUB data
     *
     *  @param fileName Name of a file
     *
     *  @param info Pointer to a string where additional information will be stored.
     *
     *  @returns True if the file is likely to contain valid CUB data.
     */
    static bool isValid(const QString& fileName, QString* info=nullptr);

    /*! \brief Reads a file in CUB format and returns a GeoJSON document
     *
     *  @param fileName Name of the CUB file
     *
     *  @param errorList Reference to a QStringList where error messages will be appended.
     *
     *  @param warningList Reference to a QStringList where warnings will be appended.
     *
     *  @return If no error messages were appended, returns a QJsonDocument with GeoJSON as specified
     *  in https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation.
     *  If error messages were appended, returns an empty QJsonDocument
     */
    static QJsonDocument parse(const QString& fileName, QStringList& errorList, QStringList& warningList);
};

} // namespace FileFormats
