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

#include <QQmlEngine>

#include "geomaps/Waypoint.h"
#include "notam/Notam.h"

namespace NOTAM {


/*! \brief A list of NOTAMs
 *
 *  This class holds a holds the result of a NOTAM request for a specific region.
 *  The class stores the time of the request in the member m_retrieved, and
 *  the region in the member m_region.
*/

class NotamList {
    Q_GADGET
    QML_VALUE_TYPE(notamList)

    friend QDataStream& operator<<(QDataStream& stream, const NOTAM::NotamList& notamList);
    friend QDataStream& operator>>(QDataStream& stream, NOTAM::NotamList& notamList);

public:
    /*! \brief Constructs an empty NotamList
     *
     *  Constructs an empty NotamList with an invalid region and an invalid QDateTime for the property
     *  retrieved.
     */
    NotamList() = default;

    /*! \brief Constructs a NotamList from FAA GeoJSON data
     *
     *  This constructor sets  the member m_retrieved to QDateTime::currentDateTimeUtc().
     *
     *  @param jsonData JSON data, as provided by the FAA
     *
     *  @param region Geographic region covered by this notam list
     */
    NotamList(const QByteArray& jsonData, const QGeoCircle& region);



    //
    // Properties
    //

    /*! \brief List of Notams */
    Q_PROPERTY(QList<NOTAM::Notam> notams READ notams)

    /*! \brief Region covered by this list */
    Q_PROPERTY(QGeoCircle region READ region)

    /*! \brief Date of retrieval */
    Q_PROPERTY(QDateTime retrieved READ retrieved)

    /*! \brief One-line summary */
    Q_PROPERTY(QString summary READ summary)



    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property notams
     */
    Q_REQUIRED_RESULT QList<NOTAM::Notam> notams() const { return m_notams; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property region
     */
    Q_REQUIRED_RESULT QGeoCircle region() const { return m_region; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property retrieved
     */
    Q_REQUIRED_RESULT QDateTime retrieved() const { return m_retrieved; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property summary
     */
    Q_REQUIRED_RESULT QString summary() const;



    //
    // Methods
    //

    /*! \brief Sublist with expired and duplicated entries removed
     *
     *  @returns Sublist with expired and duplicated entries removed.
     */
    Q_REQUIRED_RESULT NOTAM::NotamList cleaned() const;

    /*! \brief Sublist of notams relevant to a given waypoint.
     *
     *  @param waypoint Waypoint
     *
     *  @returns NotamList with all notams relevant for the given waypoint, without expired and duplicated.
     */
    Q_REQUIRED_RESULT NOTAM::NotamList restricted(const GeoMaps::Waypoint& waypoint) const;


private:
    /* List of Notams */
    QList<NOTAM::Notam> m_notams;

    /* Region */
    QGeoCircle m_region;

    /* Time of request */
    QDateTime m_retrieved;
};

/*! \brief Serialization */
QDataStream& operator<<(QDataStream& stream, const NOTAM::NotamList& notamList);

/*! \brief Deserialization */
QDataStream& operator>>(QDataStream& stream, NOTAM::NotamList& notamList);

} // namespace NOTAM


// Declare meta types
Q_DECLARE_METATYPE(NOTAM::NotamList)
