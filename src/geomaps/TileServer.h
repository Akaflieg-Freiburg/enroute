/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

#include "geomaps/MBTILES.h"
#include "geomaps/TileHandler.h"

#include <QAbstractHttpServer>
#include <QSharedPointer>


namespace GeoMaps {

/*! \brief HTTP server for mapbox' MBTiles files
 *
 *  This class features a simple HTTP server that serve the following files to
 *  http://127.0.0.1:xxxx/path.
 *
 *  - If path is a file name, the server checks if the file is available in the
 *    resource system of the app and serves that file.
 *  - If path equals "aviationData.geojson", the server returns a GeoJSON
 *    document that contains the full aviation data, as provided by
 *    GlobalObject::geoMapProvider()->geoJSON().
 *  - If path equals the base name of an MBTilesFileSet, then the server returns
 *    a JSON document describing the MBTiles.
 *  - If path equals "baseName/z/x/y.XXX", then the server returns an individual
 *    tile from the tile set.
 *
 *  This server is able to handle several sets of MBTiles files. Each set is
 *  available under a single URL, and the MBTile files in each set are expected
 *  to contain identical metadata. To serve a tile request, the server goes
 *  through the MBTiles files in random order and serves the first tile it can
 *  find. Tiles contained in more than one file need thus to be identical. A
 *  typical use case would be a server that contains to sets of files, one set
 *  with vector tiles containing openstreetmap data and one set with raster data
 *  used for hillshading. Each set contains two MBTiles files, one for Africa
 *  and one for Europe.
 */

class TileServer : public QAbstractHttpServer
{
  Q_OBJECT
  
public:
  /*! \brief Create a new tile server
   *
   *  The tile server will find a free port and listen to 127.0.0.1:port.  The
   *  method serverUrl() returns the precise Url where the server will be
   *  available.
   *
   *  @param parent The standard QObject parent
   */
  explicit TileServer(QObject* parent = nullptr);
  
  // Standard destructor
  ~TileServer() override = default;
  
  Q_PROPERTY(QString serverUrl READ serverUrl NOTIFY serverUrlChanged)

  /*! \brief URL under which this server is presently reachable
   *
   *  The method returns the Url where the server is listening to incoming
   *  connections. This is typically string of the form "http://127.0.0.1:3470".
   *  If the server is not listening to incoming connections, an empty string is
   *  returned.
   *
   *  @returns URL under which this server is presently reachable
   */
  [[nodiscard]] auto serverUrl() -> QString;

public slots:
  /*! \brief Add a new set of tile files
   *
   *  This method adds a new set of tile files, that will be available under
   *  serverUrl()+"/baseName" (typically, this is a URL of the form
   *  'http://localhost:8080/basename').
   *
   *  @param baseName The path under which the tiles will be available.
   *
   *  @param MBTilesFiles The name of one or more mbtile files on the disk,
   *  which are expected to conform to the MBTiles Specification 1.3
   *  (https://github.com/mapbox/mbtiles-spec/blob/master/1.3/spec.md). These
   *  files must exist until the file set is removed or the sever is destructed,
   *  or else replies to tile requests will yield undefined results. The tile
   *  files are expected to agree in their metadata, and the metadata
   *  (attribution, description, format, name, minzoom, maxzoom) is read only
   *  from one of the files (a random one, in fact). If a tile is contained in
   *  more than one of the files, the data is expected to be identical in each
   *  of the files.
   */
  void addMbtilesFileSet(const QString& baseName, const QVector<QPointer<GeoMaps::MBTILES>>& MBTilesFiles);

  /*! \brief Removes a set of tile files
   *
   *  @param baseName Path of tiles to remove
   */
  void removeMbtilesFileSet(const QString& baseName);

#warning
  void restart();

  signals:
#warning
  void serverUrlChanged();


private:
  Q_DISABLE_COPY_MOVE(TileServer)

  // Implemented pure virtual method from QAbstractHttpServer
  bool handleRequest(const QHttpServerRequest& request, QTcpSocket* socket) override;

  // Implemented pure virtual method from QAbstractHttpServer
  void missingHandler(const QHttpServerRequest& request, QTcpSocket* socket) override;

  // List of tile handlers
  QMap<QString, QSharedPointer<GeoMaps::TileHandler>> m_tileHandlers;

#warning
  bool suspended = false;
};

} // namespace GeoMaps
