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

#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <QTimer> 

#include "DownloadableGroup.h"

/*! \brief Manages the list of geographic maps
  
  This class manages a list of available and installed geographic maps.  It
  retrieves the list of geographic maps from a remote server on a regular basis, updates the
  list automatically once a week, and maintains a list of Downloadable objectes
  that correspond to these geographic maps.  It informs the user if updates are available
  for one or several of these geographic maps.
  
  The list of available maps is downloaded from a remote server whose address is hardcoded into the binary. The list is downloaded to a file "maps.json" in
  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation). The format of the file is described at this URL: https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/The-file-maps.json

  Those geographic maps that are downloaded will be installed into the directory "aviation_maps" in
  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation), or into a suitable subdirectory of this.

  This class assumes that no other program interferes with the file "maps.json" and with the directory "aviation_maps".
*/

class MapManager : public QObject
{
  Q_OBJECT
  
public:
  /*! \brief Standard constructor
    
    This constructor reads the file "maps.json" and initiates a download of the
    file no file is available.  If the last download is more than one week old,
    the class will try once per hour to download the latest version of
    "maps.json".
    
    @param networkAccessManager Pointer to a QNetworkAccessManager that will be
    used for network access. The QNetworkAccessManager must not be deleted while this object exists.
    
    @param parent The standard QObject parent pointer.
  */
  explicit MapManager(QNetworkAccessManager *networkAccessManager, QObject *parent=nullptr);

  // No copy constructor
  MapManager(MapManager const&) = delete;
  
  // No assign operator
  MapManager& operator =(MapManager const&) = delete;
  
  // No move constructor
  MapManager(MapManager&&) = delete;
  
  // No move assignment operator
  MapManager& operator=(MapManager&&) = delete;
  
  /*! \brief Destructor
    
    This destructor purges the download directory "aviation_map", by deleting
    all files that do not belong to any of the maps.
  */
  ~MapManager();
  
  /*! \brief Determines whether some of the installed geographic maps can be updated */
  Q_PROPERTY(bool geoMapUpdatesAvailable READ geoMapUpdatesAvailable NOTIFY geoMapUpdatesAvailableChanged)
  
  /*! \brief Getter function for the property with the same name

    @returns Property geoMapUpdatesAvailable
  */
  bool geoMapUpdatesAvailable() const { return _geoMaps.isUpdatable(); }
  
  /*! \brief Gives an estimate for the download size, as a localized string */
  Q_PROPERTY(QString geoMapUpdateSize READ geoMapUpdateSize NOTIFY geoMapUpdatesAvailableChanged)
  
  /*! \brief Getter function for the property with the same name

    @returns Property geoMapUpdateSize
  */
  QString geoMapUpdateSize() const;

  /*! \brief List of available aviation maps
    
    @returns a QList that contains pointers to all known aviation maps as values. The aviation maps are owned by this map manager and
    must not be deleted. The lifetime of the aviation maps is not guaranteed, so
    if you must store them, then store them in a QPointer and check validity of
    the pointer before every use.
  */
  QList<Downloadable*> aviationMaps() const;
  
  /*! \brief List of available aviation maps, as a list of QObjects
    
    This property is identical to aviationMaps, but returns the pointers to the
    actual maps in the form of a QObjectList instead of a QMap
  */
  Q_PROPERTY(QList<QObject*> aviationMapsAsObjectList READ aviationMapsAsObjectList NOTIFY geoMapListChanged)

  /*! \brief Getter function for the property with the same name

    @returns Property aviationMapsAsObjectList
  */
  QList<QObject*> aviationMapsAsObjectList() const;

  /*! \brief True if at least one aviation map is installed */
  Q_PROPERTY(bool hasAviationMap READ hasAviationMap NOTIFY geoMapListChanged)
  
  /*! \brief Getter function for the property with the same name

    @returns hasAviationMap
   */
  bool hasAviationMap() const;
  
  /*! \brief List of available base maps

    @returns a QMap that contains pointers to all known base maps as values, and
    map names as keys. The aviation maps are owned by this map manager and must
    not be deleted. The lifetime of the maps is not guaranteed, so if you must
    store them, then store them in a QPointer and check validity of the pointer
    before every use.
  */
  QList<Downloadable *> baseMaps() const;
  
  /*! \brief List of available aviation maps, as a list of QObjects
    
    This property is identical to aviationMaps, but returns the pointers to the
    actual maps in the form of a QObjectList instead of a QMap
  */
  Q_PROPERTY(QList<QObject*> baseMapsAsObjectList READ baseMapsAsObjectList NOTIFY geoMapListChanged)

  /*! \brief Getter function for the property with the same name

    @returns baseMapsAsObjectList
  */
  QList<QObject*> baseMapsAsObjectList() const;
  
  /*! \brief True if at least one aviation map is installed */
  Q_PROPERTY(bool hasBaseMap READ hasBaseMap NOTIFY geoMapListChanged)
  
  /*! \brief Getter function for the property with the same name

    @returns hasBaseMap
  */
  bool hasBaseMap() const;
  
  /*! \brief True if list of available geo maps has already been downloaded */
  Q_PROPERTY(bool hasGeoMapList READ hasGeoMapList NOTIFY geoMapListChanged)
  
  /*! \brief Getter function for the property with the same name
    
    @returns hasGeoMapList
   */
  bool hasGeoMapList() const { return !_geoMaps.downloadables().isEmpty(); }

  /*! \brief Indicates whether the file "maps.json" is currently being downloaded */
  Q_PROPERTY(bool downloadingGeoMapList READ downloadingGeoMapList NOTIFY downloadingGeoMapListChanged)
  
  /*! \brief Getter function for the property with the same name

    @returns Property downloadingGeoMapList
   */
  bool downloadingGeoMapList() const;

  /*! \brief Set of all mbtiles files that have been downloaded and are ready-to-use */
  Q_PROPERTY(QSet<QString> mbtileFiles READ mbtileFiles NOTIFY mbtileFilesChanged)

  /*! \brief Getter function for the property with the same name

    @returns Property mbtileFiles
   */
  QSet<QString> mbtileFiles() const;

public slots:
  /*! \brief Triggers an update of the list of available maps

    This will trigger a download the file maps.json from the remote server.
  */
  void updateGeoMapList();
  
  /*! \brief Triggers an update of every updatable map */
  void updateGeoMaps();
  
signals:
  /*! \brief Warning that the list of available aviation maps is about to change
    
    This signal is emitted once the download of the text file describing list of
    available aviation maps finished, just before the list is overwritten with
    new data. It indicates that all users should stop using the list
    immediately. This signal is always followed by the signal
    aviationMapsChanged(), which indicates that the list can be used again.
    
    @see aviationMapsChanged()
  */
  void aboutToChangeAviationMapList();
  
  /*! \brief Notification signal for the property with the same name */
  void geoMapListChanged();
  
  /*! \brief Notification signal for the property with the same name */
  void geoMapUpdatesAvailableChanged();
  
  /*! \brief Notification signal for the property with the same name */
  void downloadingGeoMapListChanged();
  
  /*! \brief Download error
    
    This signal is emitted if the download process for the list of available
    maps fails for whatever reason.  Since the MapManager updates the list
    regularly, this signal can be emitted anytime.
    
    @param message A brief error message of the form "the requested resource is
    no longer available at the server", possibly translated.
  */
  void error(QString message);

  /*! \brief Notification signal for the property with the same name
    
    @param newSet The current set of mbtile files

    @param path Constant string "osm"
  */
  void mbtileFilesChanged(QSet<QString> newSet, QString path);
							     
private slots:
  // Trivial method that re-sends the signal, but without the parameter
  // 'objectName'
  void errorReceiver(const QString& objectName, QString message);

  // This slot is called when a local file of one of the geo maps changes content or existence. If the geo map in question is unsupported, it is then removed. The signals aviationMapListChanged() and
  // aviationMapUpdatesAvailableChanged() are emitted as appropriate.
  void localFileOfGeoMapChanged();

  // This slot is called when the file "maps.json" is meant to be read.  It is
  // called by the constructor to interpret an existing "maps.json". It is also
  // connected to the signal &Downloadable::localFileChanged of
  // _availableMapsDescription, which is emitted whenever the file "maps.json"
  // changes in the file system.
  void readGeoMapListFromJSONFile();

  // This method records the current time as the time when the last update
  // succeeded, and sets the autoUpdateTimer to check again in one day. This
  // slot is connected to the signal &Downloadable::localFileChanged of
  // _availableMapsDescription, which is emitted whenever the file "maps.json"
  // changes in the file system.
  void setTimeOfLastUpdateToNow();

  // This method calls 'updateGeoMapList()' if an automatic update is due. It also sets a reasonable timeout value the timer _autoUpdateTime to fire up when the next autmatic update is due. It will then start the timer.
  void autoUpdateGeoMapList();
  
private:
  // This method returns a list of files in the download directory that have no
  // corresponding entry in _aviationMaps.
  QList<QString> unattachedFiles() const;

  // This timer is used to trigger automatic updates. Its signal QTimer::timeout
  // is connected to the slot autoUpdateGeoMapList.
  QTimer _autoUpdateTimer;

  // This Downloadable object manages the central text file that describes the
  // remotely available aviation maps. It is set in the constructor to point to
  // the URL "https://cplx.vm.uni-freiburg.de/storage/enroute/maps.json"
  QPointer<Downloadable> _maps_json;
  
  // List of geographic maps
  DownloadableGroup _geoMaps;

  // Aviation maps (currently, these are GeoJSON files)
  DownloadableGroup _aviationMaps;

  // Pointer the QNetworkAccessManager that will be used for all Downloadable
  // objects constructed by this class
  QPointer<QNetworkAccessManager> _networkAccessManager;
};

#endif // MAPMANAGER_H
