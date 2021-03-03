/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include <QTimer> 

#include "geomaps/DownloadableGroup.h"


namespace GeoMaps {

/*! \brief Manages the list of geographic maps
  
  This class manages a list of available and installed geographic maps.  It
  retrieves the list of geographic maps from a remote server on a regular basis,
  updates the list automatically once a week, and maintains a list of
  Downloadable objectes that correspond to these geographic maps.  It informs
  the user if updates are available for one or several of these geographic maps.
  
  The list of available maps is downloaded from a remote server whose address is
  hardcoded into the binary. The list is downloaded to a file "maps.json" in
  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation). The format
  of the file is described at this URL:
  https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/The-file-maps.json

  Those geographic maps that are downloaded will be installed into the directory
  "aviation_maps" in
  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation), or into a
  suitable subdirectory of this.

  This class assumes that no other program interferes with the file "maps.json"
  and with the directory "aviation_maps".
*/

class MapManager : public QObject
{
  Q_OBJECT
  
public:
  /*! \brief Standard constructor
    
    This constructor reads the file "maps.json" and initiates a download of the
    file if no file is available or if the last download is more than one week
    old.
    
    @param networkAccessManager Pointer to a QNetworkAccessManager that will be
    used for network access. The QNetworkAccessManager must not be deleted while
    this object exists.
    
    @param parent The standard QObject parent pointer.
  */
  explicit MapManager(QNetworkAccessManager *networkAccessManager, QObject *parent=nullptr);
  
  /*! \brief Destructor
    
    This destructor purges the download directory "aviation_map", by deleting
    all files that do not belong to any of the maps.
  */
  ~MapManager();
  
  /*! \brief Pointer to the DownloadableGroup that holds all aviation maps

    The is a DownloadableGroupWatcher that holds all aviation maps. The maps
    also appear in geoMaps, which is the union of aviation maps and base maps.
  */
  Q_PROPERTY(DownloadableGroupWatcher *aviationMaps READ aviationMaps CONSTANT)

  /*! \brief Getter function for the property with the same name
    
    @returns Property aviationMaps
  */
  DownloadableGroupWatcher *aviationMaps() { return &_aviationMaps; }
  
  /*! \brief Pointer to the DownloadableGroup that holds all base maps

    The is a DownloadableGroup that holds all base maps. The maps also appear in
    geoMaps, which is the union of aviation maps and base maps.
  */
  Q_PROPERTY(DownloadableGroupWatcher *baseMaps READ baseMaps CONSTANT)

  /*! \brief Getter function for the property with the same name

    @returns Property baseMaps
  */
  DownloadableGroupWatcher *baseMaps() { return &_baseMaps; };

  /*! \brief Indicates whether the file "maps.json" is currently being downloaded */
  Q_PROPERTY(bool downloadingGeoMapList READ downloadingGeoMapList NOTIFY downloadingGeoMapListChanged)

  /*! \brief Getter function for the property with the same name

    @returns Property downloadingGeoMapList
   */
  bool downloadingGeoMapList() const { return _maps_json.downloading(); };

  /*! \brief Pointer to group of all geographic maps

    The is a DownloadableGroup that holds all maps.  This is the union of
    aviation maps and base maps.
  */
  Q_PROPERTY(DownloadableGroupWatcher *geoMaps READ geoMaps CONSTANT)

  /*! \brief Getter function for the property with the same name

    @returns Property geoMaps
  */
  DownloadableGroupWatcher *geoMaps() { return &_geoMaps; }

  /*! \brief True if the list of available geo maps has already been downloaded */
  Q_PROPERTY(bool hasGeoMapList READ hasGeoMapList NOTIFY geoMapListChanged)
  
  /*! \brief Getter function for the property with the same name
    
    @returns hasGeoMapList
   */
  bool hasGeoMapList() const { return !_geoMaps.downloadables().isEmpty(); }

public slots:
  /*! \brief Triggers an update of the list of available maps

    This will trigger a download the file maps.json from the remote server.
  */
  void updateGeoMapList();
  
signals:
  /*! \brief Notification signal for the property with the same name */
  void geoMapListChanged();
  
  /*! \brief Notification signal for the property with the same name */
  void downloadingGeoMapListChanged();

  /*! \brief Download error
    
    This signal is emitted if the download process for the file "maps.json"
    fails for whatever reason.  Since the MapManager updates the list regularly,
    this signal can be emitted anytime.
    
    @param message A brief error message of the form "the requested resource is
    no longer available at the server", possibly translated.
  */
  void error(QString message);

private slots:
  // Trivial method that re-sends the signal, but without the parameter
  // 'objectName'
  void errorReceiver(const QString& objectName, QString message);

  // This slot is called when a local file of one of the geo maps changes
  // content or existence. If the geo map in question is unsupported, it is then
  // removed. The signals aviationMapListChanged() and
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

  // This method calls 'updateGeoMapList()' if an automatic update is due. It
  // also sets a reasonable timeout value the timer _autoUpdateTime to fire up
  // when the next autmatic update is due. It will then start the timer.
  void autoUpdateGeoMapList();
  
private:
  Q_DISABLE_COPY_MOVE(MapManager)
  
  // This method returns a list of files in the download directory that have no
  // corresponding entry in _aviationMaps.
  QList<QString> unattachedFiles() const;

  // This timer is used to trigger automatic updates. Its signal QTimer::timeout
  // is connected to the slot autoUpdateGeoMapList.
  QTimer _autoUpdateTimer;

  // This Downloadable object manages the central text file that describes the
  // remotely available aviation maps. It is set in the constructor to point to
  // the URL "https://cplx.vm.uni-freiburg.de/storage/enroute/maps.json"
  Downloadable _maps_json;
  
  // List of geographic maps
  DownloadableGroup _geoMaps;
  DownloadableGroup _baseMaps;
  DownloadableGroup _aviationMaps;

  // Pointer the QNetworkAccessManager that will be used for all Downloadable
  // objects constructed by this class
  QPointer<QNetworkAccessManager> _networkAccessManager;
};

};
