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

#include <QFuture>
#include <QGeoCoordinate>
#include <QJsonArray>
#include <QMutex>
#include <QMutexLocker>
#include <QPointer>
#include <QRegularExpression>
#include <QTemporaryFile>

#include "Airspace.h"
#include "Librarian.h"
#include "dataManagement/DataManager.h"
#include "Settings.h"
#include "Waypoint.h"
#include "TileServer.h"


class Librarian;
class SatNav;
class Waypoint;

namespace Weather {
class WeatherDataProvider;
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
    /*! \brief Creates a new GeoMap provider
     *
     * This constructor creates a new GeoMapProvider instance. Note that the
     * instance will only start to work once the method setWeatherDataProvider() has
     * been called.
     *
     * @param parent The standard QObject parent
     */
    explicit GeoMapProvider(QObject *parent = nullptr);

    /*! \brief Destructor */
    ~GeoMapProvider() = default;


    //
    // Methods
    //

    /*! \brief Create invalid waypoint
     *
     *  This is a helper method for QML, where creation of waypoint objects is difficult.
     *
     *  @returns An invalid waypoint
     */
    Q_INVOKABLE static GeoMaps::Waypoint createWaypoint()
    {
        return {};
    }


    //
    // Properties
    //

    /*! \brief List of airspaces at a given location
     *
     * @param position Position over which airspaces are searched for
     *
     * @returns all airspaces that exist over a given position. For better
     * cooperation with QML the list returns contains elements of type QObject*,
     * and not Airspace*.
     */
    Q_INVOKABLE QVariantList airspaces(const QGeoCoordinate& position);

    /*! \brief Find closest waypoint to a given position
     *
     * @param position Position near which waypoints are searched for
     *
     * @param distPosition Reference position
     *
     * @returns The Waypoint that is closest to the given position, provided
     * that the distance is not bigger than that to distPosition. If no
     * sufficiently close waypoint is found, a generic Waypoint with the
     * appropriate coordinate is returned.
     */
    Q_INVOKABLE GeoMaps::Waypoint closestWaypoint(QGeoCoordinate position, const QGeoCoordinate& distPosition);

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

    /*! \brief Waypoints containing a given substring
     *
     * @param filter List of words
     *
     * @returns all those waypoints whose fullName or codeName contains each of
     * the words in filter.  In order to make the result accessible to QML, the
     * list is returned as QList<QObject*>. It can thus be used as a data model
     * in QML.
     */
    Q_INVOKABLE QVariantList filteredWaypointObjects(const QString &filter);

    /*! Find a waypoint by its ICAO code
     *
     * @param id ICAO code of the waypoint, such as "EDDF" for Frankfurt
     *
     * @returns a nullpointer if no waypoint has been found, or else a pointer
     * to the waypoint. For better cooperation with QML is not a pointer of type
     * Waypoint, but ratherof type QObject. The object is owned by this class
     * and must not be deleted.
     */
    Waypoint findByID(const QString& id);

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
    Q_INVOKABLE QVariantList nearbyWaypoints(const QGeoCoordinate& position, const QString& type);

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
    QVector<Waypoint> waypoints() {
        QMutexLocker locker(&_aviationDataMutex);
        return _waypoints_;
    }

signals:
    /*! \brief Notification signal for the property with the same name */
    void geoJSONChanged();

    /*! \brief Notification signal for the property with the same name */
    void styleFileURLChanged();

private slots:
    // Intializations that are moved out of the constructor, in order to avoid
    // nested uses of constructors in Global.
    void deferredInitialization();


private:
    Q_DISABLE_COPY_MOVE(GeoMapProvider)

    // Caches used to speed up the method simplifySpecialChars
    QRegularExpression specialChars {QStringLiteral("[^a-zA-Z0-9]")};
    QHash<QString, QString> simplifySpecialChars_cache;

    // This slot is called every time the the set of GeoJSON files changes. It
    // fills the aviation data cache.
    void aviationMapsChanged();

    // Interal function that does most of the work for aviationMapsChanged() emits
    // geoJSONChanged() when done. This function is meant to be run in a separate
    // thread.
    void fillAviationDataCache(const QStringList& JSONFileNames, bool hideUpperAirspaces, bool hideGlidingSectors);

    // This slot is called every time the the set of MBTile files changes. It
    // sets up the tile server to and generates a new style file.
    void baseMapsChanged();

    // This is the path under which is tiles are available on the
    // _tileServer. This is set to a random number that changes every time the
    // set of MBTile files changes
    QString _currentPath;

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
    QVector<Waypoint> _waypoints_;       // Cache: Waypoints
    QVector<Airspace> _airspaces_;       // Cache: Airspaces
};

};
