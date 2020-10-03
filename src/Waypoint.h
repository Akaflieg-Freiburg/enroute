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

#include <QGeoCoordinate>
#include <QMap>
#include <QJsonObject>
#include <QVariant>

#include "Clock.h"
#include "Meteorologist.h"
#include "SatNav.h"

class GlobalSettings;


/*! \brief Waypoint, such as an airfield, a navaid station or a reporting point.
 *
 * This class represents a waypoint.  The relevant data that describes the waypoint is
 * stored as a list of properties that can be retrieved using the method get() the
 * properties correspond to the feature of the GeoJSON files that are used in Enroute,
 * and that are described
 * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
 * There are numerous helper methods.
 */

class Waypoint : public QObject
{
    Q_OBJECT

public:
    /*! \brief Constructs an invalid way point
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Waypoint(QObject *parent = nullptr);

    /*! \brief Constructs a waypoint by copying data from another waypoint
     *
     * @param other Waypoint whose data is copied
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Waypoint(const Waypoint &other, QObject *parent = nullptr);

    /*! \brief Constructs a way point from a coordinate
     *
     * The waypoint constructed will have property TYP="WP" and
     * property CAT="WP".
     *
     * @param coordinate Geographical position of the waypoint
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Waypoint(const QGeoCoordinate& coordinate, QObject *parent = nullptr);

    /*! \brief Constructs a way point from a coordinate and sets the COD property
     *
     * The waypoint constructed will have property TYP="WP",
     * property CAT="WP" and property COD=ICAOCode.
     *
     * @param coordinate Geographical position of the waypoint
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Waypoint(const QGeoCoordinate& coordinate, QString ICAOCode, QObject *parent= nullptr);

    /*! \brief Constructs a waypoint from a GeoJSON object
     *
     * This method constructs a Waypoint from a GeoJSON description.
     * The GeoJSON file specification is found
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     *
     * @param geoJSONObject GeoJSON Object that describes the waypoint
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Waypoint(const QJsonObject &geoJSONObject, QObject *parent = nullptr);

    // Standard destructor
    ~Waypoint() = default;


    /*! \brief Check existence of a given property
     *
     * Recall that the waypoint data is stored as a list of properties that correspond to
     * waypoint feature of a GeoJSON file, as described in
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     * This method allows to check for the existence of individual properties.
     *
     * @param propertyName The name of a property. This is a string such as "CAT", "TYP", "NAM", etc
     *
     * @returns True is property exists
     */
    Q_INVOKABLE bool containsProperty(const QString& propertyName) const
    {
        return _properties.contains(propertyName);
    }

    /*! \brief Coordinate of the waypoint
     *
     * If the coordinate is invalid, this waypoint should not be used
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordinate
     */
    QGeoCoordinate coordinate() const { return _coordinate; }

    /*! \brief Extended name of the waypoint
     *
     * This property holds a string of the form "Karlsruhe (DVOR-DME)"
     */
    Q_PROPERTY(QString extendedName READ extendedName CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
    */
    QString extendedName() const;

    /*! \brief Convenience property to access the flight category color of the current METAR report
     *
     * If a pointer to a Meteorologist instance has been set with setMeteorologist()
     * and if a WeatherStation is known to exist at this point, then this property
     * holds the color code for latest METAR report. This is really a convenience, equivalent to
     * calling weatherStation()->metar()->flightCategoryColor() (after checking for nullptrs).
     * If no flight category color can be determined, then "transparent" is returned.
     */
    Q_PROPERTY(QString flightCategoryColor READ flightCategoryColor NOTIFY flightCategoryColorChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property flightCategoryColor
     */
    QString flightCategoryColor() const;

    /*! \brief Retrieve property by name
     *
     * Recall that the waypoint data is stored as a list of properties that correspond to
     * waypoint feature of a GeoJSON file, as described in
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     * This method allows to retrieve the individual properties by name.
     *
     * @param name The name of the member. This is a string such as "CAT", "TYP", "NAM", etc
     *
     * @returns Value of the proerty
     */
    Q_INVOKABLE QVariant getPropery(const QString& propertyName) const
    {
        return _properties.value(propertyName);
    }

    /*! \brief Check if a METAR weather report is known for the waypoint
     *
     * If a pointer to a Meteorologist instance has been set with setMeteorologist(),
     * this convenience property can be used to check if a METAR report is available
     * for the waypoint.  The actual METAR report can be accessed via the
     * property weatherStation.
     */
    Q_PROPERTY(bool hasMETAR READ hasMETAR NOTIFY hasMETARChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasMETAR
     */
    bool hasMETAR() const;

    /*! \brief Check if a TAF weather forcast is known for the waypoint
     *
     * If a pointer to a Meteorologist instance has been set with setMeteorologist(),
     * this convenience property can be used to check if a TAF forcast is available
     * for the waypoint.  The actual TAF report can be accessed via the
     * property weatherStation.
     */
    Q_PROPERTY(bool hasTAF READ hasTAF NOTIFY hasTAFChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasTAF
     */
    bool hasTAF() const;

    /*! \brief Suggested icon file
     *
     * This property holds the name of an icon file in SVG format
     * that best describes the waypoint.
     */
    Q_PROPERTY(QString icon READ icon CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property icon
     */
    QString icon() const
    {
        return QString("/icons/waypoints/%1.svg").arg(getPropery("CAT").toString());
    }

    /* \brief Check if the waypoint is valid
     *
     * This method checks if the waypoint has a valid coordinate and
     * if the properties satisfy the specifications outlined
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     *
     * @returns True if the waypoint is valid.
     */
    bool isValid() const;

    /*! \brief Convenience property to access the summary of the current METAR report
     *
     * If a pointer to a Meteorologist instance has been set with setMeteorologist()
     * and if a WeatherStation is known to exist at this point, then this property
     * holds the summary of the latest METAR report. This is really a convenience, equivalent to
     * calling weatherStation()->metar()->summary() (after checking for nullptrs).
     * If no METAR is found, then an empty string is returned.
     */
    Q_PROPERTY(QString METARSummary READ METARSummary NOTIFY METARSummaryChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property METARSummary
     */
    QString METARSummary() const;

    /*! \brief Set optional pointer to a GlobalSettings instance, in order to improve messages
     *
     * This method can be used to equip the waypoint instance with global settings information.
     * If available, this information is used for instance by the method
     * richTextName() to present distance from the current position in
     * metric or imperial units, according to the user's choice.
     *
     * @param globalSettings Pointer to a GlobalSettings instance.
     */
    void setGlobalSettings(GlobalSettings *globalSettings=nullptr);

    /*! \brief Set optional pointer to a Meteorologist instance, in order to access weather information
     *
     * This method can be used to equip the waypoint instance with Weather information.
     * If available, this information is used for instance by the method
     * weatherStation() to check if there is a known weather station at the waypoint-
     *
     * @param meteorologist Pointer to a Meteorologist instance.
     */
    void setMeteorologist(Meteorologist *meteorologist=nullptr);

    /*! \brief Set optional pointer to a SatNav instance, in order to improve messages
     *
     * This method can be used to equip the waypoint instance with SatNav information.
     * If available, this information is used for instance by the method
     * richtTextName to print the distance from the current position to the waypoint.
     *
     * @param satNav Pointer to a SatNav instance.
     */
    void setSatNav(SatNav *satNav=nullptr);

    /* \brief Description of waypoint properties
     *
     * This method holds a list of strings in meaningful order that describe the waypoint properties.
     * This includes airport frequency, runway information, etc. The data is returned as a list
     * of strings where the first four letters of each string indicate the type of data with
     * an abbreviation that will be understood by pilots ("RWY ", "ELEV", etc.). The rest of the string will then contain
     * the actual data.
     */
    Q_PROPERTY(QList<QString> tabularDescription READ tabularDescription CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property tabularDescription
     */
    QList<QString> tabularDescription() const;

    /*! \brief Serialization to GeoJSON object
     *
     * This method serialises the waypoint as a GeoJSON object. The object
     * conforms to the specification outlined
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     * The waypoint can be restored with the obvious constructor
     *
     * @returns QJsonObject describing the waypoint
     */
    QJsonObject toJSON() const;

    /*! \brief Two-line description of the waypoint name
     *
     * This property holds a one-line or two-line description of the waypoint. Depending on available data, this is a
     * string of the form "<strong>LFKA</strong><br><font size='2'>ALBERTVILLE</font>" or simply "KIRCHZARTEN"
     *
     * @see threeLineTitle
     */
    Q_PROPERTY(QString twoLineTitle READ twoLineTitle CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property twoLineTitle
     */
    QString twoLineTitle() const;

    /*! \brief Description of the way from the current position to the waypoint
     *
     * If a pointer to a SatNav instance has been set with setSatNav(), then this property
     * describes that way from the current position to the waypoint.  If a pointer to a GlobalSettings instance has been set with setGlobalSettings(), then
     * user preferences (metric units versus miles) will be taken into account. The result is then a string such as "DIST 65.2 NM • QUJ 276°".
     * If the way cannot be described (e.g. because the current position is not known), an empty string is returned.
    */
    Q_PROPERTY(QString wayTo READ wayTo NOTIFY wayToChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property wayTo
     */
    QString wayTo() const;

    /*! \brief Check if a WeatherStation exists at this point
     *
     * If a pointer to a Meteorologist instance has been set with setMeteorologist()
     * and if a WeatherStation is known to exist at this point, then this property
     * holds a pointer to the station.  Otherwise, the property holds nullptr.
     * Note that the WeatherStation object is owned by the Meteorologist and that
     * it can be deleted anytime.
     */
    Q_PROPERTY(Meteorologist::WeatherStation *weatherStation READ weatherStation NOTIFY weatherStationChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property weatherStation
     */
    Meteorologist::WeatherStation *weatherStation() const;

    /*! \brief Equality check
     *
     * @returns True if the coordinates and all properties agree.
     */
    bool operator==(const Waypoint &other) const;

signals:
    /*! \brief Notifier signal */
    void flightCategoryColorChanged();

    /*! \brief Notifier signal */
    void hasMETARChanged();

    /*! \brief Notifier signal */
    void hasTAFChanged();

    /*! \brief Notifier signal */
    void METARSummaryChanged();

    /*! \brief Notifier signal */
    void weatherStationChanged();

    /*! \brief Notifier signal */
    void wayToChanged();

private:
    Q_DISABLE_COPY_MOVE(Waypoint)

    // Pointers to other classes that are used internally
    QPointer<GlobalSettings> _globalSettings {};
    QPointer<Meteorologist> _meteorologist {};
    QPointer<SatNav> _satNav {};

    QGeoCoordinate _coordinate;
    QMultiMap<QString, QVariant> _properties;
};
