/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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
#include <QXmlStreamReader>

#include "AviationUnits.h"
#include "weather/Decoder.h"


namespace Weather {

class DownloadManager;

/*! \brief METAR report
 *
 * This class contains the data of a METAR or SPECI report and provided a few
 * methods to access the data. Instances of this class are provided by the
 * DownloadManager class; there is no way to construct valid instances yourself.
 */

class METAR : public Decoder {
    Q_OBJECT

    friend DownloadManager;

public:
    /*! \brief Default constructor
     *
     * This constructor creates an invalid METAR instance.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit METAR(QObject *parent = nullptr);

    // Standard destructor
    ~METAR() = default;

    /*! \brief Flight category
     *
     * Flight category, as defined in
     * https://www.aviationweather.gov/metar/help?page=plot#fltcat
     */
    enum FlightCategory
    {
        VFR,     /*!< Visual Flight Rules */
        MVFR,    /*!< Marginal Visual Flight Rules */
        IFR,     /*!< Instrument Flight Rules */
        LIFR,    /*!< Low Instrument Flight Rules */
        unknown  /*!< Unknown conditions */
    };
    Q_ENUM(FlightCategory)

    /*! \brief Geographical coordinate of the station reporting this METAR
     *
     * If the station coordinate is unknown, the property contains an invalid
     * coordinate.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property coordiante
     */
    QGeoCoordinate coordinate() const
    {
        return _location;
    }

    /*! \brief Expiration time and date
     *
     * A METAR message is supposed to expire 1.5 hours after observation time,
     * unless raw text contains "NOSIG", then it is 3 hours.
     */
    Q_PROPERTY(QDateTime expiration READ expiration CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property expiration
     */
    QDateTime expiration() const;

    /*! \brief Suggested color describing the flight category for this METAR
     *
     * The suggested colors are  the following
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

    /*! \brief Getter function for property with the same name
     *
     * @returns Property color
     */
    QString flightCategoryColor() const;

    /*! \brief Flight category for this METAR */
    Q_PROPERTY(FlightCategory flightCategory READ flightCategory CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property flightCategory
     */
    FlightCategory flightCategory() const
    {
        return _flightCategory;
    }

    /*! \brief ICAO code of the station reporting this METAR
     *
     * This is a string such as "LSZB" for Bern/Belp airport. If the station
     * code is unknown, the property contains an empty string.
     */
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property ICAOCode
     */
    QString ICAOCode() const
    {
        return _ICAOCode;
    }

    /*! \brief Convenience method to check if this METAR is already expired
     *
     * @returns true if an expiration date/time is known and if the current time
     * is larger than the expiration
     */
    Q_INVOKABLE bool isExpired() const;

    /*! \brief Indicates if the class represents a valid METAR report */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    bool isValid() const;

    /*! \brief Observation time of this METAR */
    Q_PROPERTY(QDateTime observationTime READ observationTime CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property observationTime
     */
    QDateTime observationTime() const
    {
        return _observationTime;
    }

    /*! \brief QNH value in this METAR, in hPa
     *
     * The QNH property is set to zero if no QNH is known. Otherwise, the values
     * is guaranteed to lie in the interval [800 … 1200]
     */
    Q_PROPERTY(quint16 QNH READ QNH CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property qnh
     */
    quint16 QNH() const
    {
        return _qnh;
    }

    /*! \brief Raw METAR text
     *
     * This is a string such as "METAR EICK 092100Z 23007KT 9999 FEW038 BKN180
     * 11/08 Q1019 NOSIG=".
     */
    Q_PROPERTY(QString rawText READ rawText CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
     */
    QString rawText() const
    {
        return _raw_text;
    }

    /*! \brief Observation time, relative to now
     *
     * This is a translated, human-readable string such as "1h and 43min ago"
     * that describes the observation time.
     */
    Q_PROPERTY(QString relativeObservationTime READ relativeObservationTime NOTIFY relativeObservationTimeChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property relativeObservationTime
     */
    QString relativeObservationTime() const;

    /*! \brief One-line summary of the METAR
     *
     * This is a translated, human-readable string of the form "METAR 14min ago:
     * marginal VMC • wind at 15kt • rain"
     */
    Q_PROPERTY(QString summary READ summary NOTIFY summaryChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property summary
     */
    QString summary() const;


signals:
    /*! \brief Notifier signal */
    void summaryChanged();

    /*! \brief Notifier signal */
    void relativeObservationTimeChanged();

protected:
    // This constructor reads a XML stream, as provided by the Aviation Weather
    // Center's Text Data Server, https://www.aviationweather.gov/dataserver
    explicit METAR(QXmlStreamReader &xml, QObject *parent = nullptr);

    // This constructor reads a serialized METAR from a QDataStream
    explicit METAR(QDataStream &inputStream, QObject *parent = nullptr);

private:
    // Connects signals; this method is used internally from the constructor(s)
    void setupSignals() const;

    // Writes the METAR report to a data stream
    void write(QDataStream &out);

    Q_DISABLE_COPY_MOVE(METAR)

    // Decoded METAR text
    QString _decoded;

    // Flight category, as returned by the Aviation Weather Center
    FlightCategory _flightCategory {unknown};

    // Gust speed, as returned by the Aviation Weather Center
    AviationUnits::Speed _gust;

    // Station ID, as returned by the Aviation Weather Center
    QString _ICAOCode;

    // Station coordinate, as returned by the Aviation Weather Center
    QGeoCoordinate _location;

    // Observation time, as returned by the Aviation Weather Center
    QDateTime _observationTime;

    // QNH in hPa, as returned by the Aviation Weather Center
    quint16 _qnh {0};

    // Raw METAR text, as returned by the Aviation Weather Center
    QString _raw_text;

    // Wind speed, as returned by the Aviation Weather Center
    AviationUnits::Speed _wind;
};
}
