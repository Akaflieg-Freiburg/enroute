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

#include <QQmlEngine>
#include <QStandardPaths>

#include "GlobalObject.h"
#include "dataManagement/Downloadable_MultiFile.h"
#include "dataManagement/Downloadable_SingleFile.h"
#include "units/ByteSize.h"


namespace DataManagement {

/*! \brief Manages the list of geographic maps
 *
 *  This class manages a list of remotely available and locally installed
 *  databases and geographic maps.  More specifically, it manages the following
 *  items.
 *
 *  - Aviation maps (in GeoJSON format, file name ends in "geojson")
 *  - Base maps/raster (in MBTILES format, file name ends in "mbtiles")
 *  - Base maps/vector (in MBTILES format, file name ends in "raster")
 *  - Terrain maps (in MBTILES format, file name ends in "terrain")
 *  - FLARM Databases (as a text file, file name ends in "data")
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
    QML_ELEMENT
    QML_SINGLETON

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

    // No default constructor, important for QML singleton
    explicit DataManager() = delete;

    // factory function for QML singleton
    static DataManagement::DataManager* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::dataManager();
    }


    // deferred initialization
    void deferredInitialization() override;

    // destructor
    ~DataManager() = default;

    //
    // PROPERTIES
    //

    /*! \brief Downloadable_MultiFile that holds all approach charts
     *
     *  Pointer to a Downloadable_MultiFile that holds all approach charts.
     */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* approachCharts READ approachCharts CONSTANT)

    /*! \brief Indiates that the app needs to be updated
     *
     *  This property indicates that this version of the app is too old
     *  and cannot interpret maps and data that reside on the server.  If this is the case,
     *  the GUI should yell at the user and block any attempt to download or update maps and data.
     */
    Q_PROPERTY(bool appUpdateRequired READ appUpdateRequired NOTIFY appUpdateRequiredChanged)

    /*! \brief Downloadable_MultiFile that holds all aviation maps
     *
     *  Pointer to a Downloadable_MultiFile that holds all aviation maps.
     */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* aviationMaps READ aviationMaps CONSTANT)

    /*! \brief Downloadable_MultiFile that holds all base maps in raster format
     *
     *  Pointer to a Downloadable_MultiFile that holds all base maps in raster format.
     */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* baseMapsRaster READ baseMapsRaster CONSTANT)

    /*! \brief Downloadable_MultiFile that holds all base maps in vector format
     *
     *  Pointer to a Downloadable_MultiFile that holds all base maps in vector format.
     */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* baseMapsVector READ baseMapsVector CONSTANT)

    /*! \brief Downloadable_MultiFile that holds all base maps
     *
     *  Pointer to a Downloadable_MultiFile that holds all base maps.
     */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* baseMaps READ baseMaps CONSTANT)

    /*! \brief Downloadable_MultiFile that holds all databases
     *
     *  Pointer to a Downloadable_MultiFile that holds all databases.
     */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* databases READ databases CONSTANT)

    /*! \brief Downloadable_MultiFile that holds all data items
     *
     *  Pointer to a Downloadable_MultiFile that holds all data items.  This
     *  includes aviation maps, base maps, and databases.
     */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* items READ items CONSTANT)

    /*! \brief Downloadable_SingleFile that holds the list of all maps and databases */
    Q_PROPERTY(DataManagement::Downloadable_SingleFile* mapList READ mapList CONSTANT)

    /*! \brief Downloadable_MultiFile that holds all the map sets and databases */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* mapsAndData READ mapsAndData CONSTANT)

    /*! \brief Downloadable_MultiFile that holds all map sets
     *
     *  Pointer to a Downloadable_MultiFile that holds all map sets.
     */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* mapSets READ mapSets CONSTANT)

    /*! \brief Downloadable_MultiFile that holds all terrain maps
     *
     *  Pointer to a Downloadable_MultiFile that holds all terrain maps.
     */
    Q_PROPERTY(DataManagement::Downloadable_MultiFile* terrainMaps READ terrainMaps CONSTANT)

    /*! \brief Current "what's new" message */
    Q_PROPERTY(QString whatsNew READ whatsNew NOTIFY whatsNewChanged)

    /*! \brief Hash of the current "what's new" message */
    Q_PROPERTY(Units::ByteSize whatsNewHash READ whatsNewHash NOTIFY whatsNewChanged)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property approachCharts
     */
    [[nodiscard]] auto approachCharts() -> DataManagement::Downloadable_MultiFile* { return &m_approachCharts; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property aviationMaps
     */
    [[nodiscard]] auto appUpdateRequired() const -> bool { return m_appUpdateRequired; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property aviationMaps
     */
    [[nodiscard]] auto aviationMaps() -> DataManagement::Downloadable_MultiFile* { return &m_aviationMaps; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property baseMaps
     */
    [[nodiscard]] auto baseMaps() -> DataManagement::Downloadable_MultiFile* { return &m_baseMaps; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property baseMapsRaster
     */
    [[nodiscard]] auto baseMapsRaster() -> DataManagement::Downloadable_MultiFile* { return &m_baseMapsRaster; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property baseMapsVector
     */
    [[nodiscard]] auto baseMapsVector() -> DataManagement::Downloadable_MultiFile* { return &m_baseMapsVector; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property databases
     */
    [[nodiscard]] auto databases() -> DataManagement::Downloadable_MultiFile* { return &m_databases; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property items
     */
    [[nodiscard]] auto items() -> DataManagement::Downloadable_MultiFile* { return &m_items; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property mapsAndData
     */
    [[nodiscard]] auto mapList() -> DataManagement::Downloadable_SingleFile* { return &m_mapList; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property mapsAndData
     */
    [[nodiscard]] auto mapsAndData() -> DataManagement::Downloadable_MultiFile* { return &m_mapsAndData; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property mapSets
     */
    [[nodiscard]] auto mapSets() -> DataManagement::Downloadable_MultiFile* { return &m_mapSets; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property terrainMaps
     */
    [[nodiscard]] auto terrainMaps() -> DataManagement::Downloadable_MultiFile* { return &m_terrainMaps; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property whatsNew
     */
    [[nodiscard]] auto whatsNew() const -> QString { return m_whatsNew; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastWhatsNewHash
     */
    [[nodiscard]] auto whatsNewHash() const -> Units::ByteSize { return qHash(m_whatsNew, 0); }


    //
    // Methods
    //

    /*! \brief Import raster or vector map into the library of locally installed
     * maps
     *
     * This method imports a raster or vector map in MBTILES format into the
     * library of locally installed maps. To avoid clashes and inconsistencies,
     * the map will delete all locally install vector maps when importing a
     * raster map, and all raster maps when importing a vector map.
     *
     * @param fileName File name of locally raster or vector map, in MBTILES
     * format.
     *
     * @param newName Name under which the map is available in the library. If
     * the name exists, the library entry will be replaced.
     *
     * @returns A human-readable HTML string on error, or an empty string on
     * success
     */
    Q_INVOKABLE QString import(const QString& fileName, const QString& newName);

    /*! \brief Import airspace data into the library of locally installed
     * maps
     *
     * This method imports airspace data in OpenAir format into the
     * library of locally installed maps.
     *
     * @param fileName File name of locally raster or vector map, in OpenAir
     * format.
     *
     * @param newName Name under which the map is available in the library. If
     * the name exists, the library entry will be replaced.
     *
     * @returns A human-readable HTML string on error, or an empty string on
     * success
     */
    Q_INVOKABLE QString importOpenAir(const QString& fileName, const QString& newName);

public slots:
    /*! \brief Triggers an update of the list of remotely available data items
     *
     *  This will trigger a download the file maps.json from the remote server.
     */
    void updateRemoteDataItemList()
    {
        m_mapList.startDownload();
    }

signals:
    /*! Notifier signal */
    void appUpdateRequiredChanged();

    /*! \brief Error message for user
     *
     *  This signal is emitted if an error occurs and the GUI should display a
     *  prominent error message. This signal can be emitted anytime.
     *
     *  @param message A brief, human-readable, translated error message.
     */
    void error(const QString& message);

    /*! \brief Notifier signal */
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
    // added to _aviationMap, _baseMaps, or _databases. A pointer to that item is
    // then returned.
    DataManagement::Downloadable_SingleFile* createOrRecycleItem(const QUrl& url, const QString& localFileName, const QGeoRectangle& bBox);

    bool m_appUpdateRequired {false};

    // Full path name of data directory, without trailing slash
    QString m_dataDirectory {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/aviation_maps"};

    // The current whats new string from _aviationMaps.
    QString m_whatsNew {};

    // This Downloadable object manages the central text file that describes the
    // remotely available aviation maps. It is set in the constructor to point
    // to the URL "https://cplx.vm.uni-freiburg.de/storage/enroute/maps.json"
    DataManagement::Downloadable_SingleFile m_mapList { QUrl(QStringLiteral("https://cplx.vm.uni-freiburg.de/storage/enroute-GeoJSONv003/maps.json")), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/maps.json" };

    // List of geographic maps
    DataManagement::Downloadable_MultiFile m_approachCharts {DataManagement::Downloadable_MultiFile::SingleUpdate};
    DataManagement::Downloadable_MultiFile m_aviationMaps {DataManagement::Downloadable_MultiFile::SingleUpdate};
    DataManagement::Downloadable_MultiFile m_baseMaps {DataManagement::Downloadable_MultiFile::SingleUpdate};
    DataManagement::Downloadable_MultiFile m_baseMapsRaster {DataManagement::Downloadable_MultiFile::SingleUpdate};
    DataManagement::Downloadable_MultiFile m_baseMapsVector {DataManagement::Downloadable_MultiFile::SingleUpdate};
    DataManagement::Downloadable_MultiFile m_databases {DataManagement::Downloadable_MultiFile::SingleUpdate};
    DataManagement::Downloadable_MultiFile m_items {DataManagement::Downloadable_MultiFile::SingleUpdate};
    DataManagement::Downloadable_MultiFile m_mapsAndData {DataManagement::Downloadable_MultiFile::SingleUpdate};
    DataManagement::Downloadable_MultiFile m_terrainMaps {DataManagement::Downloadable_MultiFile::SingleUpdate};

    // List of geographic map sets
    DataManagement::Downloadable_MultiFile m_mapSets  {DataManagement::Downloadable_MultiFile::SingleUpdate};
};

} // namespace DataManagement
