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
 * This class represents a waypoint.  The properties stored in this class correspond to the feature of the GeoJSON files
 * that are used in Enroute, as described
 * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
 */

class Waypoint
{
    Q_GADGET

public:
    /*! \brief Constructs an invalid way point
     *
     * This default constructor creates a waypoint with the following properties
     *
     * - category is set to "WP"
     * - name is set to "Waypoint"
     * - type is set to "WP"
     *
     * The coordinate is invalid.
     */
    Waypoint();

    /*! \brief Constructs a waypoint from a coordinate
     *
     * - category is set to "WP"
     * - name is set to "Waypoint"
     * - type is set to "WP"
     *
     * @param coordinate Geographical position of the waypoint
     */
    Waypoint(const QGeoCoordinate& coordinate);

    /*! \brief Constructs a waypoint from a GeoJSON object
     *
     * This method constructs a Waypoint from a GeoJSON description.  The
     * GeoJSON file specification is found
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     *
     * @param geoJSONObject GeoJSON Object that describes the waypoint
     */
    explicit Waypoint(const QJsonObject &geoJSONObject);


    //
    // METHODS
    //

    /*! \brief Equality check
     *
     * This method overloads operator==. It exists for the benefit of QML, where comparison is difficult.
     *
     * @param other Right hand side of the equality check
     *
     * @returns True if the coordinates and all properties agree.
     */
    Q_INVOKABLE bool equals(const GeoMaps::Waypoint &other) const
    {
        return *this == other;
    }

    /*! \brief Check if other waypoint is geographically near *this
     *
     *  @param other Other waypoint
     *
     *  @returns True if both waypoints are valid and if tge distance between them is less than 2km
     */
    Q_INVOKABLE bool isNear(const GeoMaps::Waypoint& other) const;

    /*! \brief Equality check
     *
     * @param other Right hand side of the equality check
     *
     * @returns True if the coordinates and all properties agree.
     */
    bool operator==(const Waypoint &other) const = default;

    /*! \brief Copy waypoint and change name
     *
     *  @param newName New name of the waypoint
     *
     *  @returns Copy of the waypoints with name changed
     */
    Q_REQUIRED_RESULT Q_INVOKABLE GeoMaps::Waypoint renamed(const QString &newName) const;

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


    //
    // PROPERTIES
    //

    /*! \brief Category of the waypoint
     *
     *  This property holds the category of  the waypoint.  For a list of possible values and explanations, see the
     *  GeoJSON file specification
     *  [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     */
    Q_PROPERTY(QString category READ category CONSTANT)

    /*! \brief Getter method for property with same name
     *
     *  @returns Property category
     */
    QString category() const
    {
        return m_properties.value("CAT").toString();
    }

    /*! \brief Coordinate of the waypoint
     *
     *  This property holds the category of the waypoint. The coordinate might include the elevation.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordinate
     */
    QGeoCoordinate coordinate() const
    {
        return m_coordinate;
    }

    /*! \brief Extended name of the waypoint
     *
     * This property holds an extended name string of the form "Karlsruhe (DVOR-DME)"
     */
    Q_PROPERTY(QString extendedName READ extendedName CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
    */
    QString extendedName() const;

    /*! \brief ICAO Code of the waypoint
     *
     *  This property holds the four-letter ICAO code of the waypoint, or an empty
     *  string if no ICAO code exists.
     */
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)

    /*! \brief Getter method for property with same name
     *
     *  @returns Property ICAOCode
     */
    QString ICAOCode() const
    {
        return m_properties.value("COD").toString();
    }

    /*! \brief Suggested icon for use in GUI
     *
     *  This property holds the URL of an icon, suitable for the representation of the waypoint in the GUI.
     */
    Q_PROPERTY(QString icon READ icon CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property icon
     */
    QString icon() const;

    /* \brief Validity
     *
     * This property is set to true if the waypoint has a valid coordinate and if the
     * properties satisfy the specifications outlined
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Getter method for property with same name
     *
     *  @returns Property isValid
     */
    bool isValid() const
    {
        return m_isValid;
    }

    /*! \brief Name of the waypoint
     *
     *  This property holds the name of the waypoint.
     */
    Q_PROPERTY(QString name READ name CONSTANT)

    /*! \brief Getter method for property with same name
     *
     *  @returns Property name
     */
    QString name() const
    {
        return m_properties.value("NAM").toString();
    }

    /* \brief Verbose description of waypoint properties
     *
     * This property holds a list of strings in meaningful order that describe the
     * waypoint properties.  This includes airport frequency, runway
     * information, etc. The data is returned as a list of strings where the
     * first four letters of each string indicate the type of data with an
     * abbreviation that will be understood by pilots ("RWY ", "ELEV",
     * etc.). The rest of the string will then contain the actual data.
     */
    Q_PROPERTY(QList<QString> tabularDescription READ tabularDescription)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property tabularDescription
     */
    QList<QString> tabularDescription() const;

    /*! \brief Two-line description of the waypoint name, for use in GUI
     *
     * This property holds a one-line or two-line description of the
     * waypoint. Depending on available data, this is a string of the form
     * "<strong>LFKA</strong><br><font size='2'>ALBERTVILLE</font>" or simply
     * "KIRCHZARTEN"
     */
    Q_PROPERTY(QString twoLineTitle READ twoLineTitle CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property twoLineTitle
     */
    QString twoLineTitle() const;

    /*! \brief Type of the waypoint
     *
     *  This property holds the type of the waypoint.  For a list of possible values and explanations, see the
     *  GeoJSON file specification
     *  [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     */
    Q_PROPERTY(QString type READ type CONSTANT)

    /*! \brief Getter method for property with same name
     *
     *  @returns Property type
     */
    QString type() const
    {
        return m_properties.value("TYP").toString();
    }

private:
    // Computes the property isValid; this is used by the constructors to set the cached value
    bool computeIsValid() const;

protected:
    bool m_isValid {false};
    QGeoCoordinate m_coordinate;
    QMultiMap<QString, QVariant> m_properties;
};

}

// Declare meta types
Q_DECLARE_METATYPE(GeoMaps::Waypoint)
