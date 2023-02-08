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

#include <QFile>
#include <QGeoRectangle>
#include <QJsonDocument>
#include <QLocale>
#include <QPointer>
#include <QQmlEngine>
#include <QXmlStreamReader>

#include "geomaps/Waypoint.h"
#include "navigation/Leg.h"

namespace GeoMaps
{
    class GeoMapProvider;
    class Waypoint;
} // namespace GeoMaps

class GlobalSettings;

namespace Navigation
{

    /*! \brief Intended flight route
     *
     * This class represents an intended flight route. In essence, this class is
     * little more than a list of waypoint and a number of methods that do the
     * following.
     *
     * - Expose the list of waypoints and legs to QML and allow some
     *   manipulation from there, such as adding or re-arranging waypoints.
     *
     * - Compute length and true course for the legs in the flight path, as well
     *   as a total length and expose this data to QML.
     */

    class FlightRoute : public QObject
    {
        Q_OBJECT
        QML_ELEMENT

    public:
        //
        // Constructors and destructors
        //

        /*! \brief Construct a flight route
         *
         * This default constructor calls load(), restoring the last saved
         * route. The route is saved to a standard location whenever it changes,
         * so that the route survives when the app is closed unexpectantly.
         *
         * @param parent The standard QObject parent pointer.
         */
        explicit FlightRoute(QObject *parent = nullptr);

        // Standard destructor
        ~FlightRoute() override = default;

        //
        // PROPERTIES
        //

        /*! \brief Bounding rectangle
         *
         *  This is a QGeoRectangle that contains the route. The rectangle might
         *  be invalid, for instance if the route is empty.
         */
        Q_PROPERTY(QGeoRectangle boundingRectangle READ boundingRectangle NOTIFY waypointsChanged)

        /*! \brief List of coordinates for the waypoints
         *
         * This property holds a list of coordinates of the waypoints, suitable
         * for drawing the flight path on a QML map.
         */
        Q_PROPERTY(QList<QGeoCoordinate> geoPath READ geoPath NOTIFY waypointsChanged)

        /*! \brief List of waypoints in the flight route that are not airfields
         *
         * This property lists all the waypoints in the route that are not
         * airfields, navaids, reporting points, etc.
         */
        Q_PROPERTY(QList<GeoMaps::Waypoint> midFieldWaypoints READ midFieldWaypoints NOTIFY waypointsChanged)

        /*! \brief List of legs
         *
         * This property returns a list of all legs in the route.
         */
        Q_PROPERTY(QList<Navigation::Leg> legs READ legs NOTIFY waypointsChanged)

        /*! \brief Number of waypoints in the route */
        Q_PROPERTY(qsizetype size READ size NOTIFY waypointsChanged)

        /*! \brief Human-readable summary of the flight route
         *
         *  This is a string of the form "48 nm · 0:28 h · 1,9 gal", potentially
         *  with HTML complaints if wind or aircraft data was missing.
         *
         *  The summary is computed for the aircraft and wind that are presently
         *  set in the global Navigator class.
         */
        Q_PROPERTY(QString summary READ summary NOTIFY summaryChanged)

        /*! \brief List of waypoints in the flight route that are not airfields
         *
         * This property lists all the waypoints in the route that are not
         * airfields, navaids, reporting points, etc.
         */
        Q_PROPERTY(QList<GeoMaps::Waypoint> waypoints READ waypoints NOTIFY waypointsChanged)

        //
        // Getter Methods
        //

        /*! \brief Getter function for the property with the same name
         *
         *  @returns Property boundingRectangle
         */
        [[nodiscard]] auto boundingRectangle() const -> QGeoRectangle;

        /*! \brief Getter function for the property with the same name
         *
         *  @returns Property geoPath
         */
        [[nodiscard]] auto geoPath() const -> QList<QGeoCoordinate>;

        /*! \brief Getter function for the property with the same name
         *
         * @returns Property midFieldWaypoints
         */
        [[nodiscard]] auto midFieldWaypoints() const -> QList<GeoMaps::Waypoint>;

        /*! \brief Getter function for the property with the same name
         *
         * @returns Property legs
         */
        [[nodiscard]] auto legs() const -> QList<Navigation::Leg> { return m_legs; }

        /*! \brief Getter function for the property with the same name
         *
         * @returns Property size
         */
        [[nodiscard]] auto size() const -> qsizetype { return m_waypoints.size(); }

        /*! \brief Getter function for the property with the same name
         *
         *  @returns Property summary
         */
        [[nodiscard]] auto summary() const -> QString;

        /*! \brief Getter function for the property with the same name
         *
         * @returns Property waypoints
         */
        [[nodiscard]] auto waypoints() const -> QList<GeoMaps::Waypoint> {return m_waypoints;}


        //
        // METHODS
        //

        /*! \brief Adds a waypoint to the end of the route
         *
         * @param waypoint Waypoint to be added
         */
        Q_INVOKABLE void append(const GeoMaps::Waypoint& waypoint);

        /*! \brief Adds a waypoint to the end of the route
         *
         * This method generates a generic waypoint with the given coordinates.
         *
         * @param position Coordinates of the waypoint.
         */
        Q_INVOKABLE void append(const QGeoCoordinate& position);

        /*! \brief Checks if waypoint can be added as the new end of this
         *  route
         *
         *  @param other Waypoint to be appended
         *
         *  @returns True if route is empty or if other waypoint is not near
         *  the current end of the route.
         */
        Q_INVOKABLE [[nodiscard]] bool canAppend(const GeoMaps::Waypoint& other) const;

        /*! \brief Checks if waypoint can reasonably be inserted into this route
         *
         *  @param other Waypoint to be inserted
         *
         *  @returns True if it makes sense to insert the waypoint.
         */
        Q_INVOKABLE [[nodiscard]] bool canInsert(const GeoMaps::Waypoint& other) const;

        /*! \brief Deletes all waypoints in the current route */
        Q_INVOKABLE void clear();

        /*! \brief Returns true if waypoint is in this route
         *
         * @param waypoint Waypoint
         *
         * @returns bool Returns true if waypoint geographically close to a
         * waypoint in the route
         */
        Q_INVOKABLE [[nodiscard]] bool contains(const GeoMaps::Waypoint& waypoint) const;

        /*! \brief Inserts a waypoint into the route
         *
         *  Inserts the waypoint into the route, at the place that minimizes the
         *  overall route length. If canInsert() is false, this method does nothing.
         *
         * @param wp Waypoint to be inserted.
         */
        Q_INVOKABLE void insert(const GeoMaps::Waypoint& wp);

        /*! \brief Index for last occurrence of the waypoint in the flight route
         *
         *  This method finds the index position of the last waypoint in the
         *  route that is geograhphically close to the given waypoint.
         *
         *  @param waypoint Waypoint to be searched
         *
         *  @returns Index position of the last waypoint in the route close to
         *  the given waypoint. Returns -1 if no waypoint is close.
         */
        Q_INVOKABLE [[nodiscard]] qsizetype lastIndexOf(const GeoMaps::Waypoint& waypoint) const;

        /*! \brief Loads the route from a GeoJSON or GPX document
         *
         * This method loads the flight route from a GeoJSON or GPX file. The
         * method detects waypoints (such as airfields) by looking at the
         * coordinates.
         *
         * @param fileName File name, needs to include path and extension
         *
         * @returns Empty string in case of success, human-readable, translated
         * error message otherwise.
         */
        Q_INVOKABLE QString load(const QString& fileName);

        /*! \brief Move waypoint one position down in the list of waypoints
         *
         * @param idx Index of the waypoint
         */
        Q_INVOKABLE void moveDown(int idx);

        /*! \brief Move waypoint one position up in the list of waypoints
         *
         * @param idx Index of the waypoint
         */
        Q_INVOKABLE void moveUp(int idx);

        /*! \brief Remove waypoint from the current route
         *
         * If the waypoint is contained in the route, the method returns
         * immediately.
         *
         * @param idx Index of the waypoint
         */
        Q_INVOKABLE void removeWaypoint(int idx);

        /*! \brief Replaces a waypoint
         *
         *  Replaces the waypoint with the given index. If the index is invalid
         *  if the new waypoint equals the old one, then this method does nothing.
         *  The signal "waypoint changed" is emitted as appropriate.
         *
         *  @param idx Index of waypoint
         *
         *  @param newWaypoint New waypoint
         */
        Q_INVOKABLE void replaceWaypoint(int idx, const GeoMaps::Waypoint& newWaypoint);

        /*! \brief Reverse the route */
        Q_INVOKABLE void reverse();

        /*! \brief Saves flight route to a file
         *
         * This method saves the flight route as a GeoJSON file.  The file
         * conforms to the specification outlined
         * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
         *
         * @param fileName File name, needs to include path and extension
         *
         * @returns Empty string in case of success, human-readable, translated
         * error message otherwise.
         */
        Q_INVOKABLE [[nodiscard]] QString save(const QString& fileName = QString()) const;

        /*! \brief Suggests a name for saving this route
         *
         * This method suggests a name for saving the present route (without
         * path and file extension).
         *
         * @returns Suggested name for saving the file. If no useful suggestion
         * can be made, the returned string is a translation of "Flight Route"
         */
        Q_INVOKABLE [[nodiscard]] QString suggestedFilename() const;

        /*! \brief Exports to route to GeoJSON
         *
         * This method serialises the current flight route as a GeoJSON
         * document. The document conforms to the specification outlined
         * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
         *
         * @returns QByteArray describing the flight route
         */
        Q_INVOKABLE [[nodiscard]] QByteArray toGeoJSON() const;

        /*! \brief Exports to route to GPX
         *
         * This method serialises the current flight route as a GPX document.
         * The document conforms to the specification outlined
         * [here](https://www.topografix.com/gpx.asp)
         *
         * @returns QByteArray containing GPX data describing the flight route
         */
        Q_INVOKABLE [[nodiscard]] QByteArray toGpx() const;

    signals:
        /*! \brief Notification signal for the property with the same name */
        void waypointsChanged();

        /*! \brief Notification signal for the property with the same name */
        void summaryChanged();

    private slots:
        // Saves the route into the file stdFileName. This slot is called
        // whenever the route changes, so that the file will always contain the
        // current route.
        void saveToStdLocation() { (void)save(stdFileName); };

        void updateLegs();

    private:
        Q_DISABLE_COPY_MOVE(FlightRoute)

        // Helper function for method toGPX
        [[nodiscard]] auto gpxElements(const QString& indent, const QString& tag) const -> QString;

        // File name where the flight route is loaded upon startup are stored.
        // This member is filled in in the constructor to
        // QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        // "/flight route.geojson"
        QString stdFileName;

        QVector<GeoMaps::Waypoint> m_waypoints;

        QVector<Leg> m_legs;

        QLocale myLocale;
    };

} // namespace Navigation
