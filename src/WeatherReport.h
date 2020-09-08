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

#include <QGeoCoordinate>
#include <QObject>
#include <QPointer>
#include <QVariant>

/*! \brief WeatherReport, a weather report containing a METAR and/or a TAF
 *
 * This class holds the weather report provided by a given station. The report
 * consists of a METAR (observations) and/or a TAF (forecast), and are generated
 * from the maps provided by the Meteorologist.
 * This class provides the raw data and also formats them into a human readable
 * language.
 */
class WeatherReport : public QObject {
    Q_OBJECT

public:
    class METAR;
    class TAF;

    /*! \brief Standard constructor
    *
    * @param id The station ID
    * 
    * @param metar The METAR data
    * 
    * @param taf The TAF data
    * 
    * @param parent The standard QObject parent pointer
    */
    explicit WeatherReport(const QString &id,
                           WeatherReport::METAR *metar,
                           WeatherReport::TAF *taf,
                           QObject *parent = nullptr);

    // Standard destructor
    ~WeatherReport() = default;

    /*! \brief The station ID
     *
     * The ID of the weather station is usually the ICAO designator of the
     * aerodrome on which the station is located.
     */
    Q_PROPERTY(QString id READ id CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property id
     */
    QString id() const { return _id; }

    /*! \brief The FAA flight category
     *
     * The FAA (U.S.) flight categories are defined by the following visibility
     * (in statute miles) and ceiling (in feet):
     * - VFR (visual flight rules): vis > 5, ceil > 3000
     * - MVFR (marginal visual flight rules): 3 < vis < 5, 1000 < ceil < 3000
     * - IFR (instrument flight rules): 1 < vis < 3, 500 < ceil < 1000
     * - LIFR (low instrument flight rules): vis < 1, ceil < 500
     */
    Q_PROPERTY(QString cat READ cat CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property cat
     */
    QString cat() const { return _cat; }

    /*! \brief The METAR data
     *
     * A list of formated strings constituting the METAR data (observations).
     * If the station has no METAR, the list only contains "NONE". The first 4
     * letters of each string indicate which field the data belong to:
     * - RAW: the raw metar report
     * - TIME: the time of emission
     * - WIND: the wind direction and speed
     * - VIS: the horizontal visibility
     * - WX: the significant weather
     * - CLDS: the clouds
     * - TEMP: the temperature
     * - DEWP: the dewpoint
     * - QNH: the pressure at sea-level
     */
    Q_PROPERTY(WeatherReport::METAR *metar READ metar CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property metar
     */
    WeatherReport::METAR *metar() const { return _metar; }

    /*! \brief The METAR data
     *
     * A list of formated strings constituting the METAR data (observations).
     * If the station has no METAR, the list only contains "NONE". The first 4
     * letters of each string indicate which field the data belong to:
     * - RAW: the raw metar report
     * - TIME: the time of emission
     * - WIND: the wind direction and speed
     * - VIS: the horizontal visibility
     * - WX: the significant weather
     * - CLDS: the clouds
     * - TEMP: the temperature
     * - DEWP: the dewpoint
     * - QNH: the pressure at sea-level
     */
    Q_PROPERTY(QList<QString> metarStrings READ metarStrings CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property metarStrings
     */
    QList<QString> metarStrings() const;

    /*! \brief The TAF data
     *
     * A list of formated strings constituting the TAF data (forecast).
     * If the station has no TAF, the list only contains "NONE". The first 4
     * letters of each string indicate which field the data belong to:
     * - RAW: the raw metar report
     * - TIME: the time of emission
     * - FROM: the time at which the forecast starts
     * - TO: the time at which the forecast ends
     * - FCST: the forecast(s)
     * Each forecast contains the start and end times, its probability, the wind
     * speed and direction, the visibility, the significant weather and the
     * clouds.
     */
    Q_PROPERTY(WeatherReport::TAF *taf READ taf CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property taf
     */
    WeatherReport::TAF *taf() const { return _taf; }

    /*! \brief The TAF data
     *
     * A list of formated strings constituting the TAF data (forecast).
     * If the station has no TAF, the list only contains "NONE". The first 4
     * letters of each string indicate which field the data belong to:
     * - RAW: the raw metar report
     * - TIME: the time of emission
     * - FROM: the time at which the forecast starts
     * - TO: the time at which the forecast ends
     * - FCST: the forecast(s)
     * Each forecast contains the start and end times, its probability, the wind
     * speed and direction, the visibility, the significant weather and the
     * clouds.
     */
    Q_PROPERTY(QList<QString> tafStrings READ tafStrings CONSTANT)

    /*! \brief Getter method for property of the same name
     *
     * @returns Property taf
     */
    QList<QString> tafStrings() const;

    Q_PROPERTY(QGeoCoordinate location READ location CONSTANT);
    QGeoCoordinate location() const;

private:
    Q_DISABLE_COPY_MOVE(WeatherReport)

    /*! \brief The station ID */
    QString _id;

    /*! \brief The FAA flight category */
    QString _cat;

    /*! \brief METAR */
    QPointer<WeatherReport::METAR> _metar;

    /*! \brief TAF */
    QPointer<WeatherReport::TAF> _taf;

    /*! \brief Converts the time into a human readable string */
    static QString decodeTime(const QVariant &time);

    /*! \brief Converts the wind data into a human readable string */
    static QString decodeWind(const QVariant &windd, const QVariant &winds, const QVariant &windg = QVariant("0"));

    /*! \brief Converts the visibility into a human readable string */
    static QString decodeVis(const QVariant &vis);

    /*! \brief Converts the temperature/dewpoint into a human readable string */
    static QString decodeTemp(const QVariant &temp);

    /*! \brief Converts the QNH (pressure) into a human readable string */
    static QString decodeQnh(const QVariant &altim);

    /*! \brief Converts the weather into a human readable string */
    static QString decodeWx(const QVariant &wx);

    /*! \brief Converts the clouds into a human readable string */
    static QString decodeClouds(const QVariantList &clouds);
};

#include "WeatherReport_METAR.h"
#include "WeatherReport_TAF.h"
