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


namespace Weather {

class WeatherDataProvider;


/*! \brief Holds and updates METAR and TAF for a given waypoint */

class Observer : public QObject {
    Q_OBJECT
    QML_ELEMENT

public:
    //
    // Constructors and destructors
    //

    /*! \brief Standard constructor
     *
     * This standard constructor creates an observer with an invalid waypoint.
     *
     * @param parent Standard QObject parent
     */
    explicit Observer(QObject* parent=nullptr);

    /*! \brief Standard destructor */
    ~Observer() = default;


    //
    // Properties
    //

    /*! \brief Current METAR
     *
     * This property holds the last METAR for the waypoint, as reported by
     * WeatherDataProvider. If no METAR is known, the property will be invalid.
     */
    Q_PROPERTY(Weather::METAR metar READ metar BINDABLE bindableMetar)

    /*! \brief Current TAF
     *
     * This property holds the last TAF for the waypoint, as reported by
     * WeatherDataProvider. If no TAF is known, the property will be invalid.
     */
    Q_PROPERTY(Weather::TAF taf READ taf BINDABLE bindableTaf)

    /*! \brief Waypoint for which METAR/TAF data is retrieved
     *
     *  The class uses the waypoint's ICAOcode to retrieve METAR/TAF data from
     *  the global WeatherDataProvider instance.
     */
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


    /*! \brief Getter method for property of the same name
     *
     * @returns Property waypoint
     */
    [[nodiscard]] QBindable<QString> bindableWaypoint() {return &m_waypoint;}


    //
    // Setter Methods
    //

    /*! \brief Setter method for property of the same name
     *
     * @param wp Property waypoint
     */
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
