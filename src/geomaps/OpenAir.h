/***************************************************************************
 *   Copyright (C) 2023 by Heinz Blöchinger                                *
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

#include <QJsonDocument>

namespace GeoMaps
{

class openAir : public QObject
{
    Q_OBJECT

public:
    /*! \brief Check if file contains valid OpenAIR data
     *
     *  @param fileName Name of a file
     *
     *  @param info Pointer to a string where additional information will be stored.
     *
     *  @returns True if the file is likely to contain valid OpenAIR data.
     */
    static bool isValid(const QString &fileName, QString* info=nullptr);

    /*! \brief Check if file contains valid OpenAIR data
     *
     *  @param fileName Name of a file
     *
     *  @returns Translated HTML string with warnings.
     */
    static QString warnings(const QString &fileName);

    /*! \brief Reads a file in openAIR format and returns a GeoJSON document
     *
     *  @param fileName Name of the openAIR file
     *
     *  @param error Reference to a QStringList where error messages will be appended.
     *
     *  @return If no error messages were appended, returns a QJsonDocument with GeoJSON as specified
     *  in https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation.
     *  If error messages were appended, returns an empty QJsonDocument
     */
    static QJsonDocument parse(const QString& fileName, QStringList& errorList, QStringList& warningList);
};


} // namespace GeoMaps
