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


/*! \brief TAF report
 *
 * This class contains the data of a TAF report and provided a few
 * methods to access the data. Instances of this class are provided by the
 * DownloadManager class; there is no way to construct valid instances yourself.
 */

class TAF : public Decoder {
    Q_OBJECT

    friend DownloadManager;

public:
    /*! \brief Default constructor
     *
     * This constructor creates an invalid TAF instance.
     *
     * @param parent The standard QObject parent pointer
     */
    explicit TAF(QObject *parent = nullptr);

    // Standard destructor
    ~TAF() override = default;

    /*! \brief Geographical coordinate of the station reporting this TAF
     *
     * If the station coordinate is unknown, the property contains an invalid coordinate.
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
     * A TAF message is supposed to expire once the last forecast period ends.
     */
    Q_PROPERTY(QDateTime expiration READ expiration CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property expiration
     */
    QDateTime expiration() const
    {
        return _expirationTime;
    }

    /*! \brief ICAO code of the station reporting this TAF
     *
     * This is a string such as "LSZB" for Bern/Belp airport. If the station code is unknown,
     * the property contains an empty string.
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

    /*! \brief Convenience method to check if this TAF is already expired
     *
     * @returns true if an expiration date/time is known and if the current time is larger than the expiration
     */
    Q_INVOKABLE bool isExpired() const;

    /*! Indicates if the class represents a valid TAF report */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property isValid
     */
    bool isValid() const;

    /*! \brief Issue time of this TAF */
    Q_PROPERTY(QDateTime issueTime READ issueTime CONSTANT)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property observationTime
     */
    QDateTime issueTime() const
    {
        return _issueTime;
    }

    /*! \brief  Raw TAF text
     *
     * This is a string such as "TAF EICK 092100Z 23007KT 9999 FEW038 BKN180 11/08 Q1019 NOSIG=".
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

    /*! \brief Issue time, relative to now
     *
     * This is a translated, human-readable string such as "1h and 43min ago" that describes
     * the observation time.
     */
    Q_PROPERTY(QString relativeIssueTime READ relativeIssueTime NOTIFY relativeIssueTimeChanged)

    /*! \brief Getter function for property with the same name
     *
     * @returns Property relativeObservationTime
     */
    QString relativeIssueTime() const;

signals:
    /*! \brief Notifier signal */
    void relativeIssueTimeChanged();

private:
    // This constructor reads a XML stream, as provided by the Aviation Weather Center's Text Data Server,
    // https://www.aviationweather.gov/dataserver
    explicit TAF(QXmlStreamReader &xml, QObject *parent = nullptr);

    // This constructor reads a serialized TAF from a QDataStream
    explicit TAF(QDataStream &inputStream, QObject *parent = nullptr);

    // Connects signals; this method is used internally from the constructor(s)
    void setupSignals() const;

    // Writes the TAF report to a data stream
    void write(QDataStream &out);

    Q_DISABLE_COPY_MOVE(TAF)

    // Expiration time
    QDateTime _expirationTime;

    // Station ID, as returned by the Aviation Weather Center
    QString _ICAOCode;

    // Issue time, as returned by the Aviation Weather Center
    QDateTime _issueTime;

    // Station coordinate, as returned by the Aviation Weather Center
    QGeoCoordinate _location;

    // Raw TAF text, as returned by the Aviation Weather Center
    QString _raw_text;
};

}
