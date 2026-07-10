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

#include <QFile>
#include <QSharedPointer>

#include "fileFormats/DataFileAbstract.h"
#include "geomaps/VAC.h"


namespace FileFormats
{

/*! \brief Collection of visual approach charts
 *
 *  This class reads collections of visual approach charts, as distributed by
 *  the enroute data server. Internally, these collections are SQLite databases
 *  with the following schema.
 *
 *  - Table 'metadata', with columns 'key' and 'value'. The required entries
 *    are ('schemaVersion', '1') and ('name', <collection name>). The optional
 *    entry ('attribution', <html>) names the source of the charts.
 *
 *  - Table 'charts', with columns 'name', 'topLeftLat', 'topLeftLon',
 *    'topRightLat', 'topRightLon', 'bottomLeftLat', 'bottomLeftLon',
 *    'bottomRightLat', 'bottomRightLon' and 'image'. The column 'image'
 *    contains webp-encoded raster data.
 *
 *  The constructor reads the chart index, but no raster data, and is therefore
 *  lightweight even for large collections.
 */

class VACCollection : public DataFileAbstract
{

public:
    /*! \brief Standard constructor
     *
     *  Constructs an object from a VAC collection file. The file is supposed
     *  to exist and remain intact throughout the existence of this class
     *  instance.
     *
     *  @param fileName Name of the VAC collection file
     */
    VACCollection(const QString& fileName);

    /*! \brief Standard destructor */
    ~VACCollection();

    /*! \brief Charts contained in the collection
     *
     *  The VACs returned here have their member 'fileName' set to the file
     *  name of the collection file, and their member 'collection' set to the
     *  name of this collection. Use imageData() to obtain raster data.
     *
     *  @returns List of VACs, or an empty list on error.
     */
    [[nodiscard]] QList<GeoMaps::VAC> charts() const { return m_charts; }

    /*! \brief Attribution for the charts of this collection
     *
     *  Collections distributed by the enroute data server name the agency that
     *  publishes the charts, along with the license under which the charts are
     *  distributed. The attribution is set by the data server and is therefore
     *  not translated.
     *
     *  @returns A human-readable HTML string with attribution, or an empty
     *  string if the collection file does not specify one.
     */
    [[nodiscard]] QString attribution() const { return m_attribution; }

    /*! \brief Name of the collection
     *
     *  @returns Human-readable name (e.g. "France"), or an empty string on
     *  error.
     */
    [[nodiscard]] QString name() const { return m_name; }

    /*! \brief Retrieve raster data for a single chart
     *
     *  @param chartName Name of the chart, as found in charts()
     *
     *  @returns A QByteArray with webp-encoded raster data, or an empty
     *  QByteArray on error.
     */
    [[nodiscard]] QByteArray imageData(const QString& chartName);

private:
    Q_DISABLE_COPY_MOVE(VACCollection)

    // Name of the VAC collection file
    QString m_fileName;
    QSharedPointer<QFile> m_file;

    // Name of the data base connection. This name is unique to each instance of
    // this class, and should therefore not be copied.
    QString m_databaseConnectionName;

    QString m_name;
    QString m_attribution;
    QList<GeoMaps::VAC> m_charts;
};

} // namespace FileFormats
