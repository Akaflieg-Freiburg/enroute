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

#include <QDataStream>
#include <QGeoCoordinate>
#include <QXmlStreamReader>

#include "navigation/Aircraft.h"
#include "units/Distance.h"
#include "units/Pressure.h"
#include "units/Speed.h"
#include "units/Temperature.h"
#include "weather/Decoder.h"

namespace Weather {

class WeatherDataProvider;

/*! \brief METAR report
 *
 * This class contains the data of a METAR report and provides a few
 * methods to access the data. Instances of this class are provided by the
 * WeatherDataProvider class; there is no way to construct valid instances yourself.
 */

class METAR : public QObject {
    Q_OBJECT

    friend WeatherDataProvider;
    friend QDataStream& operator<<(QDataStream& stream, const METAR& metar);

public:
    /*! \brief Flight category
     *
     * Flight category, as defined in
     * https://www.aviationweather.gov/metar/help?page=plot#fltcat
     */
    enum FlightCategory : quint8
    {
        VFR,     /*!< Visual Flight Rules */
        MVFR,    /*!< Marginal Visual Flight Rules */
        IFR,     /*!< Instrument Flight Rules */
        LIFR,    /*!< Low Instrument Flight Rules */
        unknown  /*!< Unknown conditions */
    };
    Q_ENUM(FlightCategory)



    //
    // Constructors and destructors
    //

    /*! \brief Default constructor
     *
     * This constructor creates an invalid METAR instance.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit METAR(QObject *parent = nullptr);

    // Standard destructor
    ~METAR() override = default;


    //
    // Properties
    //

    /*! \brief Geographical coordinate of the station reporting this METAR
     *
     * If the station coordinate is unknown, the property contains an invalid
     * coordinate. Typically, the coordinate will contain the elevation of
     * the station.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Expiration time and date
     *
     * A METAR message is supposed to expire 1.5 hours after observation time,
     * unless raw text contains "NOSIG", then it is 3 hours.
     */
    Q_PROPERTY(QDateTime expiration READ expiration CONSTANT)

    /*! \brief Suggested color describing the flight category for this METAR
     *
     * The suggested colors are the following
     *
     * - "green" for VFR
     *
     * - "yellow" for MVFR
     *
     * - "red" for "IFR" and "LIFR"
     *
     * - "transparant" for "unknown"
     */
    Q_PROPERTY(QString flightCategoryColor READ flightCategoryColor CONSTANT)

    /*! \brief Flight category for this METAR */
    Q_PROPERTY(FlightCategory flightCategory READ flightCategory CONSTANT)

    /*! \brief ICAO code of the station reporting this METAR
     *
     * This is a string such as "LSZB" for Bern/Belp airport. If the station
     * code is unknown, the property contains an empty string.
     */
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)

    /*! \brief Indicates if the class represents a valid METAR report */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Observation time of this METAR */
    Q_PROPERTY(QDateTime observationTime READ observationTime CONSTANT)

    /*! \brief QNH value in this METAR
     *
     * The QNH property is set to NaN if no QNH is known. Otherwise, the values
     * is guaranteed to lie in the interval [800 … 1200]
     */
    Q_PROPERTY(Units::Pressure QNH READ QNH CONSTANT)

    /*! \brief Raw METAR text
     *
     * This is a string such as "METAR EICK 092100Z 23007KT 9999 FEW038 BKN180
     * 11/08 Q1019 NOSIG=".
     */
    Q_PROPERTY(QString rawText READ rawText CONSTANT)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordiante
     */
    [[nodiscard]] QGeoCoordinate coordinate() const
    {
        return m_location;
    }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property expiration
     */
    [[nodiscard]] QDateTime expiration() const;

    /*! \brief Getter function for property with the same name
     *
     * @returns Property flightCategoryColor
     */
    [[nodiscard]] QString flightCategoryColor() const;

    /*! \brief Getter function for property with the same name
     *
     * @returns Property flightCategory
     */
    [[nodiscard]] FlightCategory flightCategory() const
    {
        return m_flightCategory;
    }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property ICAOCode
     */
    [[nodiscard]] QString ICAOCode() const
    {
        return m_ICAOCode;
    }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    [[nodiscard]] bool isValid() const;

    /*! \brief Getter function for property with the same name
     *
     * @returns Property observationTime
     */
    [[nodiscard]] QDateTime observationTime() const
    {
        return m_observationTime;
    }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property qnh
     */
    [[nodiscard]] Units::Pressure QNH() const
    {
        return m_qnh;
    }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
     */
    [[nodiscard]] QString rawText() const
    {
        return m_raw_text;
    }


    //
    // Methods
    //

#warning
    [[nodiscard]] Q_INVOKABLE QString decodedText() {return m_decoder.decodedText();}

    /*! \brief Derived data, such as density height
     *
     * @param aircraft Current aircraft, used to determine appropriate units
     *
     * @param showPerformanceWarning If true, then show warning if density altitude
     * severely affect aircraft performance.
     *
     * @param explainPerformanceWarning If true, add text to explain performance
     * degrade
     *
     * @returns Human-readable, translated rich text. The text contains two links,
     * to 'hidePerformanceWarning' and 'hideExplanation'. When clicked, the
     * user-facing text should be replaced, if the appropriate parameter set to 'false'.
     *
     */
    [[nodiscard]] Q_INVOKABLE QString derivedData(const Navigation::Aircraft& aircraft, bool showPerformanceWarning, bool explainPerformanceWarning) const;

    /*! \brief One-line summary of the METAR
     *
     * @param aircraft Current aircraft, used to determine appropriate units
     *
     * @param currentTime Current time, used to describe time difference
     *
     * @returns A translated, human-readable string of the form "METAR 14min ago:
     * marginal VMC • wind at 15kt • rain"
     */
    [[nodiscard]] Q_INVOKABLE QString summary(const Navigation::Aircraft& aircraft, const QDateTime& currentTime) const;

protected:
    // This constructor reads a XML stream, as provided by the Aviation Weather
    // Center's Text Data Server, https://www.aviationweather.gov/dataserver
    explicit METAR(QXmlStreamReader& xml, QObject* parent = nullptr);

    // This constructor reads a serialized METAR from a QDataStream
    explicit METAR(QDataStream& inputStream, QObject* parent = nullptr);

private:
    Q_DISABLE_COPY_MOVE(METAR)

    // Decoded METAR text
    QString _decoded;

    // Flight category, as returned by the Aviation Weather Center
    FlightCategory m_flightCategory {unknown};

    // Gust speed, as returned by the Aviation Weather Center
    Units::Speed m_gust;

    // Station ID, as returned by the Aviation Weather Center
    QString m_ICAOCode;

    // Station coordinate, as returned by the Aviation Weather Center
    QGeoCoordinate m_location;

    // Observation time, as returned by the Aviation Weather Center
    QDateTime m_observationTime;

    // QNH in hPa, as returned by the Aviation Weather Center
    Units::Pressure m_qnh;

    // Raw METAR text, as returned by the Aviation Weather Center
    QString m_raw_text;

    // Wind speed, as returned by the Aviation Weather Center
    Units::Speed m_wind;

    // Temperature, as returned by the Aviation Weather Center
    Units::Temperature m_temperature;

    // Dewpoint, as returned by the Aviation Weather Center
    Units::Temperature m_dewpoint;

    // Density altitude, derived data
    Units::Distance m_densityAltitude;

#warning
    Weather::Decoder m_decoder;
};

/*! \brief Serialization
 *
 *  There is no checks for errors of any kind.
 */
QDataStream& operator<<(QDataStream& stream, const METAR& metar);

} // namespace Weather
