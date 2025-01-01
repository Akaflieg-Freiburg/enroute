/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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


/*! \brief This class represents a weather Observer that issues METAR or TAF
 * report
 *
 * This is a very simple value-base class that holds a waypoint and queries the
 * WeatherDataProvider for METAR and TAF for this waypoint.
 */

class Observer : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    //
    // Constructors and destructors
    //

    /*! \brief Standard constructor
     *
     * This standard constructor creates a WeatherObserver with an invalid waypoint.
     */
    explicit Observer(QObject* parent=nullptr);

    /*! \brief Standard destructor */
    ~Observer() = default;


    //
    // Properties
    //

    /*! \brief Last METAR for this WeatherObserver
     *
     * This property holds the last METAR for this WeatherObserver, as reported
     * by WeatherDataProvider.
     */
    Q_PROPERTY(Weather::METAR metar READ metar BINDABLE bindableMetar)

    /*! \brief Last TAF for this WeatherObserver
     *
     * This property holds the last TAF for this WeatherObserver, as reported by
     * WeatherDataProvider.
     */
    Q_PROPERTY(Weather::TAF taf READ taf BINDABLE bindableTaf)

    /*! \brief Waypoint, as set in the constructor */
    Q_PROPERTY(GeoMaps::Waypoint waypoint READ waypoint WRITE setWaypoint BINDABLE bindableWaypoint REQUIRED)


    //
    // Getter Methods
    //

    /*! \brief Getter method for property of the same name
     *
     * @returns Property metar
     */
    [[nodiscard]] Weather::METAR metar() {return m_metar.value();}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property metar
     */
    [[nodiscard]] QBindable<Weather::METAR> bindableMetar() {return &m_metar;}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property taf
     */
    [[nodiscard]] Weather::TAF taf() {return m_taf.value();}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property taf
     */
    [[nodiscard]] QBindable<Weather::TAF> bindableTaf() {return &m_taf;}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property waypoint
     */
    [[nodiscard]] GeoMaps::Waypoint waypoint() const {return m_waypoint.value();}
    [[nodiscard]] QBindable<QString> bindableWaypoint() {return &m_waypoint;}
    void setWaypoint(const GeoMaps::Waypoint& wp) {m_waypoint = wp;}


private:
    Q_DISABLE_COPY_MOVE(Observer)

    // Property Waypoint
    QProperty<GeoMaps::Waypoint> m_waypoint;

    // Property METAR
    QProperty<Weather::METAR> m_metar;

    // Property TAF
    QProperty<Weather::TAF> m_taf;
};

} // namespace Weather

