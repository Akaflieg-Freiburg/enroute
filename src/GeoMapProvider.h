/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#ifndef GEOMAPPROVIDER_H
#define GEOMAPPROVIDER_H

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
#include "MapManager.h"
#include "TileServer.h"
#include "Waypoint.h"


/*! \brief Serves GeoMaps, as MBTiles via an embedded HTTP server, and as
    GeoJSON
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
     * @param manager Pointer to a MapManager whose files will be served. The
     * manager shall exist for the lifetime of this object.
     *
     * @param settings GlobalSettings object where settings are stored. The
     * settings shall exist for the lifetime of this object.
     *
     * @param parent The standard QObject parent
     */
    explicit GeoMapProvider(MapManager *manager, GlobalSettings* settings, QObject *parent = nullptr);

    // No copy constructor
    GeoMapProvider(GeoMapProvider const&) = delete;

    // No assign operator
    GeoMapProvider& operator =(GeoMapProvider const&) = delete;

    // No move constructor
    GeoMapProvider(GeoMapProvider&&) = delete;

    // No move assignment operator
    GeoMapProvider& operator=(GeoMapProvider&&) = delete;

    // Standard destructor
    ~GeoMapProvider() override = default;

    /*! \brief List of airspaces at a given location
     *
     * This method returns all airspaces that exist over a given position. For
     * better cooperation with QML the list returns contains elements of type
     * QObject*, and not Airspace*.
     */
    Q_INVOKABLE QList<QObject*> airspaces(const QGeoCoordinate& position);

    /*! \brief Find closest waypoint to a given position
     *
     * This method returns the Waypoint that is closest to the given
     * position, provided that the distance is not bigger than that to
     * distPosition. If no sufficiently close waypoint is found a nullptr is
     * returned.
     */
    Q_INVOKABLE QObject* closestWaypoint(QGeoCoordinate position, const QGeoCoordinate& distPosition);

    /*! \brief Waypoints containing a given substring
     *
     * This method returns all those waypoints whose fullName or codeName
     * contains each of the words in filter.  In order to make the result
     * accessible to QML, the list is returned as QList<QObject*>. It can thus
     * be used as a data model in QML.
     */
    Q_INVOKABLE QList<QObject*> filteredWaypointObjects(const QString &filter);

    /*! \brief Union of all aviation maps in GeoJSON format
     *
     * This property holds all installed aviation maps in GeoJSON format,
     *  combined into one bis GeoJSON document
     */
    Q_PROPERTY(QByteArray geoJSON READ geoJSON NOTIFY geoJSONChanged)

    /*! \brief Getter function for the property with the same name */
    QByteArray geoJSON();

    /*! List of nearby airfields
     *
     * This method returns a list of the 20 airfields that are closest to the
     * given position; the list may however be empty or contain fewer than 20
     * items. For better cooperation with QML the list does not contain elements
     * of type Waypoint*, but elements of type QObject*
     */
    Q_INVOKABLE QList<QObject*> nearbyAirfields(const QGeoCoordinate& position);

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

    /*! \brief Getter function for the property with the same name */
    QString styleFileURL() const;

    /*! \brief Waypoints
     *
     * This method returns a list of all waypoints known to this GeoMapProvider (that is, the union of all waypoints in any of the installed maps)
     */
    QList<Waypoint*> waypoints() {
        QMutexLocker locker(&_aviationDataMutex);
        return _waypoints_;
    }

signals:
    /*! \brief Notification signal for the property with the same name */
    void geoJSONChanged();

    /*! \brief Notification signal for the property with the same name */
    void styleFileURLChanged();

private:
    // This method simplifies strings by replacing all special characters with
    // analogues. So, "Neuchâtel" become "Neuchatel", "Épinal" becomes "Epinal"
    // and "Allgäu" becomes "Allgau"
    QString simplifySpecialChars(const QString &string);

    // Caches used to speed up the method simplifySpecialChars
    QRegularExpression specialChars {"[^a-zA-Z0-9]"};
    QHash<QString, QString> simplifySpecialChars_cache;

    // This slot is called every time the the set of GeoJSON files changes. It
    // sets up the tile server to and generates a new style file.
    void aviationMapsChanged();
#warning docu
    QTimer _aviationDataCacheTimer;
    void fillAviationDataCache(const QStringList& geoJSONFiles, bool hideUpperAirspaces);
    QFuture<void> _aviationDataCacheFuture;

    // This slot is called every time the the set of MBTile files changes. It
    // sets up the tile server to and generates a new style file.
    void baseMapsChanged();

    // This is the path under which is tiles are available on the
    // _tileServer. This is set to a random number that changes every time the
    // set of MBTile files changes
    QString _currentPath;

    // Pointer to the MapManager
    QPointer<MapManager> _manager;

    // Pointer to global settings
    QPointer<GlobalSettings> _settings;

    // Tile Server
    TileServer _tileServer;

    // Temporary file that holds the current style file
    QPointer<QTemporaryFile> _styleFile;

    // Aviation Data Cache
    QMutex           _aviationDataMutex;
    QByteArray       _combinedGeoJSON_; // Cache: GeoJSON
    QList<Waypoint*> _waypoints_;       // Cache: Waypoints
    QList<Airspace*> _airspaces_;       // Cache: Airspaces

};

#endif
