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

#include <QGeoCoordinate>
#include <QJsonObject>
#include <QMap>
#include <QQmlEngine>
#include <QXmlStreamWriter>


namespace GeoMaps {

/*! \brief Waypoint, such as an airfield, a navaid station or a reporting point.
 *
 * This class represents a waypoint.  The properties stored in this class
 * correspond to the feature of the GeoJSON files that are used in Enroute, as
 * described
 * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
 */

class Waypoint
{
    Q_GADGET
    QML_VALUE_TYPE(waypoint)

    /*! \brief qHash */
    friend auto qHash(const GeoMaps::Waypoint& wp) -> size_t;

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
    explicit Waypoint(const QJsonObject& geoJSONObject);


    //
    // PROPERTIES
    //

    /*! \brief Category of the waypoint
     *
     *  This property holds the category of  the waypoint.  For a list of
     *  possible values and explanations, see the GeoJSON file specification
     *  [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     */
    Q_PROPERTY(QString category READ category CONSTANT)

    /*! \brief Coordinate of the waypoint
     *
     *  This property holds the category of the waypoint. The coordinate might
     *  include the elevation.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate WRITE setCoordinate)

    /*! \brief Extended name of the waypoint
     *
     * This property holds an extended name string of the form "Karlsruhe
     * (DVOR-DME)"
     */
    Q_PROPERTY(QString extendedName READ extendedName)

    /*! \brief ICAO Code of the waypoint
     *
     *  This property holds the four-letter ICAO code of the waypoint, or an
     *  empty string if no ICAO code exists.
     */
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)

    /*! \brief Suggested icon for use in GUI
     *
     *  This property holds the URL of an icon, suitable for the representation
     *  of the waypoint in the GUI.
     */
    Q_PROPERTY(QString icon READ icon CONSTANT)

    /* \brief Validity
     *
     * This property is set to true if the waypoint has a valid coordinate and
     * if the properties satisfy the specifications outlined
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     */
    Q_PROPERTY(bool isValid READ isValid)

    /*! \brief Name of the waypoint
     *
     *  This property holds the name of the waypoint.
     */
    Q_PROPERTY(QString name READ name WRITE setName)

    /*! \brief Notes attached to the waypoint
     *
     *  This property holds notes attached to the waypoint.
     */
    Q_PROPERTY(QString notes READ notes WRITE setNotes)

    /*! \brief Short name of the waypoint
     *
     *  This property holds the ICAO code if it exists, or else the name of the
     *  waypoint.
     */
    Q_PROPERTY(QString shortName READ shortName CONSTANT)

    /* \brief Verbose description of waypoint properties
     *
     * This property holds a list of strings in meaningful order that describe
     * the waypoint properties.  This includes airport frequency, runway
     * information, etc. The data is returned as a list of strings where the
     * first four letters of each string indicate the type of data with an
     * abbreviation that will be understood by pilots ("RWY ", "ELEV", etc.).
     * The rest of the string will then contain the actual data.
     */
    Q_PROPERTY(QList<QString> tabularDescription READ tabularDescription)

    /*! \brief Two-line description of the waypoint name, for use in GUI
     *
     * This property holds a one-line or two-line description of the waypoint.
     * Depending on available data, this is a string of the form
     * "<strong>LFKA</strong><br><font size='2'>ALBERTVILLE</font>" or simply
     * "KIRCHZARTEN"
     */
    Q_PROPERTY(QString twoLineTitle READ twoLineTitle)

    /*! \brief Type of the waypoint
     *
     *  This property holds the type of the waypoint.  For a list of possible
     *  values and explanations, see the GeoJSON file specification
     *  [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     */
    Q_PROPERTY(QString type READ type CONSTANT)


    //
    // GETTER METHODS
    //

    /*! \brief Getter method for property with same name
     *
     *  @returns Property category
     */
    [[nodiscard]] auto category() const -> QString
    {
        return m_properties.value(QStringLiteral("CAT")).toString();
    }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordinate
     */
    [[nodiscard]] auto coordinate() const -> QGeoCoordinate
    {
        return m_coordinate;
    }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
    */
    [[nodiscard]] auto extendedName() const -> QString;

    /*! \brief Getter method for property with same name
     *
     *  @returns Property ICAOCode
     */
    [[nodiscard]] auto ICAOCode() const -> QString
    {
        return m_properties.value(QStringLiteral("COD")).toString();
    }

    /*! \brief Getter method for property with the same name
     *
     * @returns Property icon
     */
    [[nodiscard]] auto icon() const -> QString;

    /*! \brief Getter method for property with same name
     *
     *  @returns Property isValid
     */
    [[nodiscard]] auto isValid() const -> bool;

    /*! \brief Getter method for property with same name
     *
     *  @returns Property name
     */
    [[nodiscard]] auto name() const -> QString
    {
        return m_properties.value(QStringLiteral("NAM")).toString();
    }

    /*! \brief Getter method for property with same name
     *
     *  @returns Property notes
     */
    [[nodiscard]] auto notes() const -> QString
    {
        return m_properties.value(QStringLiteral("NOT")).toString();
    }

    /*! \brief Getter method for property with same name
     *
     *  @returns Property shortName
     */
    [[nodiscard]] auto shortName() const -> QString
    {
        if (ICAOCode().isEmpty()) {
            return name();
        }
        return ICAOCode();
    }

    /*! \brief Getter method for property with the same name
     *
     * @returns Property tabularDescription
     */
    [[nodiscard]] auto tabularDescription() const -> QList<QString>;

    /*! \brief Getter function for property with the same name
     *
     * @returns Property twoLineTitle
     */
    [[nodiscard]] auto twoLineTitle() const -> QString;

    /*! \brief Getter method for property with same name
     *
     *  @returns Property type
     */
    [[nodiscard]] auto type() const -> QString
    {
        return m_properties.value(QStringLiteral("TYP")).toString();
    }


    //
    // SETTER METHODS
    //

    /*! \brief Set coordinate
     *
     *  @param newCoordinate New coordinate of the waypoint
     */
    void setCoordinate(const QGeoCoordinate& newCoordinate)
    {
        m_coordinate = newCoordinate;
    }

    /*! \brief Set name
     *
     *  @param newName New name of the waypoint
     */
    void setName(const QString &newName)
    {
        m_properties.insert(QStringLiteral("NAM"), newName);
    }

    /*! \brief Set notes
     *
     *  @param newNotes New notes attached to the waypoint
     */
    void setNotes(const QString &newNotes)
    {
        m_properties.insert(QStringLiteral("NOT"), newNotes);
    }


    //
    // METHODS
    //

    /*! \brief Comparison
     *
     *  @param other Waypoint to compare with
     *
     *  @returns Result of comparison
     */
    Q_INVOKABLE [[nodiscard]] bool operator==(const GeoMaps::Waypoint& other) const = default;

    /*! \brief Comparison
     *
     *  @param other Waypoint to compare with
     *
     *  @returns Result of comparison
     */
    Q_INVOKABLE [[nodiscard]] bool operator!=(const GeoMaps::Waypoint& other) const = default;

    /*! \brief Deep copy
     *
     *  This method exists for the benefit of QML, where deep copies are hard to produce.
     *
     * @returns Copy of the present waypoint.
     */
    Q_INVOKABLE [[nodiscard]] GeoMaps::Waypoint copy() const
    {
        return *this;
    }

    /*! \brief Check if other waypoint is geographically near *this
     *
     *  @param other Other waypoint
     *
     *  @returns True if both waypoints are valid and if the distance between
     *  them is less than 2km
     */
    Q_INVOKABLE [[nodiscard]] bool isNear(const GeoMaps::Waypoint& other) const;

    /*! \brief Serialization to GeoJSON object
     *
     * This method serialises the waypoint as a GeoJSON object. The object
     * conforms to the specification outlined
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     * The waypoint can be restored with the obvious constructor
     *
     * @returns QJsonObject describing the waypoint
     */
    [[nodiscard]] QJsonObject toJSON() const;

    /*! \brief Serialization to GPX object
     *
     * This method serialises the waypoint as a GPX object and writes the object
     * to an XmlStream. If the waypoint is invalid, this method does nothing.
     *
     * @param stream XmlStream that the waypoint is written to, as a wpt element
     */
    void toGPX(QXmlStreamWriter& stream) const;

protected:
    QGeoCoordinate m_coordinate;
    QMap<QString, QVariant> m_properties;
};

/*! \brief Hash function for airspaces
 *
 * @param wp Waypoint
 *
 * @returns Hash value
 */
auto qHash(const GeoMaps::Waypoint& wp) -> size_t;

} // namespace GeoMaps

// Declare meta types
Q_DECLARE_METATYPE(GeoMaps::Waypoint)
