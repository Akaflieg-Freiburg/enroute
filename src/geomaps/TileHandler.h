/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include <QSet>
#include <QSqlDatabase>

#include <qhttpengine/handler.h>


namespace GeoMaps {

class Downloadable;


/*! \brief Implementation of QHttpEngine::Handler that serves mbtile files
 
  This class is the core of the tileserver. It takes one or more mbtile files
  and serves the individual tiles as well as TileJSON that describes the
  source. Once the tileHandler is installed, a TileJSON file (following the
  TileJSON Specification 2.2.0 found
  https://github.com/mapbox/tilejson-spec/tree/master/2.2.0) is served at the
  URL whose names is set in the baseURLName argument of the constructor.
*/

class TileHandler : public QHttpEngine::Handler
{
  Q_OBJECT
  
public:
  /*! \brief Create a new  tile handler
   
    This constructor sets up a new tile handler.
    
    @param mbtileFiles A list of Downloadable whose files are expected to
    conform to the MBTiles Specification 1.3
    (https://github.com/mapbox/mbtiles-spec/blob/master/1.3/spec.md).  Whenever
    the Downloadble emits the signal aboutToChange to indicate that the file
    will change on the dist, the TileHandler drops to files from its list. There
    are not re-added if the file is restored or overwritten later.  The tile
    files are expected to agree in their metadata, and the metadata
    (attribution, description, format, name, minzoom, maxzoom) is read only from
    one of the files (a random one, in fact). If a tile is contained in more
    than one of the files, the data is expected to be identical in each of the
    files.
    
    @param baseURLName The name of the URL under which the tile server allows
    access to this tile. Typically a string of the form
    "http://localhost:8080/osm"
    
    @param parent The standard QObject parent
  */
  explicit TileHandler(const QVector<QPointer<GeoMaps::Downloadable>>& mbtileFiles, const QString& baseURLName, QObject *parent = nullptr);
  
  // Destructor
  ~TileHandler() override;
  
  /*! \brief Attribution property, as found in the metadata table of the mbtile file
    
    This property is empty if no attribution is found.
  */
  Q_PROPERTY(QString attribution READ attribution CONSTANT)
  
  /*! \brief Getter function for property with the same name
    
    @returns Property attribution
  */
  QString attribution() const {return _attribution;}
  
  /*! \brief Description property, as found in the metadata table of the mbtile file
    
    This property is empty if no description is found.
  */
  Q_PROPERTY(QString description READ description CONSTANT)
  
  /*! \brief Getter function for property with the same name
    
    @returns Property description
  */
  QString description() const {return _description;}
  
  /*! \brief Format property, as found in the metadata table of the mbtile file */
  Q_PROPERTY(QString format READ format CONSTANT)
  
  /*! \brief Getter function for property with the same name
    
    @returns Property format
  */
  QString format() const {return _format;}
  
  /*! \brief Maxzoom property, as found in the metadata table of the mbtile file
    
    This property is set to -1 if no maxversion is found.
  */
  Q_PROPERTY(int maxzoom READ maxzoom CONSTANT)

  /*! \brief Getter function for property with the same name
    
    @returns Property maxzoom
  */
  int maxzoom() const {return _maxzoom;}
  
  /*! \brief Minzoom property, as found in the metadata table of the mbtile file
    
    This property is set to -1 if no maxversion is found.
  */
  Q_PROPERTY(int minzoom READ minzoom CONSTANT)
  
  /*! \brief Getter function for property with the same name
    
    @returns Property minzoom
  */
  int minzoom() const {return _minzoom;}
  
  /*! \brief Name property, as found in the metadata table of the mbtile file */
  Q_PROPERTY(QString name READ name CONSTANT)
  
  /*! \brief Getter function for property with the same name 
    
    @returns Property name
  */
  QString name() const {return _name;}
  
  /*! \brief TileJSON source
    
    This property holds a TileJSON file that describes the source. The file
    complies with specification 2.2.0
    (https://github.com/mapbox/tilejson-spec/tree/master/2.2.0).
  */
  Q_PROPERTY(QByteArray tileJSON READ tileJSON CONSTANT)
  
  /* \brief Getter function for property with the same name

     @returns Property tileJSON
  */
  QByteArray tileJSON() const;
  
  /*! \brief Tile URL endpoints
    
    Tile endpoint URL, as specified in the TileJSON Specification 2.2.0
    (https://github.com/mapbox/tilejson-spec/tree/master/2.2.0). This is a
    string that might look like "http://localhost:8080/osm//{z}/{x}/{y}.pbf"
  */
  Q_PROPERTY(QString tiles READ tiles CONSTANT)
  
  /* \brief Getter function for property with the same name

     @returns Property tiles
  */
  QString tiles() const {return _tiles;}
  
  /*! \brief Version property, as found in the metadata table of the mbtile file
    
    This property is empty if no version is found.
  */
  Q_PROPERTY(QString version READ version CONSTANT)

  /*! \brief Getter function for property with the same name
   
    @returns Property version
  */
  QString version() const {return _version;}
  
protected:
  /*
   * @brief Reimplementation of
   * [Handler::process()](QHttpEngine::Handler::process)
   */
  void process(QHttpEngine::Socket *socket, const QString &path) override;
  
private slots:
  // This slot is connected to aboutToChangeLocalFile of the Downloadables, in
  // order to make sure that databases are closed before the file changes.
  void removeFile(const QString& localFileName);

private:
  Q_DISABLE_COPY_MOVE(TileHandler)

  QSet<QString> databaseConnections;
  
  QString _name;
  QString _format;
  QString _tiles;
  QString _description;
  
  QString _version;
  
  QString _attribution;
  
  int _maxzoom {-1};
  int _minzoom {-1};
  
  bool hasDBError {false};
};

};
