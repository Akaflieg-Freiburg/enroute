/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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

#include "Meteorologist.h"

class GeoMapProvider;


/*! \brief This class represents a weather station that issues METAR or TAF report
 *
 * This is a very simple class that represents a weather station. Weather stations
 * are uniquely identified by their ICAO code. Depending on available data, they
 * hold pointers to the latest METAR and TAF reports.
 */
class Meteorologist::WeatherStation : public QObject {
    Q_OBJECT

    friend class Meteorologist;
    friend class Meteorologist::METAR;
    friend class Meteorologist::TAF;
public:
    /*! \brief Standard constructor
     *
     * This standard constructor creates an weather WeatherStation invalid.
     * Valid weather WeatherStations can only be created by instances of the
     * Meteorologist class.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit WeatherStation(QObject *parent = nullptr);

    // Standard destructor
    ~WeatherStation() = default;

    /*! Geographical coordinate of the WeatherStation reporting this METAR
     *
     * If the WeatherStation coordinate is unknown, the property contains an
     * invalid coordinate.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordiante
     */
    QGeoCoordinate coordinate() const
    {
        return _coordinate;
    }

    /*! \brief Extended name of the waypoint
     *
     * This property holds a string of the form "Karlsruhe (DVOR-DME)"
     */
    Q_PROPERTY(QString extendedName READ extendedName CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
    */
    QString extendedName() const
    {
        return _extendedName;
    }

    /*! \brief Check if a METAR weather report is known for this weather station
     *
     * This convenience property can be used to check if a METAR report is
     * available for the weather station.  The actual METAR report can be
     * accessed via the property metar.
     */
    Q_PROPERTY(bool hasMETAR READ hasMETAR CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property hasMetar
     */
    bool hasMETAR() const
    {
        return !_metar.isNull();
    }

    /*! \brief Check if a TAF weather forecast is known for this weather station
     *
     * This convenience property can be used to check if a TAF forecast is available
     * for the weather station.  The actual TAF can be accessed via the
     * property taf.
     */
    Q_PROPERTY(bool hasTAF READ hasTAF CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    bool hasTAF() const
    {
        return !_taf.isNull();
    }

    /*! \brief ICAO code of the weather station
     * 
     * This property holds the ICAO designator of the
     * aerodrome on which the weather station is located.
     */
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property ICAOCode
     */
    QString ICAOCode() const
    {
        return _ICAOCode;
    }

    /*! \brief Suggested icon for this weather station
     *
     * This property holds the name of an icon file in SVG format that best
     * describes the weather station.
     */
    Q_PROPERTY(QString icon READ icon CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property id
     */
    QString icon() const
    {
        return _icon;
    }

    /*! Indicates if the WeatherStation is valid */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    bool isValid() const
    {
        return (_ICAOCode.length() == 4);
    }

    /*! \brief Last METAR provided by this WeatherStation
     *
     * This property holds a pointer to the last METAR provided by this
     * WeatherStation, which can be a nullptr if no data is available.  The
     * METAR instance is owned by an instance of Meteorologist, and can be
     * deleted or updated by the Meteorologist anytime.
     */
    Q_PROPERTY(Meteorologist::METAR *metar READ metar NOTIFY metarChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property metar
     */
    Meteorologist::METAR *metar() const
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
    Q_PROPERTY(QString twoLineTitle READ twoLineTitle CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property twoLineTitle
     */
    QString twoLineTitle() const
    {
        return _twoLineTitle;
    }

    /*! \brief Last TAF provided by this WeatherStation
     * 
     * This property holds a pointer to the last TAF provided by this
     * WeatherStation, which can be a nullptr if no data is available.  The TAF
     * instance is owned by an instance of Meteorologist, and can be deleted or
     * updated by the Meteorologist anytime.
     */
    Q_PROPERTY(Meteorologist::TAF *taf READ taf NOTIFY tafChanged)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property taf
     */
    Meteorologist::TAF *taf() const
    {
        return _taf;
    }

    /*! \brief Description of the way from a given point to the weather station
     *
     * @param from Starting point of the way
     *
     * @param useMetric If true, then description uses metric units. Otherwise,
     * nautical units are used.
     *
     * @returns A string such as "DIST 65.2 NM • QUJ 276°".  If the way cannot
     * be described (e.g. because one of the coordinates is invalid or unknown),
     * then an empty string is returned.
     */
    Q_INVOKABLE QString wayTo(QGeoCoordinate from, bool useMetric) const;

signals:
    /* \brief Notifier signal */
    void metarChanged();

    /* \brief Notifier signal */
    void tafChanged();

private:
    Q_DISABLE_COPY_MOVE(WeatherStation)

    // This constructor is only meant to be called by instances of the
    // Meteorologist class
    explicit WeatherStation(const QString &id, GeoMapProvider *geoMapProvider, QObject *parent);

    // Sets the METAR message and deletes any existing METAR. This
    // WeatherStation will take ownership of the METAR. The signal
    // metarChanged() will be emitted if appropriate.
    void setMETAR(Meteorologist::METAR *metar);

    // Sets the TAF message, and deletes any existing TAF. This WeatherStation
    // will take ownership of the TAF. The signal tafChanged() will be emitted
    // if appropriate.
    void setTAF(Meteorologist::TAF *taf);

    // Converts the time into a human readable string
    static QString decodeTime(const QVariant &time);

    // Converts the wind data into a human readable string
    static QString decodeWind(const QVariant &windd, const QVariant &winds, const QVariant &windg = QVariant("0"));

    // Converts the visibility into a human readable string
    static QString decodeVis(const QVariant &vis);

    // Converts the temperature/dewpoint into a human readable string
    static QString decodeTemp(const QVariant &temp);

    // Converts the QNH (pressure) into a human readable string
    static QString decodeQnh(const QVariant &altim);

    // Converts the weather into a human readable string
    static QString decodeWx(const QVariant &wx);

    // Converts the clouds into a human readable string
    static QString decodeClouds(const QVariantList &clouds);


    // Coordinate of this weather station
    QGeoCoordinate _coordinate;

    // The weather station extended name
    QString _extendedName;

    // ICAO code of this weather station
    QString _ICAOCode;

    // Icon for this weather station
    QString _icon {"/icons/waypoints/WP.svg"};

    // METAR
    QPointer<Meteorologist::METAR> _metar;

    // TAF
    QPointer<Meteorologist::TAF> _taf;

    // Two-Line-Title
    QString _twoLineTitle;
};
