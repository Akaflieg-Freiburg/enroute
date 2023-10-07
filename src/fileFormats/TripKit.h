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

#include <QGeoCoordinate>
#include <QJsonArray>

#include "fileFormats/ZipFile.h"

namespace FileFormats
{

/*! \brief Trip Kit
 *
 *  This class implements a Trip Kit reader. Trip kits are ZIP files
 *  that contain georeferences VACs and JSON files that describe the
 *  georeferencing.
 */
class TripKit : public DataFileAbstract
{
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

    /*! \brief Name of the trip kit, as specified in the JSON file
     *
     *  @returns The name or an empty string in case of error.
     */
    [[nodiscard]] QString name() const { return m_name; }

    /*! \brief Number of VACs in this trip kit
     *
     *  @returns The number of VACs in this trip kit
     */
    [[nodiscard]] qsizetype numCharts() const { return m_entries.size(); }



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
     *  @returns Path of the extracted VAC, or an empty string in case of error.
     */
    [[nodiscard]] QString extract(const QString& directoryPath, qsizetype index);


    //
    // Static methods
    //

    /*! \brief Mime type for files that can be opened by this class
     *
     *  @returns Name of mime type
     */
    [[nodiscard]] static QStringList mimeTypes() { return FileFormats::ZipFile::mimeTypes(); }

private:
    Q_DISABLE_COPY_MOVE(TripKit)

    // Fills the list m_entries, returns an error message or an empty string.
    QString readTripKitData();

    // Check if the ZIP file contains image file with coordinates in the file name
    void readVACs();

    struct chartEntry
    {
        QString name; // Name of the chart e.g. "EDTF"
        QString ending; // file path ending e.g. "webp" or "tiff". Will be in lower case, might be empty
        QString path; // Path of the chart within the ZIP file
        QGeoCoordinate topLeft;
        QGeoCoordinate topRight;
        QGeoCoordinate bottomLeft;
        QGeoCoordinate bottomRight;
    };
    QList<TripKit::chartEntry> m_entries;

    FileFormats::ZipFile m_zip;
    QString m_name;
};

} // namespace GeoMaps
