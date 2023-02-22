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

    /*! \brief Coordinates of the Notam */
    Q_PROPERTY(QGeoCoordinate coordinate READ coordinate CONSTANT)

    /*! \brief Effective end of the Notam, if date is given
     *
     *  If the effectiveEnd field of the Notam specified a precise date/time,
     *  then this time is found here. If not, the property contains an invalid
     *  QDateTime.
     */
    Q_PROPERTY(QDateTime effectiveEnd READ effectiveEnd CONSTANT)

    /*! \brief Rich text description of the Notam */
    Q_PROPERTY(QGeoCircle region READ region CONSTANT)

    /*! \brief Rich text description of the Notam */
    Q_PROPERTY(QString richText READ richText CONSTANT)

    /*! \brief Traffic entry of the Notam */
    Q_PROPERTY(QString traffic READ traffic CONSTANT)


    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property coordinate
     */
    QGeoCoordinate coordinate() const { return m_coordinates; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property effectiveEnd
     */
    QDateTime effectiveEnd() const { return m_effectiveEnd; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property region
     */
    QGeoCircle region() const { return m_region; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property richText
     */
    QString richText() const;

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property traffic
     */
    QString traffic() const { return m_traffic; }



    //
    // Methods
    //

    /*! \brief Read coordinate in NOTAM format
     *
     *  This method converts a string with a coordinate description into
     *  a QGeoCoordinate. The string must be exactly 11 characters long and
     *  adhere to the following format:
     *
     *  AABBCDDDEEF
     *
     *  AA:  Degrees of latitude
     *  BB:  Minutes of latitude
     *  C:   'N' or 'S'
     *  DDD: Degrees of longitude
     *  EE:  Minutes of longitude
     *  F:   'E' or 'W'
     *
     *  @param string String of the form
     *
     *  @returns Interpreted QGeoCoordinate, or an invalid coordinate on error
     *
     */
    QGeoCoordinate static interpretNOTAMCoordinates(const QString& string);

    /*! \brief Comparison */
    Q_INVOKABLE [[nodiscard]] bool operator==(const NOTAM::Notam& rhs) const = default;


private:
    /* Notam members, as described by the FAA */
    QGeoCoordinate  m_coordinates;
    QString         m_effectiveEndString;
    QString         m_effectiveStartString;
    QString         m_icaoLocation;
    QString         m_number;
    Units::Distance m_radius;
    QString         m_text;
    QString         m_traffic;

    /* Derivative data, obtained from the FAA notam members above */
    QDateTime       m_effectiveEnd;
    QDateTime       m_effectiveStart;
    QGeoCircle      m_region;
};

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
