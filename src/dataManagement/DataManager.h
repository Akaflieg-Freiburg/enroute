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

#include <QStandardPaths>

#include "GlobalObject.h"
#include "dataManagement/DownloadableGroup.h"


namespace DataManagement {

/*! \brief Manages the list of geographic maps
 *
 *  This class manages a list of remotely available and locally installed
 *  databases and geographic maps.  More specifically, it manages the following
 *  items.
 *
 *  - Aviation maps (in GeoJSON format)
 *  - Base maps (in MBTILES format)
 *  - FLARM Databases (as a text file)
 *
 *  In addition, it allows access to the following data.
 *
 *  - "What's new"-message from remove server
 *
 *  The class retrieves the list of available items from a remote server on a
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
    explicit DataManager(QObject* parent=nullptr);

    // deferred initialization
    void deferredInitialization() override;


    //
    // PROPERTIES
    //

    /*! \brief DownloadableGroupWatcher that holds all aviation maps
     *
     *  Pointer to a DownloadableGroupWatcher that holds all aviation maps.
     */
    Q_PROPERTY(DataManagement::DownloadableGroupWatcher* aviationMaps READ aviationMaps CONSTANT)

    /*! \brief DownloadableGroupWatcher that holds all base maps in raster format
     *
     *  Pointer to a DownloadableGroupWatcher that holds all base maps in raster format.
     */
    Q_PROPERTY(DataManagement::DownloadableGroupWatcher* baseMapsRaster READ baseMapsRaster CONSTANT)

    /*! \brief DownloadableGroupWatcher that holds all base maps
     *
     *  Pointer to a DownloadableGroupWatcher that holds all base maps.
     */
    Q_PROPERTY(DataManagement::DownloadableGroupWatcher* baseMaps READ baseMaps CONSTANT)

    /*! \brief DownloadableGroupWatcher that holds all data items
     *
     *  Pointer to a DownloadableGroupWatcher that holds all databases.
     */
    Q_PROPERTY(DataManagement::DownloadableGroupWatcher* databases READ databases CONSTANT)

    /*! \brief True while the list of remotely available items is retrieved */
    Q_PROPERTY(bool downloadingRemoteItemList READ downloadingRemoteItemList NOTIFY downloadingRemoteItemListChanged)

    /*! \brief DownloadableGroupWatcher that holds all data items
     *
     *  Pointer to a DownloadableGroupWatcher that holds all data items.  This
     *  includes aviation maps, base maps, and databases.
     */
    Q_PROPERTY(DataManagement::DownloadableGroupWatcher* items READ items CONSTANT)

    /*! \brief True if list of remotely available items has been downloaded */
    Q_PROPERTY(bool hasRemoteItemList READ hasRemoteItemList NOTIFY hasRemoteItemListChanged)

    /*! \brief Current "what's new" message */
    Q_PROPERTY(QString whatsNew READ whatsNew NOTIFY whatsNewChanged)

    /*! \brief Hash of the current "what's new" message */
    Q_PROPERTY(uint whatsNewHash READ whatsNewHash NOTIFY whatsNewChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property aviationMaps
     */
    [[nodiscard]] auto aviationMaps() -> DataManagement::DownloadableGroupWatcher* { return &m_aviationMaps; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property baseMaps
     */
    [[nodiscard]] auto baseMaps() -> DataManagement::DownloadableGroupWatcher* { return &m_baseMaps; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property baseMapsRaster
     */
    [[nodiscard]] auto baseMapsRaster() -> DataManagement::DownloadableGroupWatcher* { return &m_baseMapsRaster; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property baseMapsVector
     */
    [[nodiscard]] auto baseMapsVector() -> DataManagement::DownloadableGroupWatcher* { return &m_baseMapsVector; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property databases
     */
    [[nodiscard]] auto databases() -> DataManagement::DownloadableGroupWatcher* { return &m_databases; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property downloadingRemoteItemList
     */
    [[nodiscard]] auto downloadingRemoteItemList() const -> bool { return m_mapsJSON.downloading(); }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property items
     */
    [[nodiscard]] auto items() -> DataManagement::DownloadableGroupWatcher* { return &m_items; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns hasRemoteItemList
     */
    [[nodiscard]] auto hasRemoteItemList() const -> bool { return m_mapsJSON.hasFile(); }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property whatsNew
     */
    [[nodiscard]] auto whatsNew() const -> QString { return m_whatsNew; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastWhatsNewHash
     */
    [[nodiscard]] auto whatsNewHash() const -> uint { return qHash(m_whatsNew, 0); }


    //
    // Methods
    //

    /*! \brief Describe installed map
     *
     * This method describes a data item, by inspecting the locally installed
     * map file.
     *
     * @warning The description not always available right after installation --
     * information about aviation maps is only updated after the maps have been
     * parsed in the GeoJSON parsing process. It is therefore possible that the
     * method returns wrong information if it is called directly after a new map
     * has been installed.
     *
     * @param fileName File name of locally installed file.
     *
     * @returns A human-readable HTML string, or an empty string if no data is
     * available
     */
    Q_INVOKABLE [[nodiscard]] static QString describeDataItem(const QString& fileName);

public slots:
    /*! \brief Triggers an update of the list of remotely available data items
     *
     *  This will trigger a download the file maps.json from the remote server.
     */
    void updateRemoteDataItemList()
    {
        m_mapsJSON.startFileDownload();
    }

signals:
    /*! \brief Notification signal for the property with the same name */
    void hasRemoteItemListChanged();

    /*! \brief Notification signal for the property with the same name */
    void downloadingRemoteItemListChanged();

    /*! \brief Error message for user
     *
     *  This signal is emitted if an error occurs and the GUI should display a
     *  prominent error message. This signal can be emitted anytime.
     *
     *  @param message A brief, human-readable, translated error message.
     */
    void error(const QString& message);

    /*! \brief Notification signal for the property with the same name */
    void whatsNewChanged();

private:
    Q_DISABLE_COPY_MOVE(DataManager)

    // Clean the data directory.
    //
    // - delete all files with unexpected file names
    // - earlier versions of this program constructed files with names ending in
    //   ".geojson.geojson" or ".mbtiles.mbtiles". We correct those file names
    //   here.
    // - remove all empty sub directories
    void cleanDataDirectory();

    // This slot is called when a local file of one of the Downloadables changes
    // content or existence. If the Downloadable in question has no file
    // anymore, and has an invalid URL, it is then removed.
    void onItemFileChanged();

    // This slot updates the DownloadableGroups as well as the propery
    // 'whatsNew', by reading the file 'maps.json' and by checking the data
    // directory for locally installed, unsupported files.
    void updateDataItemListAndWhatsNew();

    // This method checks if a Downloadable item with the given url and
    // localFileName already exists in _items. If so, it returns a pointer to
    // that item. If not, then a Downloadable with the url and localFileName is
    // created and added to _items. Depending on localFileName, it will also be
    // added to _aviationMap, _baseMaps, or _databases. A point to that item is
    // then returned.
    DataManagement::Downloadable* createOrRecycleItem(const QUrl& url, const QString& localFileName);

    // Full path name of data directory, without trailing slash
    QString m_dataDirectory {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/aviation_maps"};

    // The current whats new string from _aviationMaps.
    QString m_whatsNew {};

    // This Downloadable object manages the central text file that describes the
    // remotely available aviation maps. It is set in the constructor to point
    // to the URL "https://cplx.vm.uni-freiburg.de/storage/enroute/maps.json"
    DataManagement::Downloadable m_mapsJSON { QUrl(QStringLiteral("https://cplx.vm.uni-freiburg.de/storage/enroute-GeoJSONv003/maps.json")), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/maps.json" };

    // List of geographic maps
    DataManagement::DownloadableGroup m_databases;
    DataManagement::DownloadableGroup m_items;
    DataManagement::DownloadableGroup m_baseMaps;
    DataManagement::DownloadableGroup m_baseMapsRaster;
    DataManagement::DownloadableGroup m_baseMapsVector;
    DataManagement::DownloadableGroup m_aviationMaps;
};

};
