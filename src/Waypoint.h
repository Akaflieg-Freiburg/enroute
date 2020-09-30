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
class Meteorologist;

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
    Q_INVOKABLE QVariant get(const QString& propertyName) const
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
     * @returns Property hasMAF
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
        return QString("/icons/waypoints/%1.svg").arg(get("CAT").toString());
    }

    /* \brief Check is the waypoint is valid
     *
     * This method checks if the waypoint has a valid coordinate and
     * if the properties satisfy the specifications outlined
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     *
     * @returns True if the waypoint is valid.
     */
    bool isValid() const;

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

    // ===================================

    /*! \brief Extended name of the Waypoints

    This method returns a string of the form "Karlsruhe (DVOR-DME)"
  */
    Q_PROPERTY(QString extendedName READ extendedName CONSTANT)

    /*! \brief Getter function for property with the same name

    @returns Property extendedName
  */
    Q_INVOKABLE QString extendedName() const;


#warning documentation. THIS IS NOT CONSTANT!
    Q_PROPERTY(QString color READ color NOTIFY colorChanged)
    QString color() const;

#warning documentation
    Q_PROPERTY(QString richTextName READ richTextName NOTIFY richTextNameChanged)
    QString richTextName() const;

#warning documentation
    Q_PROPERTY(QString simpleDescription READ simpleDescription CONSTANT)
    QString simpleDescription() const;


#warning documentation
    Q_PROPERTY(QObject *metar READ metar NOTIFY weatherStationChanged)
    QObject *metar() const;

    /* \brief Description of waypoint details as an HTML table */
    Q_PROPERTY(QList<QString> tabularDescription READ tabularDescription CONSTANT)

    /*! \brief Getter function for property with the same name

    @returns Property tabularDescription
    */
    QList<QString> tabularDescription() const;

    /*! \brief Description of the way from a given position to the waypoint

    @param position Position
    @param useMetricUnits if true, render distance in km, else in NM

    @returns a string of the form "DIST 65.2 NM • QUJ 276°"
    */
    Q_INVOKABLE QString wayFrom(const QGeoCoordinate& position, bool useMetricUnits) const;


signals:
    void colorChanged();
    void hasMETARChanged();
    void hasTAFChanged();
    void richTextNameChanged();
    void weatherStationChanged();

private:
    Q_DISABLE_COPY_MOVE(Waypoint)

    // Pointers to other classes that are used internally
    QPointer<Meteorologist> _meteorologist {};
    QPointer<SatNav> _satNav {};
    QPointer<GlobalSettings> _globalSettings {};

    QGeoCoordinate _coordinate;
    QMultiMap<QString, QVariant> _properties;
};
