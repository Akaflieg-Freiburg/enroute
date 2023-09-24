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

#include <QJsonArray>

#include "geomaps/Zip.h"

namespace GeoMaps
{

/*! \brief Trip Kit
 *
 *  This class implements a Trip Kit reader. Trip kits are ZIP files
 *  that contain georeferences VACs and JSON files that describe the
 *  georeferencing.
 */
class TripKit {
public:
    /*! \brief Constructor
     *
     *  The constructor opens the trip kit and reads the JSON file,
     *  but does not yet extract any images.
     *
     *  \param fileName File name of a trip kit.
     */
    TripKit(const QString& fileName);


    //
    // Getter Methods
    //

    /*! \brief Error message
     *
     *  If the trip kit is invalid, this method contains a short
     *  explanation.
     *
     *  @returns A human-readable, translated warning or an empty string if no
     *  error.
     */
    [[nodiscard]] auto error() const -> QString { return m_error; }

    /*! \brief Test for validity
     *
     *  @returns True if this trip kit appears to be valid
     */
    [[nodiscard]] auto isValid() const -> bool { return m_error.isEmpty(); }

    /*! \brief Name of the trip kit, as specified in the JSON file
     *
     *  @returns The name or an empty string in case of error.
     */
    [[nodiscard]] auto name() const -> QString { return m_name; }

    /*! \brief Number of VACs in this trip kit
     *
     *  @returns The number of VACs in this trip kit
     */
    [[nodiscard]] auto numCharts() const -> qsizetype { return m_charts.size(); }

    //
    // Methods
    //

    /*! \brief Extract visual approach chart
     *
     *  This method copies the visual approach chart to a new directory, chooses
     *  an appropriate file name of the form
     *  "baseName-geo_7.739665_48.076416_7.9063883_47.96452.webp". If the image
     *  is not in webp format already, it will be converted to webp using a
     *  lossy encoder. This method is slow.
     *
     *  @param directoryPath Name of a directory where the VAC will be stored.
     *  The directory and its parents are created if necessary
     *
     *  @param index Index of the VAC that is to be extracted
     *
     *  @returns Path of the newly created file, or an empty string in case of
     *  error.
     */
    auto extract(const QString &directoryPath, qsizetype index) -> QString;

private:
    GeoMaps::Zip m_zip;
    QString m_error;
    QString m_name;
    QJsonArray m_charts;
};

} // namespace GeoMaps
