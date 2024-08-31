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
#include "units/Timespan.h"

namespace NOTAM {


/*! \brief A list of NOTAMs
 *
 *  This class holds a holds the result of a NOTAM request for a specific
 *  region. The class stores the time of the request in the member m_retrieved,
 *  and the region in the member m_region.
*/

class NotamList {
    Q_GADGET
    QML_VALUE_TYPE(notamList)

    friend QDataStream& operator<<(QDataStream& stream, const NOTAM::NotamList& notamList);
    friend QDataStream& operator>>(QDataStream& stream, NOTAM::NotamList& notamList);

public:
    /*! \brief Constructs an empty NotamList
     *
     *  Constructs an empty NotamList with an invalid region and an invalid
     *  QDateTime for the property retrieved.
     */
    NotamList() = default;

    /*! \brief Constructs a NotamList from FAA GeoJSON data
     *
     *  This constructor sets  the member m_retrieved to
     *  QDateTime::currentDateTimeUtc(). Invalid Notams and Cancel Notams
     *  will not be added to the list.
     *
     *  @param jsonDoc JSON dociment, as provided by the FAA
     *
     *  @param region Geographic region covered by this notam list
     *
     *  @param cancelledNotamNumbers Pointer to a set where numbers of cancelled
     *  Notams are added. The nullptr is allowed.
     */
    NotamList(const QJsonDocument& jsonDoc, const QGeoCircle& region, QSet<QString>* cancelledNotamNumbers=nullptr);



    //
    // Properties
    //

    /*! \brief Emptyness */
    Q_PROPERTY(bool isEmpty READ isEmpty)

    /*! \brief Validity */
    Q_PROPERTY(bool isValid READ isValid)

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
     *  @returns Property isEmpty
     */
    Q_REQUIRED_RESULT bool isEmpty() const { return m_notams.isEmpty(); }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property isValid
     */
    Q_REQUIRED_RESULT bool isValid() const { return m_retrieved.isValid() && m_region.isValid(); }

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

    /*! \brief Time span between retrieved and now
     *
     *  @returns Time span between retrieved and now. If retrieved() is invalid, and invalid time is returned.
     */
    Q_REQUIRED_RESULT Units::Timespan age() const;

    /*! \brief Sublist with expired and duplicated entries removed
     *
     *  @param cancelledNotamNumbers Set with numbers of notams that are
     *  known as cancelled
     *
     *  @returns Sublist with expired and duplicated entries removed.
     */
    Q_REQUIRED_RESULT NOTAM::NotamList cleaned(const QSet<QString>& cancelledNotamNumbers) const;

    /*! \brief Check if outdated
     *
     *  A NotamList is outdated if its age is invalid or greater than 24h
     *
     *  @returns True if outdated
     */
    Q_REQUIRED_RESULT bool isOutdated() const { auto _age = age(); return !_age.isFinite() || (_age > Units::Timespan::fromH(24)); }

    /*! \brief Check if list needs update
     *
     *  A NotamList needs an update if its age is invalid or greater than 12h
     *
     *  @returns True if outdated
     */
    Q_REQUIRED_RESULT bool needsUpdate() const { auto _age = age(); return !_age.isFinite() || (_age > Units::Timespan::fromH(12)); }

    /*! \brief Sublist of notams relevant to a given waypoint.
     *
     *  @param waypoint Waypoint
     *
     *  @returns NotamList with all notams centered within restrictionRadius of the given waypoint, without expired and duplicated NOTAMs.
     *  Section titles are set depending on the current time, using NOTAM::updateSectionTitle().
     */
    Q_REQUIRED_RESULT NOTAM::NotamList restricted(const GeoMaps::Waypoint& waypoint) const;

    /*! \brief Radius used in the method restricted() */
    static constexpr Units::Distance restrictionRadius = Units::Distance::fromNM(20.0);

private:
    /* List of Notams */
    QList<NOTAM::Notam> m_notams;

    /* Region */
    QGeoCircle m_region;

    /* Time of request */
    QDateTime m_retrieved;
};

/*! \brief Serialization
 *
 *  There is no checks for errors of any kind.
 */
QDataStream& operator<<(QDataStream& stream, const NOTAM::NotamList& notamList);

/*! \brief Deserialization
 *
 *  There is no checks for errors of any kind.
 */
QDataStream& operator>>(QDataStream& stream, NOTAM::NotamList& notamList);

} // namespace NOTAM


// Declare meta types
Q_DECLARE_METATYPE(NOTAM::NotamList)
