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

class SimpleWaypoint
{
    Q_GADGET

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
    Q_INVOKABLE SimpleWaypoint();

    /*! \brief Constructs a waypoint from a coordinate
     *
     * The waypoint constructed will have property TYP="WP" and
     * property CAT="WP".
     *
     * @param coordinate Geographical position of the waypoint
     *
     * @param parent The standard QObject parent pointer
     */
    explicit SimpleWaypoint(const QGeoCoordinate& coordinate);

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
    explicit SimpleWaypoint(const QJsonObject &geoJSONObject);

    //
    // METHODS
    //

    /*! \brief Check if other waypoint is geographically near *this
     *
     *  @param other Pointer to other waypoint (nullptr is allowed)
     *
     *  @returns True if distance can be computed and is less than 2km
     */
    Q_INVOKABLE bool isNear(const GeoMaps::SimpleWaypoint& other) const;

    /*! \brief Retrieve property by name
     *
     * Recall that the waypoint data is stored as a list of properties that
     * correspond to waypoint feature of a GeoJSON file, as described in
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     * This method allows to retrieve the individual properties by name.
     *
     * @param propertyName The name of the member. This is a string such as "CAT",
     * "TYP", "NAM", etc
     *
     * @returns Value of the proerty
     */
/*    Q_INVOKABLE QVariant getPropery(const QString& propertyName) const
    {
        return _properties.value(propertyName);
    }
*/
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)
    Q_INVOKABLE QString ICAOCode() const
    {
        return _properties.value("COD").toString();
    }

    Q_PROPERTY(QString category READ category CONSTANT)
    Q_INVOKABLE QString category() const
    {
        return _properties.value("CAT").toString();
    }

    Q_PROPERTY(QString name READ name CONSTANT)
    Q_INVOKABLE QString name() const
    {
        return _properties.value("NAM").toString();
    }

    Q_PROPERTY(QString type READ type CONSTANT)
    Q_INVOKABLE QString type() const
    {
        return _properties.value("TYP").toString();
    }

    /* \brief Check if the waypoint is valid
     *
     * This method checks if the waypoint has a valid coordinate and if the
     * properties satisfy the specifications outlined
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     *
     * @returns True if the waypoint is valid.
     */
    Q_INVOKABLE bool isValid() const;

    /*! \brief Equality check
     *
     * @param other Right hand side of the equality check
     *
     * @returns True if the coordinates and all properties agree.
     */
    bool operator==(const SimpleWaypoint &other) const;

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

    /*! \brief Description of the way from a given point to the waypoint
     *
     * @param from Starting point of the way
     *
     * @param useMetric If true, then description uses metric units. Otherwise, nautical units are used.
     *
     * @returns A string such as "DIST 65.2 NM • QUJ 276°".  If the way cannot be described (e.g. because one of the coordinates is invalid), then an empty string is returned.
     */
    Q_INVOKABLE QString wayTo(const QGeoCoordinate& from, bool useMetric) const;


    //
    // PROPERTIES
    //

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordinate
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)
    Q_INVOKABLE QGeoCoordinate coordinate() const { return _coordinate; }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordinate
    */
    Q_PROPERTY(QString extendedName READ extendedName WRITE setExtendedName)

    QString extendedName() const;
    void setExtendedName(const QString &newExtendedName);

    /*! \brief Getter function for property with the same name
     *
     * @returns Property icon
     */
    Q_PROPERTY(QString icon READ icon CONSTANT)
    QString icon() const;

    /* \brief Description of waypoint properties
     *
     * This method holds a list of strings in meaningful order that describe the
     * waypoint properties.  This includes airport frequency, runway
     * information, etc. The data is returned as a list of strings where the
     * first four letters of each string indicate the type of data with an
     * abbreviation that will be understood by pilots ("RWY ", "ELEV",
     * etc.). The rest of the string will then contain the actual data.
     *
     * @returns Property tabularDescription
     */
    Q_PROPERTY(QList<QString> tabularDescription READ tabularDescription)
    QList<QString> tabularDescription() const;

    /*! \brief Two-line description of the waypoint name
     *
     * This property holds a one-line or two-line description of the
     * waypoint. Depending on available data, this is a string of the form
     * "<strong>LFKA</strong><br><font size='2'>ALBERTVILLE</font>" or simply
     * "KIRCHZARTEN"
     *
     * @see threeLineTitle
     *
     * @returns Property twoLineTitle
     */
    Q_PROPERTY(QString twoLineTitle READ twoLineTitle CONSTANT)
    QString twoLineTitle() const;

protected:
    QGeoCoordinate _coordinate;
    QMultiMap<QString, QVariant> _properties;
};

}

// Declare meta types
Q_DECLARE_METATYPE(GeoMaps::SimpleWaypoint)

