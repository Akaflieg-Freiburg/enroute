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

    /*! \brief Rich text description of the Notam */
    Q_PROPERTY(QString richText READ richText)



    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property richText
     */
    QString richText() const;



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


    //
    // Members
    //

    /* Notam members, as described by the FAA */
    QGeoCoordinate  m_coordinates;
    QDateTime       m_effectiveEnd;
    QString         m_effectiveEndString;
    QDateTime       m_effectiveStart;
    QString         m_effectiveStartString;
    QString         m_icaoLocation;
    QString         m_number;
    Units::Distance m_radius;
    QGeoCircle      m_region;
    QString         m_text;
    QString         m_traffic;
};

} // namespace NOTAM

/*! \brief Serialization */
QDataStream& operator<<(QDataStream& stream, const NOTAM::Notam &notam);

/*! \brief Deserialization */
QDataStream& operator>>(QDataStream& stream, NOTAM::Notam& notam);

// Declare meta types
Q_DECLARE_METATYPE(NOTAM::Notam)
