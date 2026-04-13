/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include "units/Angle.h"


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
    friend size_t qHash(const GeoMaps::Waypoint& waypoint);

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
     * - name is set to the "Waypoint" or to the name given in the argument
     * - type is set to "WP"
     *
     * @param coordinate Geographical position of the waypoint
     * @param name Name of the waypoint
     */
    Waypoint(const QGeoCoordinate& coordinate, const QString& name = {});

    /*! \brief Constructs a waypoint from a GeoJSON object
     *
     * This method constructs a Waypoint from a GeoJSON description.  The
     * GeoJSON file specification is found
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     *
     * @param geoJSONObject GeoJSON Object that describes the waypoint
     */
    explicit Waypoint(const QJsonObject& geoJSONObject);

    /*! \brief Constructs a waypoint from fix, bearing and distance
     *
     * This constructor calculates a waypoint position based on a reference
     * navigation aid, a magnetic bearing from that navaid, and a distance.
     * The magnetic variation at the navaid is taken into account.
     *
     * - category is set to "WP"
     * - name is set to "Waypoint"
     * - type is set to "WP"
     * - representation is set accordingly
     *
     * If the navaid is invalid or doesn't have valid magnetic variation,
     * the resulting waypoint will be invalid.
     *
     * @param navaid Reference navigation aid waypoint
     * @param magneticBearing Magnetic bearing from the navaid (in degrees)
     * @param distanceNM Distance from the navaid in nautical miles
     */
    Waypoint(const GeoMaps::Waypoint& navaid, const Units::Angle& magneticBearing, double distanceNM);


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

    /*! \brief Magnetic variation at the waypoint
     *
     * This property holds the magnetic variation at the waypoint as a number.
     */
    Q_PROPERTY(Units::Angle variation READ variation CONSTANT)

    /*! \brief Custom representation of the waypoint
     *
     * This property holds a custom representation of the waypoint, e.g.
     * by fix/bearing/distance.
     * The custom representation is used when exporting a VFR flight plan.
     *
     * The property is reset, whenever the underlying waypoint is changed
     * (i.e. the coordinates are changed).
     */
    Q_PROPERTY(QString representation READ representation WRITE setRepresentation)


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
        if (!ICAOCode().isEmpty()) {
            return ICAOCode();
        }

        // If name is generic "Waypoint", use a more concise representation instead:
        if (name() == u"Waypoint") {
            return representation();
        }

        return name();
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

    /*! \brief Get variation
     *
     * @returns The variation, or NaN if not available.
     */
    [[nodiscard]] auto variation() const -> Units::Angle;

    /*! \brief Representation for writing this in a VFR flight plan.
     *
     * A custom representation may be set using #setRepresentation().
     * If no custom representation is set, the default geocoordinate
     * representation is returned.
     *
     * @returns the applicable representation.
     */
    [[nodiscard]] auto representation() const -> QString
    {
        if (!m_properties.contains(QStringLiteral("REP")) ||
                m_properties.value(QStringLiteral("REP")).toString().isEmpty()) {
            return coordinateNotation();
        }

        return m_properties.value(QStringLiteral("REP")).toString();
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
        m_properties.remove("REP");
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

    /*! \brief Set custom representation for use in VFR flight plans.
     *
     * @param rep The new representation.
     */
    void setRepresentation(const QString rep)
    {
        m_properties.insert(QStringLiteral("REP"), QString(rep));
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
    [[nodiscard]] Q_INVOKABLE bool operator==(const GeoMaps::Waypoint& other) const = default;

    /*! \brief Comparison
     *
     *  @param other Waypoint to compare with
     *
     *  @returns Result of comparison
     */
    [[nodiscard]] Q_INVOKABLE bool operator!=(const GeoMaps::Waypoint& other) const = default;

    /*! \brief Deep copy
     *
     *  This method exists for the benefit of QML, where deep copies are hard to produce.
     *
     * @returns Copy of the present waypoint.
     */
    [[nodiscard]] Q_INVOKABLE GeoMaps::Waypoint copy() const
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
    [[nodiscard]] Q_INVOKABLE bool isNear(const GeoMaps::Waypoint& other) const;

    /*! \brief Generate radial notation string from a navaid
     *
     * This method generates a radial position notation in the format used in
     * flight plans (e.g., "LNO070012" = LNO VOR, radial 070°, distance 12 nm).
     *
     * @param navaid Navigation aid waypoint to use as reference
     *
     * @returns Radial notation string, or empty string if this waypoint or the
     * navaid is invalid, or if navaid is not a NAV type
     */
    [[nodiscard]] Q_INVOKABLE QString radialNotation(const GeoMaps::Waypoint& navaid) const;

    /*! \brief Generate VFR flight plan format
     *
     * Convert coordinates to VFR flight plan format (degrees and minutes)
     * Format: "4620N07805W" (DDMMNDDDMME) - 11 characters as per AIP.
     *
     * For more information, see (2) Significant point in
     * https://www.faa.gov/air_traffic/publications/atpubs/fss/AppendixA.htm
     *
     * @returns coordinate string in VFR flight plan format.
     */
    [[nodiscard]] Q_INVOKABLE QString coordinateNotation() const;

    /*! \brief Compute available representations for this waypoint
     *
     * This method computes all possible representations for this waypoint's
     * coordinate, including coordinate notation and radial notations from nearby
     * navigation aids.
     *
     * @returns A list of representation strings, with coordinate notation first,
     *          followed by radial notations from nearby navaids
     */
    [[nodiscard]] Q_INVOKABLE QStringList availableRepresentations() const;

    /*! \brief Find index of representation in available representations
     *
     * As the list of representations is always sorted (by distance to reference
     * station), we can refer to a representation by its index in that list.
     *
     * This method checks whether a given representation string is valid for
     * this waypoint's coordinate and returns its index.
     *
     * @param representation The representation string to find
     *
     * @returns Index of the representation (0 or higher if found), or -1 if not found
     */
    [[nodiscard]] Q_INVOKABLE int representationIndex(const QString& representation) const;

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
 * @param waypoint Waypoint
 *
 * @returns Hash value
 */
auto qHash(const GeoMaps::Waypoint& waypoint) -> size_t;

} // namespace GeoMaps

// Declare meta types
Q_DECLARE_METATYPE(GeoMaps::Waypoint)
