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


/*! \brief TAF report
 *
 * This class contains the data of a TAF report and provided a few
 * methods to access the data. Instances of this class are provided by the
 * WeatherDataProvider class; there is no way to construct valid instances yourself.
 */

class TAF {
    Q_GADGET
    QML_VALUE_TYPE(taf)

    friend WeatherDataProvider;

public:
    /*! \brief Default constructor
     *
     * This constructor creates an invalid TAF instance.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TAF();

    // Standard destructor
    ~TAF() = default;

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

    // ----

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

    /*! \brief Convenience method to check if this TAF is already expired
     *
     * @returns true if an expiration date/time is known and if the current time is larger than the expiration
     */
    [[nodiscard]] Q_INVOKABLE bool isExpired() const;

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

#warning
    [[nodiscard]] Q_INVOKABLE QString decodedText(const Navigation::Aircraft& act, const QDateTime& time)
    {
        return m_decoder.decodedText(act, time);
    }

private:
    // This constructor reads a XML stream, as provided by the Aviation Weather Center's Text Data Server,
    // https://www.aviationweather.gov/dataserver
    explicit TAF(QXmlStreamReader& xml);

    // This constructor reads a serialized TAF from a QDataStream
    explicit TAF(QDataStream& inputStream);

    // Writes the TAF report to a data stream
    void write(QDataStream& out);

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

#warning
    Weather::Decoder m_decoder;
};

} // namespace Weather
