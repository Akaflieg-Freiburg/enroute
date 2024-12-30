/***************************************************************************
 *   Copyright (C) 2020-2022 by Stefan Kebekus                             *
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

#include <QPointer>
#include <QProperty>

#include "weather/METAR.h"
#include "weather/TAF.h"

namespace GeoMaps {
class GeoMapProvider;
} // namespace GeoMaps


namespace Weather {

class WeatherDataProvider;

/*! \brief This class represents a weather station that issues METAR or TAF report
 *
 * This is a very simple class that represents a weather station. Weather stations
 * are uniquely identified by their ICAO code. Depending on available data, they
 * hold latest METAR and TAF reports.
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
     * This standard constructor creates an weather WeatherStation invalid.
     * Valid weather WeatherStations can only be created by instances of the
     * WeatherDataProvider class.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Station();

    // This constructor is only meant to be called by instances of the
    // WeatherDataProvider class
    explicit Station(QString id, GeoMaps::GeoMapProvider *geoMapProvider);

    // Copy constructor
    Station(const Station&);

    // Standard destructor
    ~Station() = default;

    // Copy Assignment
    Station& operator=(const Station&);


    //
    // Properties
    //

    /*! \brief Geographical coordinate of the WeatherStation reporting this METAR
     *
     * If the WeatherStation coordinate is unknown, the property contains an
     * invalid coordinate.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate BINDABLE bindableCoordinate)

    /*! \brief Extended name of the waypoint
     *
     * This property holds a string of the form "Karlsruhe (DVOR-DME)"
     */
    Q_PROPERTY(QString extendedName READ extendedName BINDABLE bindableExtendedName)

    /*! \brief ICAO code of the weather station
     *
     * This property holds the ICAO designator of the aerodrome on which the
     * weather station is located.
     */
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)

    /*! \brief Suggested icon for this weather station
     *
     * This property holds the name of an icon file in SVG format that best
     * describes the weather station.
     */
    Q_PROPERTY(QString icon READ icon BINDABLE bindableIcon)

    /*! \brief Indicates if the WeatherStation is valid */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Last METAR provided by this WeatherStation
     *
     * This property holds a pointer to the last METAR provided by this
     * WeatherStation, which can be a nullptr if no data is available.  The
     * METAR instance is owned by an instance of WeatherDataProvider, and can be
     * deleted or updated by the WeatherDataProvider anytime.
     */
    Q_PROPERTY(Weather::METAR metar READ metar BINDABLE bindableMetar)

    /*! \brief Two-line description of the waypoint name
     *
     * This property holds a one-line or two-line description of the
     * waypoint. Depending on available data, this is a string of the form
     * "<strong>LFKA</strong><br><font size='2'>ALBERTVILLE</font>" or simply
     * "KIRCHZARTEN"
     *
     * @see threeLineTitle
     */
    Q_PROPERTY(QString twoLineTitle READ twoLineTitle BINDABLE bindableTwoLineTitle)

    /*! \brief Last TAF provided by this WeatherStation
     *
     * This property holds a pointer to the last TAF provided by this
     * WeatherStation, which can be a nullptr if no data is available.  The TAF
     * instance is owned by an instance of WeatherDataProvider, and can be deleted or
     * updated by the WeatherDataProvider anytime.
     */
    Q_PROPERTY(Weather::TAF taf READ taf BINDABLE bindableTaf)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordiante
     */
    [[nodiscard]] QGeoCoordinate coordinate() const {return m_coordinate.value();}

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordiante
     */
    [[nodiscard]] QBindable<QGeoCoordinate> bindableCoordinate() const {return &m_coordinate;}

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
    */
    [[nodiscard]] QString extendedName() const {return m_extendedName.value();}

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
    */
    [[nodiscard]] QBindable<QString> bindableExtendedName() const {return &m_extendedName;}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property ICAOCode
     */
    [[nodiscard]] QString ICAOCode() const
    {
        return m_ICAOCode;
    }

    /*! \brief Getter method for property of the same name
     *
     * @returns Property id
     */
    [[nodiscard]] QString icon() const {return m_icon.value();}

    /*! \brief Getter method for property of the same name
     *
     * @returns Property id
     */
    [[nodiscard]] QBindable<QString> bindableIcon() const {return &m_icon;}

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    [[nodiscard]] bool isValid() const
    {
        return (m_ICAOCode.length() == 4);
    }

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

    /*! \brief Getter function for property with the same name
     *
     * @returns Property twoLineTitle
     */
    [[nodiscard]] QString twoLineTitle() const {return m_twoLineTitle.value();}

    /*! \brief Getter function for property with the same name
     *
     * @returns Property twoLineTitle
     */
    [[nodiscard]] QBindable<QString> bindableTwoLineTitle() const {return &m_twoLineTitle;}

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


private slots:
    // This method attempts to find a waypoint matchting this weather station,
    // in order to learn additional data about the station. This method is
    // called automaticall whenever the GeoMapProvider has new data.
    void readDataFromWaypoint();

private:
    // Coordinate of this weather station
    QProperty<QGeoCoordinate> m_coordinate;

    // The weather station extended name
    QProperty<QString> m_extendedName;

    // ICAO code of this weather station
    QString m_ICAOCode;

    // Icon for this weather station
    QProperty<QString> m_icon {QStringLiteral("/icons/waypoints/WP.svg")};

    // METAR
    QProperty<Weather::METAR> m_metar;

    // TAF
    QProperty<Weather::TAF> m_taf;

    // Two-Line-Title
    QProperty<QString> m_twoLineTitle;

    // Pointer to GeoMapProvider, used in order to find matching waypoints
    QPointer<GeoMaps::GeoMapProvider> m_geoMapProvider;
};

} // namespace Weather
