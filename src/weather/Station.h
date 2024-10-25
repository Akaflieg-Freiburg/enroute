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
 * hold pointers to the latest METAR and TAF reports.
 */
class Station : public QObject {
    Q_OBJECT

    friend WeatherDataProvider;
    friend METAR;
    friend TAF;
public:
    /*! \brief Standard constructor
     *
     * This standard constructor creates an weather WeatherStation invalid.
     * Valid weather WeatherStations can only be created by instances of the
     * WeatherDataProvider class.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit Station(QObject *parent = nullptr);

    // Standard destructor
    ~Station() override = default;

    /*! \brief Geographical coordinate of the WeatherStation reporting this METAR
     *
     * If the WeatherStation coordinate is unknown, the property contains an
     * invalid coordinate.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate NOTIFY coordinateChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordiante
     */
    [[nodiscard]] auto coordinate() const -> QGeoCoordinate
    {
        return _coordinate;
    }

    /*! \brief Extended name of the waypoint
     *
     * This property holds a string of the form "Karlsruhe (DVOR-DME)"
     */
    Q_PROPERTY(QString extendedName READ extendedName NOTIFY extendedNameChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
    */
    [[nodiscard]] auto extendedName() const -> QString
    {
        return _extendedName;
    }

    /*! \brief Check if a METAR weather report is known for this weather station
     *
     * This convenience property can be used to check if a METAR report is
     * available for the weather station.  The actual METAR report can be
     * accessed via the property metar.
     */
    Q_PROPERTY(bool hasMETAR READ hasMETAR NOTIFY hasMETARChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasMetar
     */
    [[nodiscard]] auto hasMETAR() const -> bool
    {
        return !_metar.isNull();
    }

    /*! \brief Check if a TAF weather forecast is known for this weather station
     *
     * This convenience property can be used to check if a TAF forecast is available
     * for the weather station.  The actual TAF can be accessed via the
     * property taf.
     */
    Q_PROPERTY(bool hasTAF READ hasTAF NOTIFY hasTAFChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    [[nodiscard]] auto hasTAF() const -> bool
    {
        return !_taf.isNull();
    }

    /*! \brief ICAO code of the weather station
     * 
     * This property holds the ICAO designator of the aerodrome on which the
     * weather station is located.
     */
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property ICAOCode
     */
    [[nodiscard]] auto ICAOCode() const -> QString
    {
        return m_ICAOCode;
    }

    /*! \brief Suggested icon for this weather station
     *
     * This property holds the name of an icon file in SVG format that best
     * describes the weather station.
     */
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property id
     */
    [[nodiscard]] auto icon() const -> QString
    {
        return _icon;
    }

    /*! \brief Indicates if the WeatherStation is valid */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    [[nodiscard]] auto isValid() const -> bool
    {
        return (m_ICAOCode.length() == 4);
    }

    /*! \brief Last METAR provided by this WeatherStation
     * 
     * This property holds a pointer to the last METAR provided by this
     * WeatherStation, which can be a nullptr if no data is available.  The
     * METAR instance is owned by an instance of WeatherDataProvider, and can be
     * deleted or updated by the WeatherDataProvider anytime.
     */
    Q_PROPERTY(Weather::METAR *metar READ metar NOTIFY metarChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property metar
     */
    [[nodiscard]] auto metar() const -> Weather::METAR *
    {
        return _metar;
    }

    /*! \brief Two-line description of the waypoint name
     *
     * This property holds a one-line or two-line description of the
     * waypoint. Depending on available data, this is a string of the form
     * "<strong>LFKA</strong><br><font size='2'>ALBERTVILLE</font>" or simply
     * "KIRCHZARTEN"
     *
     * @see threeLineTitle
     */
    Q_PROPERTY(QString twoLineTitle READ twoLineTitle NOTIFY twoLineTitleChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property twoLineTitle
     */
    [[nodiscard]] auto twoLineTitle() const -> QString
    {
        return _twoLineTitle;
    }

    /*! \brief Last TAF provided by this WeatherStation
     * 
     * This property holds a pointer to the last TAF provided by this
     * WeatherStation, which can be a nullptr if no data is available.  The TAF
     * instance is owned by an instance of WeatherDataProvider, and can be deleted or
     * updated by the WeatherDataProvider anytime.
     */
    Q_PROPERTY(Weather::TAF *taf READ taf NOTIFY tafChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property taf
     */
    [[nodiscard]] auto taf() const -> Weather::TAF *
    {
        return _taf;
    }

    /*! \brief Check if a Calculated Density Altitude is known for this weather station
     *
     * This convenience property can be used to check if a Calculated Density Altitude is available
     * for the weather station.  The actual Calculated Density Altitude can be accessed via the
     * property calculatedDensityAltitude.
     */
    Q_PROPERTY(bool hasCalculatedDensityAltitude READ hasCalculatedDensityAltitude NOTIFY calculatedDensityAltitudeChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasCalculatedDensityAltitude
     */
    [[nodiscard]] auto hasCalculatedDensityAltitude() const -> bool
    {
        return _calculatedDensityAltitude.has_value();
    }

    /*! \brief Calculated Density Altitude for this weather station as an integer
     *
     * This property holds the calculated density altitude for the weather station as an integer.
     */
    Q_PROPERTY(int calculatedDensityAltitude READ calculatedDensityAltitude NOTIFY calculatedDensityAltitudeChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property calculatedDensityAltitudeInt
     */
    [[nodiscard]] auto calculatedDensityAltitude() const -> int
    {
        return _calculatedDensityAltitude.value_or(0);
    }

signals:
    /* \brief Notifier signal */
    void coordinateChanged();

    /* \brief Notifier signal */
    void extendedNameChanged();

    /* \brief Notifier signal */
    void hasMETARChanged();

    /* \brief Notifier signal */
    void hasTAFChanged();

    /* \brief Notifier signal */
    void iconChanged();

    /* \brief Notifier signal */
    void metarChanged();

    /* \brief Notifier signal */
    void tafChanged();

    /* \brief Notifier signal */
    void twoLineTitleChanged();

    /* \brief Notifier signal */
    void calculatedDensityAltitudeChanged();

private slots:
    // This method attempts to find a waypoint matchting this weather station,
    // in order to learn additional data about the station. This method is
    // called automaticall whenever the GeoMapProvider has new data.
    void readDataFromWaypoint();

private:
    Q_DISABLE_COPY_MOVE(Station)

    // This constructor is only meant to be called by instances of the
    // WeatherDataProvider class
    explicit Station(QString id, GeoMaps::GeoMapProvider *geoMapProvider, QObject *parent);

    // If the metar is valid, not expired and newer than the existing metar,
    // this method sets the METAR message and deletes any existing METAR;
    // otherwise, the metar is deleted. In any case, this WeatherStation will
    // take ownership of the METAR. The signal metarChanged() will be emitted if
    // appropriate.
    void setMETAR(Weather::METAR *metar);

    // If the taf is valid, not expired and newer than the existing taf, this
    // method sets the TAF message and deletes any existing TAF; otherwise, the
    // taf is deleted. In any case, this WeatherStation will take ownership of
    // the TAF. The signal tafChanged() will be emitted if appropriate.
    void setTAF(Weather::TAF *taf);

    // calculates the density altitude
    void calculateDensityAltitude();

    // Coordinate of this weather station
    QGeoCoordinate _coordinate;

    // The weather station extended name
    QString _extendedName;

    // ICAO code of this weather station
    QString m_ICAOCode;

    // Icon for this weather station
    QString _icon {QStringLiteral("/icons/waypoints/WP.svg")};

    // METAR
    QPointer<Weather::METAR> _metar;

    // TAF
    QPointer<Weather::TAF> _taf;

    // Two-Line-Title
    QString _twoLineTitle;

    // calculated Density Altitude
    std::optional<int> _calculatedDensityAltitude;

    // Pointer to GeoMapProvider, used in order to find matching waypoints
    QPointer<GeoMaps::GeoMapProvider> _geoMapProvider;

    // Internal flag to indicate if data has been read from a matching waypoint
    // already
    bool hasWaypointData {false};
};

} // namespace Weather
