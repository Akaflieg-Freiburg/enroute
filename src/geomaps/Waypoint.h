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

#include <QGeoCoordinate>
#include <QMap>
#include <QJsonObject>

#include "geomaps/SimpleWaypoint.h"
#include "weather/DownloadManager.h"


namespace GeoMaps {

/*! \brief Waypoint, such as an airfield, a navaid station or a reporting point.
 *
 * This class represents a waypoint.  The relevant data that describes the
 * waypoint is stored as a list of properties that can be retrieved using the
 * method get() the properties correspond to the feature of the GeoJSON files
 * that are used in Enroute, and that are described
 * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
 * There are numerous helper methods.
 */

class Waypoint : public QObject, public SimpleWaypoint
{
    Q_OBJECT

public:
    /*! \brief Constructs an invalid way point
     *
     * This default constructor creates a waypoint with the following properties
     *
     * - "CAT" is set to "WP"
     * - "NAM" is set to "Waypoint"
     * - "TYP" is set to "WP"
     *
     * The coordinate is invalid.
     *
     * @param parent The standard QObject parent pointer
     */
    Q_INVOKABLE explicit Waypoint(QObject *parent = nullptr);

    /*! \brief Constructs a waypoint by copying data from another waypoint
     *
     * @param other Waypoint whose data is copied
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Waypoint(const GeoMaps::Waypoint &other, QObject *parent = nullptr);

#warning
    explicit Waypoint(const GeoMaps::SimpleWaypoint &other, QObject *parent = nullptr);
    Q_INVOKABLE void copyFrom(const GeoMaps::SimpleWaypoint &other);

    /*! \brief Constructs a waypoint from a coordinate
     *
     * The waypoint constructed will have property TYP="WP" and
     * property CAT="WP".
     *
     * @param coordinate Geographical position of the waypoint
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Waypoint(const QGeoCoordinate& coordinate, QObject *parent = nullptr);

    /*! \brief Constructs a waypoint from a GeoJSON object
     *
     * This method constructs a Waypoint from a GeoJSON description.  The
     * GeoJSON file specification is found
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     *
     * @param geoJSONObject GeoJSON Object that describes the waypoint
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Waypoint(const QJsonObject &geoJSONObject, QObject *parent = nullptr);

    // Standard destructor
    ~Waypoint() = default;

    //
    // METHODS
    //

    /*! \brief Equality check
     *
     * @param other Right hand side of the equality check
     *
     * @returns True if the coordinates and all properties agree.
     */
    bool operator==(const Waypoint &other) const;

    /*! \brief Connects this waypoint with a DownloadManager instance
     *
     * This method optionally connects the waypoint with an instance of the
     * DownloadManager class.  Once connected, functions such has hasTAF can be
     * used.
     *
     * @param downloadManager Pointer to a download manager
     */
    void setDownloadManager(Weather::DownloadManager *downloadManager);


    //
    // PROPERTIES
    //

    /*! \brief Coordinate of the waypoint
     *
     * If the coordinate is invalid, this waypoint should not be used
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Extended name of the waypoint
     *
     * This property holds a string of the form "Karlsruhe (DVOR-DME)"
     */
    Q_PROPERTY(QString extendedName READ extendedName WRITE setExtendedName NOTIFY extendedNameChanged)

    /*! \brief Setter function for property with the same name
     *
     * @param newExtendedName Property extendedName
    */
    void setExtendedName(const QString &newExtendedName);

    /*! \brief Check if a METAR weather report is known for the waypoint
     *
     * If a pointer to a DownloadManager instance has been set with
     * setDownloadManager(), this convenience property can be used to check if a
     * METAR report is available for the waypoint.  The actual METAR report can
     * be accessed via the property weatherStation.
     */
    Q_PROPERTY(bool hasMETAR READ hasMETAR NOTIFY hasMETARChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasMETAR
     */
    bool hasMETAR() const;

    /*! \brief Check if a TAF weather forcast is known for the waypoint
     *
     * If a pointer to a DownloadManager instance has been set with
     * setDownloadManager(), this convenience property can be used to check if a
     * TAF forcast is available for the waypoint.  The actual TAF report can be
     * accessed via the property weatherStation.
     */
    Q_PROPERTY(bool hasTAF READ hasTAF NOTIFY hasTAFChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasTAF
     */
    bool hasTAF() const;

    /*! \brief Suggested icon file
     *
     * This property holds the name of an icon file in SVG format that best
     * describes the waypoint.
     */
#warning is this constant?
    Q_PROPERTY(QString icon READ icon CONSTANT)


    /* \brief Description of waypoint properties
     *
     * This method holds a list of strings in meaningful order that describe the
     * waypoint properties.  This includes airport frequency, runway
     * information, etc. The data is returned as a list of strings where the
     * first four letters of each string indicate the type of data with an
     * abbreviation that will be understood by pilots ("RWY ", "ELEV",
     * etc.). The rest of the string will then contain the actual data.
     */
    Q_PROPERTY(QList<QString> tabularDescription READ tabularDescription  NOTIFY extendedNameChanged)

    /*! \brief Two-line description of the waypoint name
     *
     * This property holds a one-line or two-line description of the
     * waypoint. Depending on available data, this is a string of the form
     * "<strong>LFKA</strong><br><font size='2'>ALBERTVILLE</font>" or simply
     * "KIRCHZARTEN"
     *
     * @see threeLineTitle
     */
    Q_PROPERTY(QString twoLineTitle READ twoLineTitle NOTIFY extendedNameChanged)

    /*! \brief Check if a WeatherStation exists at this point
     *
     * If a pointer to a DownloadManager instance has been set with setDownloadManager()
     * and if a WeatherStation is known to exist at this point, then this property
     * holds a pointer to the station.  Otherwise, the property holds nullptr.
     * Note that the WeatherStation object is owned by the DownloadManager and that
     * it can be deleted anytime.
     */
    Q_PROPERTY(Weather::Station *weatherStation READ weatherStation NOTIFY weatherStationChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property weatherStation
     */
    Weather::Station *weatherStation() const;

    Q_INVOKABLE QString wayTo(const QGeoCoordinate& from, bool useMetric) const
    {
        return SimpleWaypoint::wayTo(from, useMetric);
    }


signals:
    /*! \brief Notifier signal */
    void extendedNameChanged();

    /*! \brief Notifier signal */
    void hasMETARChanged();

    /*! \brief Notifier signal */
    void hasTAFChanged();

    /*! \brief Notifier signal */
    void weatherStationChanged();


private:
    Q_DISABLE_COPY_MOVE(Waypoint)

    // This method is called when the _DownloadManager is set, and then every time
    // the weather station for this waypoint changes (e.g. because the
    // DownloadManager receives a new list of stations, or the existing weather
    // station gets destructed). It wires the weather station up and emits
    // appropriate signals.
    void initializeWeatherStationConnections();

    // Pointers to other classes that are used internally
#warning Want this global
    QPointer<Weather::DownloadManager> _downloadManager {};

    // Guarded and unguarded pointers
    Weather::Station *_weatherStation_unguarded {nullptr};
    QPointer<Weather::Station> _weatherStation_guarded;
};

}
