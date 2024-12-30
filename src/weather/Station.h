/***************************************************************************
 *   Copyright (C) 2020-2024 by Stefan Kebekus                             *
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

#include <QProperty>

#include "geomaps/Waypoint.h"
#include "weather/METAR.h"
#include "weather/TAF.h"

namespace GeoMaps {
class GeoMapProvider;
} // namespace GeoMaps


namespace Weather {

class WeatherDataProvider;


/*! \brief This class represents a weather station that issues METAR or TAF report
 *
 * This is a very simple value-base class that holds a waypoint and queries the WeatherDataProvider
 * for METAR and TAF for this waypoint.
 */

class Station {
    Q_GADGET
    QML_VALUE_TYPE(weatherStation)

public:
    //
    // Constructors and destructors
    //

    /*! \brief Standard constructor
     *
     * This standard constructor creates a WeatherStation with an invalid waypoint.
     */
    explicit Station();

    /*! \brief Constructor
     *
     * This standard constructor creates a WeatherStation for the given waypoint.
     *
     * @param wp Waypoint
     */
    explicit Station(const GeoMaps::Waypoint& wp);

    /*! \brief Copy constructor
     *
     *  @param other Other station
     */
    Station(const Station& other);

    /*! \brief Standard destructor */
    ~Station() = default;

    /*! \brief Copy assignment operator
     *
     *  @param other Other station
     */
    Station& operator=(const Station& other);


    //
    // Properties
    //

    /*! \brief Last METAR provided by this WeatherStation
     *
     * This property holds a pointer to the last METAR provided by this
     * WeatherStation, which can be a nullptr if no data is available.  The
     * METAR instance is owned by an instance of WeatherDataProvider, and can be
     * deleted or updated by the WeatherDataProvider anytime.
     */
    Q_PROPERTY(Weather::METAR metar READ metar BINDABLE bindableMetar)

    /*! \brief Last TAF provided by this WeatherStation
     *
     * This property holds a pointer to the last TAF provided by this
     * WeatherStation, which can be a nullptr if no data is available.  The TAF
     * instance is owned by an instance of WeatherDataProvider, and can be deleted or
     * updated by the WeatherDataProvider anytime.
     */
    Q_PROPERTY(Weather::TAF taf READ taf BINDABLE bindableTaf)

    /*! \brief Waypoint, as set in the constructor */
    Q_PROPERTY(GeoMaps::Waypoint waypoint READ waypoint CONSTANT)


    //
    // Getter Methods
    //

    /*! \brief Getter method for property of the same name
     *
     * @returns Property metar
     */
    [[nodiscard]] Weather::METAR metar() const {return m_metar.value();}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property metar
     */
    [[nodiscard]] QBindable<Weather::METAR> bindableMetar() const {return &m_metar;}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property taf
     */
    [[nodiscard]] Weather::TAF taf() const {return m_taf.value();}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property taf
     */
    [[nodiscard]] QBindable<Weather::TAF> bindableTaf() const {return &m_taf;}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property waypoint
     */
    [[nodiscard]] GeoMaps::Waypoint waypoint() const {return m_waypoint;}


private:
    // Property Waypoint
    GeoMaps::Waypoint m_waypoint;

    // Property METAR
    QProperty<Weather::METAR> m_metar;

    // Property TAF
    QProperty<Weather::TAF> m_taf;
};

} // namespace Weather

