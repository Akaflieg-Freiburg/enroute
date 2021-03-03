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

#include <QFuture>
#include <QGeoCoordinate>
#include <QJsonArray>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QRegularExpression>
#include <QTemporaryFile>

#include "Airspace.h"
#include "GlobalSettings.h"
#include "Librarian.h"
#include "MapManager.h"
#include "TileServer.h"
#include "Waypoint.h"

class Librarian;
class SatNav;
class Waypoint;

namespace Weather {
class DownloadManager;
}


namespace GeoMaps {

/*! \brief Serves GeoMaps, as MBTiles via an embedded HTTP server, and as GeoJSON
 *
 * This class works closely with MapManager.  It reads the files managed by
 * MapManager and provides them for use in MapBoxGL powered maps. The data is
 * served via two channels.
 *
 * - All files in GeoJSON format are concatenated, and the resulting compound
 *   GeoJSON is served via the geoJSON property of this class.
 *
 * - A list of waypoints is generated and available via the waypoints property
 *
 * - All files in MBTiles format are served via an embedded TileServer that
 *   listens to a free port on address 127.0.0.1. The GeoMapProvider generates a
 *   mapbox style file whose source element points to the URL of that
 *   TileServer. The URL of the style file is served via the property
 *   styleFileURL property of this class.
 */

class GeoMapProvider : public QObject
{
    Q_OBJECT

public:
    /*! \brief Create a new GeoMap provider
     *
     * This constructor creates a new GeoMapProvider instance. Note that the
     * instance will only start to work once the method setDownloadManager() has
     * been called.
     *
     * @param manager Pointer to a MapManager whose files will be served. The
     * manager shall exist for the lifetime of this object.
     *
     * @param librarian Global Librarian object. The librarian shall exist for
     * the lifetime of this object.
     *
     * @param parent The standard QObject parent
     */
    explicit GeoMapProvider(MapManager *manager, Librarian *librarian, QObject *parent = nullptr);

    // Standard destructor
    ~GeoMapProvider() override = default;

    /*! \brief List of airspaces at a given location
     *
     * @param position Position over which airspaces are searched for
     *
     * @returns all airspaces that exist over a given position. For better
     * cooperation with QML the list returns contains elements of type QObject*,
     * and not Airspace*.
     */
    Q_INVOKABLE QList<QObject*> airspaces(const QGeoCoordinate& position);

    /*! \brief Find closest waypoint to a given position
     *
     * @param position Position near which waypoints are searched for
     *
     * @param distPosition Reference position
     *
     * @param flightRoute If not nullptr, then the mid-field waypoints of the
     * flight route are also searched for nearby waypoints. (Note: This should be
     * of type "const FlightRoute*", but QML cannot handle const)
     *
     * @returns the Waypoint that is closest to the given position, provided
     * that the distance is not bigger than that to distPosition. If no
     * sufficiently close waypoint is found a nullptr is returned.  Ownership of
     * the returned object is NOT transferred to the caller. For QML, ownership
     * explicitly set to QQmlEngine::CppOwnership.
     */
    Q_INVOKABLE QObject* closestWaypoint(QGeoCoordinate position, const QGeoCoordinate& distPosition, FlightRoute *flightRoute=nullptr);

    /*! \brief Copyright notice for the map
     *
     * This property holds the copyright notice for the installed aviation
     * and base maps as a HTML string, ready to be shown to the user.
     */
    Q_PROPERTY(QString copyrightNotice READ copyrightNotice CONSTANT)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property copyrightNotice
     */
    QString copyrightNotice() const
    {
        return QStringLiteral("<a href='https://openAIP.net'>© openAIP</a> • <a href='https://openflightmaps.org'>© open flightmaps</a> • <a href='https://maptiler.com/copyright/'>© MapTiler</a> • <a href='https://www.openstreetmap.org/copyright'>© OpenStreetMap contributors</a>");
    }

    /*! \brief Create waypoint object
     *
     * This default constructor creates a waypoint with the following properties
     *
     * - QObject parent is nullptr
     * - "CAT" is set to "WP"
     * - "NAM" is set to "Waypoint"
     * - "TYP" is set to "WP"
     *
     * This method exists because it is difficult to create C++ objects in QML.
     * The caller must accept ownership of the object.
     *
     * @returns Pointer to a newly created waypoint object
     */
    Q_INVOKABLE static GeoMaps::Waypoint* createWaypoint() ;

    /*! \brief Describe installed map
     *
     * This method describes installed GeoJSON map files.
     *
     * @warning The data is only updated
     * after the maps have been parsed in the GeoJSON parsing process. It is
     * therefore possible that the method returns wrong information if it is
     * called directly after a new map has been installed.
     *
     * @param fileName Name of a GeoJSON file.
     *
     * @returns A human-readable HTML string, or an empty string if no data is available
     */
    Q_INVOKABLE static QString describeMapFile(const QString& fileName);

    /*! \brief Waypoints containing a given substring
     *
     * @param filter List of words
     *
     * @returns all those waypoints whose fullName or codeName contains each of
     * the words in filter.  In order to make the result accessible to QML, the
     * list is returned as QList<QObject*>. It can thus be used as a data model
     * in QML.
     */
    Q_INVOKABLE QList<QObject*> filteredWaypointObjects(const QString &filter);

    /*! \brief Union of all aviation maps in GeoJSON format
     *
     * This property holds all installed aviation maps in GeoJSON format,
     * combined into one GeoJSON document.
     */
    Q_PROPERTY(QByteArray geoJSON READ geoJSON NOTIFY geoJSONChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property geoJSON
     */
    QByteArray geoJSON() {
        QMutexLocker lock(&_aviationDataMutex);
        return _combinedGeoJSON_;
    }

    /*! Find a waypoint by its ICAO code
     *
     * @param id ICAO code of the waypoint, such as "EDDF" for Frankfurt
     *
     * @returns a nullpointer if no waypoint has been found, or else a pointer
     * to the waypoint. For better cooperation with QML is not a pointer of type
     * Waypoint, but ratherof type QObject. The object is owned by this class
     * and must not be deleted.
     */
    Waypoint* findByID(const QString& id);

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
    Q_INVOKABLE QList<QObject*> nearbyWaypoints(const QGeoCoordinate& position, const QString& type);

    /*! \brief URL where a style file for the base map can be retrieved
     *
     * This property holds a URL where a mapbox style file for the base map can
     * be retrieved. The style file is adjusted, so that its source element
     * points to the local TileServer URL where the base map is served.
     * Whenever the base map changes (e.g. because new maps have been downloaded
     * or removed), the style file is deleted, a new style file is generated and
     * a notification signal is emitted.
     */
    Q_PROPERTY(QString styleFileURL READ styleFileURL NOTIFY styleFileURLChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property styleFileURL
     */
    QString styleFileURL() const;

    /*! \brief Waypoints
     *
     * @returns a list of all waypoints known to this GeoMapProvider (that is,
     * the union of all waypoints in any of the installed maps)
     */
    QVector<QPointer<Waypoint>> waypoints() {
        QMutexLocker locker(&_aviationDataMutex);
        return _waypoints_;
    }

    /*! \brief Connects this GeoMapProvider with a DownloadManager instance
     *
     * To work right, the GeoMapProvider needs access to an instance of the
     * DownloadManager class. Because of cross-dependencies between the classes
     * GeoMapProvider and DownloadManager, the pointer to the DownloadManager is not
     * given as an argument in the constructor, but must be set as soon as
     * possible after construction with the present method.
     *
     * The GeoMapProvider will not crash if the DownloadManager is deleted at
     * run-time, but several method will only work with reduced functionality,
     * as newly generated waypoint will no longer contain weather information.
     *
     * @param downloadManager Pointer to a DownloadManager object.
     */
    void setDownloadManager(Weather::DownloadManager *downloadManager);

signals:
    /*! \brief Notification signal for the property with the same name */
    void geoJSONChanged();

    /*! \brief Notification signal for the property with the same name */
    void styleFileURLChanged();

private:
    Q_DISABLE_COPY_MOVE(GeoMapProvider)

    // Pointers to other classes that are used internally
    QPointer<Weather::DownloadManager> _downloadManager;

    // Caches used to speed up the method simplifySpecialChars
    QRegularExpression specialChars {QStringLiteral("[^a-zA-Z0-9]")};
    QHash<QString, QString> simplifySpecialChars_cache;

    // This slot is called every time the the set of GeoJSON files changes. It
    // fills the aviation data cache.
    void aviationMapsChanged();

    // Interal function that does most of the work for aviationMapsChanged() emits
    // geoJSONChanged() when done. This function is meant to be run in a separate
    // thread.
    void fillAviationDataCache(const QStringList& JSONFileNames, bool hideUpperAirspaces);

    // This slot is called every time the the set of MBTile files changes. It
    // sets up the tile server to and generates a new style file.
    void baseMapsChanged();

    // This is the path under which is tiles are available on the
    // _tileServer. This is set to a random number that changes every time the
    // set of MBTile files changes
    QString _currentPath;

    // Pointer to the MapManager
    QPointer<MapManager> _manager;

    // Pointer to library
    QPointer<Librarian> _librarian;

    // Tile Server
    TileServer _tileServer;

    // Temporary file that holds the current style file
    QPointer<QTemporaryFile> _styleFile;

    //
    // Aviation Data Cache
    //
    QFuture<void>    _aviationDataCacheFuture; // Future; indicates if fillAviationDataCache() is currently running
    QTimer           _aviationDataCacheTimer;  // Timer used to start another run of fillAviationDataCache()

    // The data in this group is accessed by several threads. The following classes
    // (whose names ends in an underscore) are therefore
    // protected by this mutex.
    QMutex           _aviationDataMutex;
    QByteArray       _combinedGeoJSON_; // Cache: GeoJSON
    QVector<QPointer<Waypoint>> _waypoints_;       // Cache: Waypoints
    QVector<QPointer<Airspace>> _airspaces_;       // Cache: Airspaces
};

};
