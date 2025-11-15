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

#include <QCache>
#include <QFuture>
#include <QGeoRectangle>
#include <QImage>
#include <QProperty>
#include <QQmlEngine>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QTimer>

#include "Airspace.h"
#include "GlobalObject.h"
#include "TileServer.h"
#include "Waypoint.h"
#include "fileFormats/MBTILES.h"

using namespace Qt::Literals::StringLiterals;


namespace GeoMaps
{

/*! \brief Provides geographic information
   *
   * This class works closely with dataManagement/DataManager.  It takes the
   * data provided by the DataManager, and serves it for use in MapBoxGL powered
   * maps. Additional data is served via the API.
   *
   * - The class ensures that the currently available base maps are served via
   *   the embedded TileServer.
   *
   * - The class generates a mapbox style file whose source element points to
   *   the URL of the embedded TileServer. The style file automatically adjusts
   *   when raster maps or vector maps are installed.
   *
   * - All available aviation data is provided in GeoJSON.
   *
   * - Waypoints and airspaces are accessible via the API.
   *
   */

class GeoMapProvider : public GlobalObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    /*! \brief Creates a new GeoMap provider
     *
     * This constructor creates a new GeoMapProvider instance.
     *
     * @param parent The standard QObject parent
     */
    explicit GeoMapProvider(QObject *parent = nullptr);

    // No default constructor, important for QML singleton
    explicit GeoMapProvider() = delete;

    // deferred initialization
    void deferredInitialization() override;

    // factory function for QML singleton
    static GeoMaps::GeoMapProvider* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::geoMapProvider();
    }

    /*! \brief Destructor */
    ~GeoMapProvider() override = default;



    //
    // Properties
    //

    /*! \brief Available Airspaces
     *
     *  This property holds all airspaces known to Enroute Flight Navigation
     */
    Q_PROPERTY(QList<GeoMaps::Airspace> airspaces READ airspaces BINDABLE bindableAirspaces)

    /*! \brief Available Raster Maps
     *
     *  This property holds the names of raster maps that can be set with
     *  setCurrentRasterMap.
     */
    Q_PROPERTY(QStringList availableRasterMaps READ availableRasterMaps BINDABLE bindableAvailableRasterMaps)

    /*! \brief Copyright notice for the map
     *
     * This property holds the copyright notice for the installed aviation and
     * base maps as a HTML string, ready to be shown to the user.
     */
    Q_PROPERTY(QString copyrightNotice READ copyrightNotice CONSTANT)

    /*! \brief Current Raster Map
     *
     *  This property holds the name of the current raster map, or an empty
     *  string if no map has been set. The raster map is exposed via the URL
     *
     *  GeoMapProvider.serverUrl() + "/rasterMap/"
     *
     *  @see The documentation of the setter for naming conventions.
     */
    Q_PROPERTY(QString currentRasterMap READ currentRasterMap WRITE setCurrentRasterMap BINDABLE bindableCurrentRasterMap)

    /*! \brief Current Raster Map Tile Size
     *
     *  This property holds the tile size name of the current raster map, or the
     *  default value 512.
     */
    Q_PROPERTY(int currentRasterMapTileSize READ currentRasterMapTileSize BINDABLE bindableCurrentRasterMapTileSize)

    /*! \brief Union of all aviation maps in GeoJSON format
     *
     * This property holds all installed aviation maps in GeoJSON format,
     * combined into one GeoJSON document.
     */
    Q_PROPERTY(QByteArray geoJSON READ geoJSON BINDABLE bindableGeoJSON)

    /*! \brief URL under which this server is presently reachable
     *
     *  The property holds returns the Url where the server is listening to
     *  incoming connections. This is typically string of the form
     *  "http://127.0.0.1:3470". If the server is not listening to incoming
     *  connections, the string is empty.
     */
    Q_PROPERTY(QString serverUrl READ serverUrl NOTIFY serverUrlChanged)

    /*! \brief URL where a style file for the base map can be retrieved
     *
     * This property holds a URL where a mapbox style file for the base map can
     * be retrieved. The style file is adjusted, so that its source element
     * points to the local TileServer URL where the base map is served. Whenever
     * the base map changes (e.g. because new maps have been downloaded or
     * removed), the style file is deleted, a new style file is generated and a
     * notification signal is emitted.
     */
    Q_PROPERTY(QString styleFileURL READ styleFileURL NOTIFY styleFileURLChanged)

    /*! \brief List of terrain map MBTILES */
    Q_PROPERTY(QList<QSharedPointer<FileFormats::MBTILES>> terrainMapTiles READ terrainMapTiles NOTIFY terrainMapTilesChanged)

    /*! \brief Waypoints
     *
     * A list of all waypoints known to this GeoMapProvider (that is,
     * the union of all waypoints in any of the installed maps)
     */
    Q_PROPERTY(QList<GeoMaps::Waypoint> waypoints READ waypoints NOTIFY waypointsChanged)



    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property airspaces
     */
    [[nodiscard]] QList<GeoMaps::Airspace> airspaces() const {return m_airspaces.value();}

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property airspaces
     */
    [[nodiscard]] QBindable<QList<GeoMaps::Airspace>> bindableAirspaces() {return &m_airspaces;}

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property availableRasterMaps
     */
    [[nodiscard]] QStringList availableRasterMaps() const
    {
        return m_availableRasterMaps.value();
    }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property availableRasterMaps
     */
    [[nodiscard]] QBindable<QStringList> bindableAvailableRasterMaps() const
    {
        return &m_availableRasterMaps;
    }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property copyrightNotice
     */
    [[nodiscard]] static QString copyrightNotice();

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property currentRasterMap
     */
    [[nodiscard]] QString currentRasterMap() const {return m_currentRasterMap.value();}

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property currentRasterMap
     */
    [[nodiscard]] QBindable<QString> bindableCurrentRasterMap() const {return &m_currentRasterMap;}

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property currentRasterMapTileSize
     */
    [[nodiscard]] int currentRasterMapTileSize() const {return m_currentRasterMapTileSize.value();}

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property currentRasterMapTileSize
     */
    [[nodiscard]] QBindable<int> bindableCurrentRasterMapTileSize() const {return &m_currentRasterMapTileSize;}

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property geoJSON
     */
    [[nodiscard]] QByteArray geoJSON() const {return m_combinedGeoJSON.value();}

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property geoJSON
     */
    [[nodiscard]] QBindable<QByteArray> bindableGeoJSON() {return &m_combinedGeoJSON;}

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property serverUrl
     */
    [[nodiscard]] QString serverUrl() {return m_tileServer.serverUrl();}

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property styleFileURL
     */
    [[nodiscard]] QString styleFileURL();

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property terrainMapTiles
     */
    [[nodiscard]] QList<QSharedPointer<FileFormats::MBTILES>> terrainMapTiles() const
    {
        return m_terrainMapTiles;
    }

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property waypoints
     */
    [[nodiscard]] QList<Waypoint> waypoints();



    //
    // Setter Methods
    //

    /*! \brief Setter function for the property with the same name
     *
     * @param mapName currentRasterMap
     */
    void setCurrentRasterMap(const QString& mapName);



    //
    // Methods
    //

    /*! \brief List of airspaces at a given position
     *
     * @param position Position over which airspaces are searched for
     *
     * @returns all airspaces that exist over a given position. For better
     * cooperation with QML the list returns contains elements of type QObject*,
     * and not Airspace*.
     */
    [[nodiscard]] Q_INVOKABLE QVariantList airspacesAtPosition(const QGeoCoordinate &position);

    /*! \brief Find closest waypoint to a given position
     *
     * @param position Position near which waypoints are searched for
     *
     * @param distPosition Reference position
     *
     * @returns The Waypoint that is closest to the given position, provided
     * that the distance is not bigger than that to distPosition. If no
     * sufficiently close waypoint is found, a generic Waypoint with the
     * appropriate coordinate is returned. The method checks waypoints from the
     * map, and waypoints from the library.
     */
    [[nodiscard]] Q_INVOKABLE GeoMaps::Waypoint closestWaypoint(QGeoCoordinate position, const QGeoCoordinate &distPosition);

    /*! \brief Create invalid waypoint
     *
     *  This is a helper method for QML, where creation of waypoint objects is
     *  difficult.
     *
     *  @returns An invalid waypoint
     */
    [[nodiscard]] Q_INVOKABLE static GeoMaps::Waypoint createWaypoint()
    {
        return {};
    }

    /*! \brief Elevation of terrain at a given coordinate, above sea level
     *
     *  @param coordinate Coordinate
     *
     *  @return Elevation of the terrain at coordinate over MSP, or NaN if the
     *  terrain elevation is unknown
     */
    [[nodiscard]] Q_INVOKABLE Units::Distance terrainElevationAMSL(const QGeoCoordinate& coordinate);

    /*! \brief Create empty GeoJSON document
     *
     *  @returns Empty, but valid GeoJSON document
     */
    [[nodiscard]] Q_INVOKABLE static QByteArray emptyGeoJSON();

    /*! \brief Waypoints containing a given substring
     *
     * @param filter List of words
     *
     * @returns all those waypoints whose fullName or codeName contains each of
     * the words in filter. The list contains both waypoints from the map, and
     * waypoints from the library and is sorted alphabetically.
     */
    [[nodiscard]] Q_INVOKABLE QVector<GeoMaps::Waypoint> filteredWaypoints(const QString& filter);

    /*! Find a waypoint by its ICAO code
     *
     * @param icaoID ICAO code of the waypoint, such as "EDDF" for Frankfurt
     *
     * @returns A waypoint, potentially invalid if the icaoID is unknown
     */
    [[nodiscard]] Q_INVOKABLE Waypoint findByID(const QString& icaoID);

    /*! List of nearby waypoints
     *
     * @param position Position near which waypoints are searched for
     * @param type Type of waypoints (AD, NAV, WP)
     *
     * @returns a list of the 20 waypoints of requested type that are closest to
     * the given position; the list may however be empty or contain fewer than
     * 20 items.  For better cooperation with QML the list does not contain
     * elements of type Waypoint*, but elements of type QObject*
     */
    [[nodiscard]] Q_INVOKABLE QList<GeoMaps::Waypoint> nearbyWaypoints(const QGeoCoordinate& position, const QString& type);


signals:
    /*! \brief Notification signal for the property with the same name */
    void geoJSONChanged();

    /*! \brief Notification signal for the property with the same name */
    void styleFileURLChanged();

    /*! \brief Notification signal for the property with the same name */
    void serverUrlChanged();

    /*! \brief Notification signal for the property with the same name */
    void terrainMapTilesChanged();

    /*! \brief Notification signal for the property with the same name */
    void waypointsChanged();

private:
    Q_DISABLE_COPY_MOVE(GeoMapProvider)

    // This slot is called every time the the set of aviation maps qchanges. It
    // fills the aviation data cache.
    void onAviationMapsChanged();

    // This slot is called every time the the set of MBTile files changes. It
    // sets up the tile server to and generates a new style file.
    void onMBTILESChanged();

    // Interal function that does most of the work for aviationMapsChanged()
    // emits geoJSONChanged() when done. This function is meant to be run in a
    // separate thread.
    struct aviationDataCacheResult {
        QList<Waypoint> waypoints;
        QList<Airspace> airspaces;
        QByteArray combinedGeoJSON;
    };
    aviationDataCacheResult fillAviationDataCache(QStringList JSONFileNames, Units::Distance airspaceAltitudeLimit, bool hideGlidingSectors);

    // Caches used to speed up the method simplifySpecialChars
    QRegularExpression specialChars{QStringLiteral("[^a-zA-Z0-9]")};
    QHash<QString, QString> simplifySpecialChars_cache;

    // This is the path under which map tiles are available on the m_tileServer.
    // This is set to a random number that changes every time the set of MBTile
    // files changes
    QString _currentBaseMapPath;
    QString _currentTerrainMapPath;

    // Tile Server
    TileServer m_tileServer;

    // Temporary file that holds the current style file
    QPointer<QTemporaryFile> m_styleFile;

    //
    // Aviation Data Cache
    //
    QFuture<GeoMaps::GeoMapProvider::aviationDataCacheResult> _aviationDataCacheFuture; // Future; indicates if fillAviationDataCache() is currently running
    QTimer _aviationDataCacheTimer;         // Timer used to start another run of fillAviationDataCache()

    //
    // MBTILES
    //
    QList<QSharedPointer<FileFormats::MBTILES>> m_baseMapVectorTiles;
    QProperty<QList<QSharedPointer<FileFormats::MBTILES>>> m_baseMapRasterTiles;
    QList<QSharedPointer<FileFormats::MBTILES>> m_terrainMapTiles;

    QProperty<QStringList> m_availableRasterMaps;
    QStringList computeAvailableRasterMaps();

    QProperty<QString> m_currentRasterMap {u"non-empty place holder"_s};
    QPropertyNotifier m_currentRasterMapNotifier; // Used to save the currentRasterMap
    QProperty<int> m_currentRasterMapTileSize {512};

    Q_OBJECT_BINDABLE_PROPERTY(GeoMaps::GeoMapProvider, QByteArray, m_combinedGeoJSON, &GeoMaps::GeoMapProvider::geoJSONChanged)
    QList<Waypoint> _waypoints_; // Cache: Waypoints
    QProperty<QList<Airspace>> m_airspaces; // Cache: Airspaces

    // TerrainImageCache
    QCache<qint64,QImage> terrainTileCache {6}; // Hold 6 tiles, roughly 1.2MB

    // GeoJSON file
    QString geoJSONCache {QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + u"/aviationData.json"_s};
};

} // namespace GeoMaps
