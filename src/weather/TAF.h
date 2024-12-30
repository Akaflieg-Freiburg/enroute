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

#include <QDataStream>
#include <QGeoCoordinate>
#include <QXmlStreamReader>

#include "weather/Decoder.h"


namespace Weather {

class WeatherDataProvider;


/*! \brief TAF forecast
 *
 * This class contains the data of a TAF report and provided a few
 * methods to access the data. Instances of this class are provided by the
 * WeatherDataProvider class; there is no way to construct valid instances yourself.
 */

class TAF {
    Q_GADGET
    QML_VALUE_TYPE(taf)

    friend QDataStream& operator<<(QDataStream& stream, const TAF& taf);
    friend QDataStream& operator>>(QDataStream& stream, Weather::TAF& taf);

public:

    //
    // Constructors and destructors
    //

    /*! \brief Default constructor */
    TAF() = default;

    /*! \brief Deserialization constructor
     *
     * This constructor reads a XML stream, as provided by the Aviation Weather
     * Center's Text Data Server, https://www.aviationweather.gov/dataserver
     *
     * @param xml XML Stream reader
     */
    explicit TAF(QXmlStreamReader& xml);

    /*! \brief Destructor */
    ~TAF() = default;

    /*! \brief Copy constructor */
    TAF(const TAF&) = default;

    /*! \brief Move constructor */
    TAF(TAF&&) = default;

    /*! \brief Copy assignment operator
     *
     *  @returns Reference to self
     */

    TAF& operator=(const TAF&) = default;

    /*! \brief Move assignment operator
     *
     *  @returns Reference to self
     */

    TAF& operator=(TAF&&) = default;

    /*! \brief Equality check
     *
     *  @param other Other TAF to compare with
     *
     *  @returns True on equality
     */
    bool operator==(const TAF& other) const = default;


    //
    // Properties
    //

    /*! \brief Geographical coordinate of the station reporting this TAF
     *
     * If the station coordinate is unknown, the property contains an invalid coordinate.
     */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Expiration time and date
     *
     * A TAF message is supposed to expire once the last forecast period ends.
     */
    Q_PROPERTY(QDateTime expiration READ expiration CONSTANT)

    /*! \brief ICAO code of the station reporting this TAF
     *
     * This is a string such as "LSZB" for Bern/Belp airport. If the station code is unknown,
     * the property contains an empty string.
     */
    Q_PROPERTY(QString ICAOCode READ ICAOCode CONSTANT)

    /*! Indicates if the class represents a valid TAF report */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Issue time of this TAF */
    Q_PROPERTY(QDateTime issueTime READ issueTime CONSTANT)

    /*! \brief  Raw TAF text
     *
     * This is a string such as "TAF EICK 092100Z 23007KT 9999 FEW038 BKN180 11/08 Q1019 NOSIG=".
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
    [[nodiscard]] QDateTime expiration() const
    {
        return m_expirationTime;
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
    [[nodiscard]] QDateTime issueTime() const
    {
        return m_issueTime;
    }

    /*! \brief Getter function for property with the same name
     *
     * @returns Property extendedName
     */
    [[nodiscard]] QString rawText() const
    {
        return m_rawText;
    }


    //
    // Methods
    //

    /*! \brief Decoded TAF text
     *
     * @param act Current aircraft, used to determine appropriate units
     *
     * @param time Current time, used to describe points in time
     *
     * @returns Human-readable, translated rich text.
     */
    [[nodiscard]] Q_INVOKABLE QString decodedText(const Navigation::Aircraft& act, const QDateTime& time)
    {
        // Paranoid safety checks
        if (m_decoder.isNull())
        {
            return {};
        }

        return m_decoder->decodedText(act, time);
    }

private:
    // Expiration time
    QDateTime m_expirationTime;

    // Station ID, as returned by the Aviation Weather Center
    QString m_ICAOCode;

    // Issue time, as returned by the Aviation Weather Center
    QDateTime m_issueTime;

    // Station coordinate, as returned by the Aviation Weather Center
    QGeoCoordinate m_location;

    // Raw TAF text, as returned by the Aviation Weather Center
    QString m_rawText;

    // Decoder
    QSharedPointer<Weather::Decoder> m_decoder;
};

/*! \brief Serialization
 *
 *  There are no checks for errors of any kind.
 */
QDataStream& operator<<(QDataStream& stream, const TAF& taf);


/*! \brief Deserialization
 *
 *  There are no checks for errors of any kind.
 */
QDataStream& operator>>(QDataStream& stream, Weather::TAF& taf);

} // namespace Weather
