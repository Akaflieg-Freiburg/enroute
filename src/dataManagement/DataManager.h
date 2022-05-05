/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include "GlobalObject.h"
#include "dataManagement/DownloadableGroup.h"


namespace DataManagement {

/*! \brief Manages the list of geographic maps
 *
 *  This class manages a list of remotely available and locally installed
 *  databases and geographic maps.  More specifically, it manages the following
 *  objects.
 *
 *  - Aviation maps (in GeoJSON format)
 *  - Base maps (in MBTILES format)
 *  - FLARM Databases (as a text file)
 *
 *  The class retrieves the list of available objects from a remote server on a
 *  regular basis, updates the list automatically once a week, and maintains a
 *  list of Downloadable objects that corresponds to the remotely available and
 *  locally installed items.
 *
 *  The address of the remote server is hardcoded into the binary. The list is
 *  downloaded to a file "maps.json" in
 *  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation). The
 *  format of the file is described at this URL:
 *
 *  https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/The-file-maps.json
 *
 *  Locally installed items are saved in the directory "aviation_maps" in
 *  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation), or into a
 *  suitable subdirectory of this.
 *
 *  The implementation assumes that no other program interferes with the file
 *  "maps.json" and with the directory "aviation_maps".
 */

class DataManager : public GlobalObject
{
    Q_OBJECT

public:
    /*! \brief Standard constructor
     *
     *  This constructor reads the file "maps.json" and initiates a download of
     *  the file if no file is available or if the last download is more than
     *  one week old.
     *
     *  @param parent The standard QObject parent pointer.
     */
    explicit DataManager(QObject *parent=nullptr);

    // deferred initialization
    void deferredInitialization() override;


    //
    // PROPERTIES
    //

    /*! \brief DownloadableGroupWatcher that holds all aviation maps
     *
     *  Pointer to a DownloadableGroupWatcher that holds all aviation maps. The
     *  maps also appear in geoMaps, which is the union of aviation maps and
     *  base maps.
     */
    Q_PROPERTY(DataManagement::DownloadableGroupWatcher* aviationMaps READ aviationMaps CONSTANT)

    /*! \brief DownloadableGroupWatcher that holds all base maps
     *
     *  Pointer to a DownloadableGroupWatcher that holds all base maps. The maps
     *  also appear in geoMaps, which is the union of aviation maps and base
     *  maps.
     */
    Q_PROPERTY(DataManagement::DownloadableGroupWatcher* baseMaps READ baseMaps CONSTANT)

    /*! \brief DownloadableGroupWatcher that holds all data items
     *
     *  Pointer to a DownloadableGroupWatcher that holds all data items.
     */
    Q_PROPERTY(DataManagement::DownloadableGroupWatcher* databases READ databases CONSTANT)

    /*! \brief Indicates whether the file "maps.json" is currently being
     * downloaded */
    #warning name misleading
    Q_PROPERTY(bool downloadingGeoMapList READ downloadingGeoMapList NOTIFY downloadingGeoMapListChanged)

    /*! \brief DownloadableGroupWatcher that holds all geographic maps
     *
     *  Pointer to a DownloadableGroupWatcher that holds all maps.  This is the
     *  union of aviation maps and base maps.
     */
#warning check if correct. Are data items also included?
    Q_PROPERTY(DataManagement::DownloadableGroupWatcher* geoMaps READ geoMaps CONSTANT)

    /*! \brief True if the list of available geo maps has already been
     * downloaded */
#warning also includes data items. might want to change name
    Q_PROPERTY(bool hasGeoMapList READ hasGeoMapList NOTIFY geoMapListChanged)

    /*! \brief Current "what's new" message */
    Q_PROPERTY(QString whatsNew READ whatsNew NOTIFY whatsNewChanged)

    /*! \brief Hash of the current "what's new" message */
    Q_PROPERTY(uint whatsNewHash READ whatsNewHash NOTIFY whatsNewChanged)

    // ------------------------------------------

    /*! \brief Destructor
     *
     *  This destructor purges the download directory "aviation_map", by deleting
     *  all files that do not belong to any of the maps.
     */
    void cleanUp();

    /*! \brief Getter function for the property with the same name

    @returns Property aviationMaps
  */
    auto aviationMaps() -> DataManagement::DownloadableGroupWatcher * { return &_aviationMaps; }

    /*! \brief Getter function for the property with the same name

    @returns Property baseMaps
  */
    auto baseMaps() -> DataManagement::DownloadableGroupWatcher * { return &_baseMaps; };

    /*! \brief Getter function for the property with the same name
   *
   *  @returns Property databases
   */
    auto databases() -> DataManagement::DownloadableGroupWatcher * { return &_databases; };

    /*! \brief Describe installed map
     *
     * This method describes installed GeoJSON map files.
     *
     * @warning The data is only updated after the maps have been parsed in the
     * GeoJSON parsing process. It is therefore possible that the method returns
     * wrong information if it is called directly after a new map has been
     * installed.
     *
     * @param fileName Name of a GeoJSON file.
     *
     * @returns A human-readable HTML string, or an empty string if no data is
     * available
     */
    #warning is docu correct?
    Q_INVOKABLE static auto describeMapFile(const QString& fileName) -> QString;

    /*! \brief Getter function for the property with the same name

    @returns Property downloadingGeoMapList
   */
    [[nodiscard]] auto downloadingGeoMapList() const -> bool { return _maps_json.downloading(); };

    /*! \brief Getter function for the property with the same name

    @returns Property geoMaps
  */
    auto geoMaps() -> DataManagement::DownloadableGroupWatcher * { return &_geoMaps; }

    /*! \brief Getter function for the property with the same name

    @returns hasGeoMapList
   */
    [[nodiscard]] auto hasGeoMapList() const -> bool { return !_geoMaps.downloadables().isEmpty(); }

    /*! \brief Getter function for property of the same name
   *
   * @returns Property lastWhatsNew
   */
    [[nodiscard]] auto whatsNew() const -> QString { return _whatsNew; }

    /*! \brief Getter function for property of the same name
   *
   * @returns Property lastWhatsNewHash
   */
    [[nodiscard]] auto whatsNewHash() const -> uint { return qHash(_whatsNew, 0); }

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
    fails for whatever reason.  Since the DataManager updates the list
    regularly, this signal can be emitted anytime.

    @param message A brief error message of the form "the requested resource is
    no longer available at the server", possibly translated.
  */
    void error(QString message);

    /*! \brief Notification signal for the property with the same name */
    void whatsNewChanged();

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
    Q_DISABLE_COPY_MOVE(DataManager)

    // This method returns a list of files in the download directory that have no
    // corresponding entry in _aviationMaps.
    [[nodiscard]] auto unattachedFiles() const -> QList<QString>;

    // The current whats new string from _aviationMaps.
    QString _whatsNew {};

    // This timer is used to trigger automatic updates. Its signal QTimer::timeout
    // is connected to the slot autoUpdateGeoMapList.
    QTimer _autoUpdateTimer;

    // This Downloadable object manages the central text file that describes the
    // remotely available aviation maps. It is set in the constructor to point to
    // the URL "https://cplx.vm.uni-freiburg.de/storage/enroute/maps.json"
    DataManagement::Downloadable _maps_json;

    // List of geographic maps
    DataManagement::DownloadableGroup _databases;
    DataManagement::DownloadableGroup _geoMaps;
    DataManagement::DownloadableGroup _baseMaps;
    DataManagement::DownloadableGroup _aviationMaps;
};

};
