/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include "fileFormats/MBTILES.h"

class QHttpServerResponder;

namespace GeoMaps {


/*! \brief Implementation of QHttpEngine::Handler that serves mbtile files
 *
 *  This is a helper clas for TileServer. It gathers a set of MBTiles files.
 *  The method process() takes the path of an incoming HTTP request and uses a
 *  QHttpServerResponder to reply with appropriate tile data, and with TileJSON
 *  (following the TileJSON Specification 2.2.0 found in
 *  https://github.com/mapbox/tilejson-spec/tree/master/2.2.0).
 */

class TileHandler
{

public:
    /*! \brief Create a new tile handler
    *
    *  This constructor sets up a new tile handler.
    *
    *  @param mbtileFiles A list of pointers to MBTiles. The MBTiles are
    *  expected to agree in their metadata, and the metadata (attribution,
    *  description, format, name, minzoom, maxzoom) is read only from one of the
    *  files (a random one, in fact). If a tile is contained in more than one of
    *  the files, the data is expected to be identical in each of the files.
    *
    *  @param baseURLName The name of the URL under which the tile server allows
    *  access to this tile. Typically, this is a string of the form
    *  "http://localhost:8080/osm"
    */
    explicit TileHandler(const QVector<QSharedPointer<GeoMaps::MBTILES>>& mbtileFiles, const QString& baseURLName);

    // Standard descructor
    ~TileHandler() = default;

    /*! \brief Process request
    *
    *  The method processes and answers an incomin HTTP request for TileServer.
    *
    *  @param responder QHttpServerResponder that is used to send the reply.
    *
    *  @param pathElements URL string of the incoming HTTP request. This is the
    *  part of the URL following the baseURLName that was given in the
    *  constructor, split at the '/'.
    *
    *  @return True if the request could be answered.
    */
    bool process(QHttpServerResponder* responder, const QStringList& pathElements);

private:
    Q_DISABLE_COPY_MOVE(TileHandler)

    // List of MBTiles
    QVector<QSharedPointer<GeoMaps::MBTILES>> m_mbtiles;

    // Format of tiles. This is a short string such as "jpg", "pbf", "png" or
    // "webp".
    QString m_format;

    // TileJSON that will be served in appropriate requests.
    QJsonDocument m_tileJSON;
};

} // namespace GeoMaps
