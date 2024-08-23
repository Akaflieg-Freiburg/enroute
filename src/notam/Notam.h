/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include <QDateTime>
#include <QGeoCircle>
#include <QQmlEngine>

#include "units/Distance.h"

namespace NOTAM {

/*! \brief This extremely simple class holds a the data item of a NOTAM */

class Notam {
    Q_GADGET
    QML_VALUE_TYPE(notam)

    friend QDataStream& operator<<(QDataStream& stream, const NOTAM::Notam &notam);
    friend QDataStream& operator>>(QDataStream& stream, NOTAM::Notam& notam);

public:
    /*! \brief Constructs an invalid Notam */
    Notam() = default;

    /*! \brief Constructs a Notam from GeoJSON data, as provided by the FAA
     *
     *  @param jsonObject JSON object, as provided by the FAA
     */
    explicit Notam(const QJsonObject& jsonObject);


    //
    // Properties
    //

    /*! \brief Flight Information Region of this NOTAM */
    Q_PROPERTY(QString affectedFIR READ affectedFIR CONSTANT)

    /*! \brief Cancels other Notam
     *
     *  If this is a cancel notam, then this property holds the number
     *  of the notam that is to be cancelled. Otherwise, this property
     *  holds an empty string.
     */
    Q_PROPERTY(QString cancels READ cancels CONSTANT)

    /*! \brief Coordinates of the Notam */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Effective end of the Notam, if date is given
     *
     *  If the effectiveEnd field of the Notam specified a precise date/time,
     *  then this time is found here. If not, the property contains an invalid
     *  QDateTime.
     */
    Q_PROPERTY(QDateTime effectiveEnd READ effectiveEnd CONSTANT)

    /*! \brief Effective start of the Notam, if date is given
     *
     *  If the effectiveStart field of the Notam specified a precise date/time,
     *  then this time is found here. If not, the property contains an invalid
     *  QDateTime.
     */
    Q_PROPERTY(QDateTime effectiveStart READ effectiveStart CONSTANT)

    /*! \brief Waypoint in GeoJSON format
     */
    Q_PROPERTY(QJsonObject GeoJSON READ GeoJSON CONSTANT)

    /*! \brief ICAO location of this NOTAM */
    Q_PROPERTY(QString icaoLocation READ icaoLocation CONSTANT)

    /*! \brief Validity of this NOTAM */
    Q_PROPERTY(bool isValid READ isValid CONSTANT)

    /*! \brief Number of this NOTAM */
    Q_PROPERTY(QString number READ number CONSTANT)

    /*! \brief Region where this NOTAM is valid */
    Q_PROPERTY(QGeoCircle region READ region CONSTANT)

#warning
    Q_PROPERTY(QString sectionTitle READ sectionTitle CONSTANT)

    /*! \brief Traffic entry of the NOTAM */
    Q_PROPERTY(QString traffic READ traffic CONSTANT)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property affectedFIR
     */
    Q_REQUIRED_RESULT QString affectedFIR() const { return m_affectedFIR; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property cancels
     */
    Q_REQUIRED_RESULT QString cancels() const;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property coordinate
     */
    Q_REQUIRED_RESULT QGeoCoordinate coordinate() const { return m_coordinate; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property effectiveEnd
     */
    Q_REQUIRED_RESULT QDateTime effectiveEnd() const { return m_effectiveEnd; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property effectiveStart
     */
    Q_REQUIRED_RESULT QDateTime effectiveStart() const { return m_effectiveStart; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property GeoJSON
     */
    Q_REQUIRED_RESULT QJsonObject GeoJSON() const;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property icaoLocation
     */
    Q_REQUIRED_RESULT QString icaoLocation() const { return m_icaoLocation; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property isValid
     */
    Q_REQUIRED_RESULT bool isValid() const;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property number
     */
    Q_REQUIRED_RESULT QString number() const { return m_number; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property region
     */
    Q_REQUIRED_RESULT QGeoCircle region() const { return m_region; }

    Q_REQUIRED_RESULT QString sectionTitle() const {
        return {};
#warning Need to work here
        if (m_icaoLocation == m_affectedFIR)
        {
            return u"FIR %1"_qs.arg(m_icaoLocation);
        }
        return m_icaoLocation;
    }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property traffic
     */
    Q_REQUIRED_RESULT QString traffic() const { return m_traffic; }



    //
    // Methods
    //

    /*! \brief Comparison
     *
     *  @param rhs Right hand side of the comparison
     *
     *  @returns True on equality.
     */
    Q_REQUIRED_RESULT [[nodiscard]] Q_INVOKABLE bool operator==(const NOTAM::Notam& rhs) const = default;

    /*! \brief Check if effectiveEnd is valid and earlier than currentTime
     *
     *  @return True if effectiveEnd is valid and earlier than currentTime
     */
    Q_REQUIRED_RESULT bool isOutdated() const
    {
        return m_effectiveEnd.isValid() && (m_effectiveEnd < QDateTime::currentDateTimeUtc());
    }

    /*! \brief Rich text description of the Notam
     *
     *  The description and changes with time (e.g. when passing the effective start
     *  date of the Notam.
     *
     *  @return HTML string
     */
    Q_REQUIRED_RESULT Q_INVOKABLE QString richText() const;


private:
    /* Notam members, as described by the FAA */
    QString         m_affectedFIR;
    QGeoCoordinate  m_coordinate;
    QString         m_effectiveEndString;
    QString         m_effectiveStartString;
    QString         m_icaoLocation;
    QString         m_maximumFL;
    QString         m_minimumFL;
    QString         m_number;
    Units::Distance m_radius;
    QString         m_schedule;
    QString         m_text;
    QString         m_traffic;

    /* Derivative data, obtained from the FAA notam members above */
    QDateTime       m_effectiveEnd;
    QDateTime       m_effectiveStart;
    QGeoCircle      m_region;
};


/*! \brief Read coordinate in NOTAM format
 *
 *  This method converts a string with a coordinate description into a
 *  QGeoCoordinate. The string must be exactly 11 characters long and adhere to
 *  the following format:
 *
 *  AABBCDDDEEF
 *
 *  - AA:  Degrees of latitude
 *  - BB:  Minutes of latitude
 *  - C:   'N' or 'S'
 *  - DDD: Degrees of longitude
 *  - EE:  Minutes of longitude
 *  - F:   'E' or 'W'
 *
 *  @param string String of the form
 *
 *  @returns Interpreted QGeoCoordinate, or an invalid coordinate on error
 *
 */
QGeoCoordinate interpretNOTAMCoordinates(const QString& string);

/*! \brief Serialization
 *
 *  There is no checks for errors of any kind.
 */
QDataStream& operator<<(QDataStream& stream, const NOTAM::Notam &notam);

/*! \brief Deserialization
 *
 *  There is no checks for errors of any kind.
 */
QDataStream& operator>>(QDataStream& stream, NOTAM::Notam& notam);

} // namespace NOTAM

// Declare meta types
Q_DECLARE_METATYPE(NOTAM::Notam)
