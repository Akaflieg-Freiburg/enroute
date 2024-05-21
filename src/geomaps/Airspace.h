/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include <QGeoPolygon>
#include <QJsonObject>

#include "units/Distance.h"

namespace GeoMaps {

/*! \brief A very simple class that describes an airspace */

class Airspace {
    Q_GADGET

    /*! \brief Comparison */
    friend auto operator==(const GeoMaps::Airspace&, const GeoMaps::Airspace&) -> bool;

public:
    /*! \brief Constructs an invalid airspace */
    Airspace() = default;

    /*! \brief Constructs an airspace from a GeoJSON object
     *
     * This method constructs an Airpace from a GeoJSON description. The GeoJSON
     * file specification is found
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     *
     * @param geoJSONObject GeoJSON Object that describes the airspace.
     */
    explicit Airspace(const QJsonObject &geoJSONObject);

    /*! \brief Estimates the lower limit of the airspace above MSL
     *
     * This method gives a rought estimate for the lower limit of the airspace.
     * The result is not reliable enough for aviation purposes but
     * can be used to sort the airspaces in the GUI.
     *
     * @returns Estimated lower bound of the airspace, above main sea
     * level
     */
    [[nodiscard]] auto estimatedLowerBoundMSL() const -> Units::Distance;
    [[nodiscard]] auto estimatedLowerBoundMSL(int elevation) const -> Units::Distance;
    /*! \brief Estimates the lower limit of the airspace above MSL
     *
     * This method gives a rought estimate for the lower limit of the airspace.
     * The result is not reliable enough for aviation purposes but
     * can be used to sort the airspaces in the GUI.
     *
     * @returns Estimated lower bound of the airspace, above main sea
     * level
     */
    [[nodiscard]] auto estimatedUpperBoundMSL() const -> Units::Distance;
    [[nodiscard]] auto estimatedUpperBoundMSL(int elevation) const -> Units::Distance;

    /*! \brief Validity */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    [[nodiscard]] auto isValid() const -> bool { return !m_polygon.isEmpty(); }

    /*! \brief Lower limit of the airspace
     *
     * A string that describes the lower bound of the airspace
     *
     * @see upperBound
     */
    Q_PROPERTY(QString lowerBound READ lowerBound CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property lowerBound
     */
    [[nodiscard]] auto lowerBound() const -> QString { return m_lowerBound; }

    /*! \brief Lower limit of the airspace
     *
     * A string that describes the lower bound of the airspace in metric terms
     */
    Q_PROPERTY(QString lowerBoundMetric READ lowerBoundMetric CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property lowerBoundMetric
     */
    [[nodiscard]] auto lowerBoundMetric() const -> QString { return makeMetric(m_lowerBound); }

    /* \brief Name of the airspace, such as "ED-R 31" */
    Q_PROPERTY(QString name READ name CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property name
     */
    [[nodiscard]] auto name() const -> QString { return m_name; }

    /*! \brief QGeoPolygon that describes the lateral limits of the airspace */
    Q_PROPERTY(QGeoPolygon polygon READ polygon CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property polygon
     */
    [[nodiscard]] auto polygon() const -> QGeoPolygon { return m_polygon; }

    /* \brief Category of the airspace
     *
     * A string with the category of the airspace, as described in the GeoJSON
     * file specification
     * [here](https://github.com/Akaflieg-Freiburg/enrouteServer/wiki/GeoJSON-files-used-in-enroute-flight-navigation).
     */
    Q_PROPERTY(QString CAT READ CAT CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property CAT
     */
    [[nodiscard]] auto CAT() const -> QString { return m_CAT; }

    /*! \brief Upper limit of the airspace
     *
     * A string that describes the upper bound of the airspace
     */
    Q_PROPERTY(QString upperBound READ upperBound CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property upperBound
     */
    [[nodiscard]] auto upperBound() const -> QString { return m_upperBound; }

    /*! \brief Upper limit of the airspace
     *
     * A string that describes the upper bound of the airspace in metric terms
     */
    Q_PROPERTY(QString upperBoundMetric READ upperBoundMetric CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property upperBoundMetric
     */
    [[nodiscard]] auto upperBoundMetric() const -> QString { return makeMetric(m_upperBound); }

private:
    // Transforms a height string such as "4500", "1500 GND" or "FL 130" into a string that describes the height
    // in meters. If the height string cannot be parsed, returns the original string
    [[nodiscard]] static auto makeMetric(const QString& standard) -> QString;

    QString m_name{};
    QString m_CAT{};
    QString m_upperBound{};
    QString m_lowerBound{};
    QGeoPolygon m_polygon{};
};

/*! \brief Comparison */
auto operator==(const GeoMaps::Airspace&, const GeoMaps::Airspace&) -> bool;

/*! \brief Hash function for airspaces
 *
 * @param as Airspace
 *
 * @returns Hash value
 */
auto qHash(const GeoMaps::Airspace& as) -> size_t;

} // namespace GeoMaps


// Declare meta types
Q_DECLARE_METATYPE(GeoMaps::Airspace)
