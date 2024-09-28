/***************************************************************************
 *   Copyright (C) 2023-2024 by Stefan Kebekus                             *
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

#include <QBindable>
#include <QNetworkReply>
#include <QQmlEngine>
#include <QStandardPaths>

#include "GlobalObject.h"
#include "notam/NOTAMList.h"

namespace NOTAM {

/*! \brief Manage NOTAM data and download NOTAM data from the FAA if required.
 *
 *  This class attempts to ensure that for any given point in time, current
 *  NOTAM data is provided for circles of radius minimumRadiusPoint around the
 *  current position and every waypoint in the flightRoute, as well as a region
 *  of at least minimumRadiusFlightRoute around the flight route.
 *
 *  There is API to access the data, and to request NOTAM data for arbitrary
 *  coordinates.
 */

class NOTAMProvider : public GlobalObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit NOTAMProvider(QObject* parent = nullptr);

    // deferred initialization
    void deferredInitialization() override;

    // No default constructor, important for QML singleton
    explicit NOTAMProvider() = delete;

    /*! \brief Standard destructor */
    ~NOTAMProvider() override;

    // factory function for QML singleton
    static NOTAMProvider* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::notamProvider();
    }

    // NOTAM data is considered to cover a given point if the data covers a
    // circle of radius minimumRadiusPoint around that point.
    static constexpr Units::Distance minimumRadiusPoint = Units::Distance::fromNM(20.0);

    // NOTAM data is considered to cover the flight route if it covers a region of at least
    // minimumRadiusFlightRoute around the route
    static constexpr Units::Distance minimumRadiusFlightRoute = Units::Distance::fromNM(3.0);


    //
    // Properties
    //

    /*! \brief List of NOTAM points
     *
     *  This property holds GeoJSON, to describe points where NOTAMs are active.
     */
    Q_PROPERTY(QByteArray geoJSON READ geoJSON BINDABLE bindableGeoJSON)

    /*! \brief Time of last database update
     *
     *  This is the time of the last successful data download from the FAA
     *  server. The property holds an invalid QDateTime if no data is available.
     */
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate BINDABLE bindableLastUpdate)

    /*! \brief Status
     *
     *  This is a translated, human-readable text with warnings about incomplete
     *  NOTAM data, or an empty string in case of no warning.
     */
    Q_PROPERTY(QString status READ status BINDABLE bindableStatus)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property with the same name
     *
     * @returns Property geoJSON
     */
    Q_REQUIRED_RESULT QByteArray geoJSON() const {return m_geoJSON.value();}

    /*! \brief Getter function for property with the same name
     *
     * @returns Property geoJSON
     */
    Q_REQUIRED_RESULT QBindable<QByteArray> bindableGeoJSON() const {return &m_geoJSON;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastUpdate
     */
    Q_REQUIRED_RESULT QDateTime lastUpdate() const {return {m_lastUpdate};}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastUpdate
     */
    Q_REQUIRED_RESULT QBindable<QDateTime> bindableLastUpdate() const {return &m_lastUpdate;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property status
     */
    Q_REQUIRED_RESULT QString status() const {return {m_status};}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property status
     */
    Q_REQUIRED_RESULT QBindable<QString> bindableStatus() const {return &m_status;}


    //
    // Methods
    //


    /*! \brief NOTAMs for a given waypoint
     *
     *  The returned list is empty and has a valid property "retrieved" if the
     *  NOTAMProvider is sure that there are no relevant NOTAMs for the given
     *  waypoint.
     *
     *  The returned list is empty and has an invalid property "retrieved" if
     *  the NOTAMProvider has no data.
     *
     *  Calling this method might trigger an update of the NOTAM database.
     *  Consumers can watch the property lastUpdate to learn about database
     *  updates.
     *
     *  @param waypoint Waypoint for which the notam list is compiled
     *
     *  @returns List of NOTAMS relevant for the waypoint
     */
    Q_REQUIRED_RESULT Q_INVOKABLE NOTAMList notams(const GeoMaps::Waypoint& waypoint);

    /*! \brief Check if a NOTAM number is registred as read
     *
     *  @param number Notam number
     *
     *  @returns True is notam is known as read
     */
    Q_REQUIRED_RESULT Q_INVOKABLE bool isRead(const QString& number) const { return m_readNotamNumbers.contains(number); }

    /*! \brief Register NOTAM number as read or unread
     *
     *  @param number Notam number
     *
     *  @param read True if notam is to be registred as read
     */
    Q_INVOKABLE void setRead(const QString& number, bool read);

private:


private:
    Q_DISABLE_COPY_MOVE(NOTAMProvider)


    //
    // Private Methods
    //

    // Removes outdated NOTAMs and outdated NOTAMLists.
    Q_REQUIRED_RESULT static QList<NOTAMList> cleaned(const QList<NOTAMList>& notamLists, const QSet<QString>& cancelledNotams = {});

    // This method reads the incoming data from network replies and adds it to
    // the database. It cleans up the list of network replies in
    // m_networkReplies. On error, it requests a call to updateData in five
    // minutes. This method is connected to signals QNetworkReply::finished and
    // QNetworkReply::errorOccurred of the QNetworkReply contained in the list
    // in m_networkReply.
    void downloadFinished();

    // Check if current NOTAM data exists for a circle of radius minimalRadius
    // around position. This method ignores outdated NOTAM data. An invalid
    // position is always considered to be covered.
    //
    // includeDataThatNeedsUpdate: If true, then also count NOTAM lists that
    // need an update as NOTAM data
    //
    // includeRunningDownloads: If true, then also count running downloads as
    // NOTAM data
    Q_REQUIRED_RESULT bool hasDataForPosition(const QGeoCoordinate& position, bool includeDataThatNeedsUpdate, bool includeRunningDownloads) const;

    // Save NOTAM data to a file, using the filename found in m_stdFileName.
    // There are no error checks of any kind. The propertyNotifier ensures that
    // the method save() is called whenever m_notamLists changes.
    void save() const;
    QPropertyNotifier m_saveNotifier;

    // Request NOTAM data from the FAA, for a circle of radius requestRadius
    // around the coordinate.  For performance reasons, the request will be
    // ignored if existing NOTAM data or ongoing download requests cover the
    // position alreads.
    void startRequest(const QGeoCoordinate& coordinate);

    // Checks if NOTAM data is available for an area of marginRadius around the
    // current position and around the current flight route. If not, requests
    // the data.
    void updateData();


    //
    // Private Members and Member Computing Methods
    //

    // List with numbers of notams that have been marked as read
    QList<QString> m_readNotamNumbers;

    // List of pending network requests
    QList<QPointer<QNetworkReply>> m_networkReplies;

    // List of NOTAMLists, sorted so that newest lists come first
    QProperty<QList<NOTAMList>> m_notamLists;

    // This is a list of control points.  The computing function guarantees that
    // the NOTAM data covers a region of at least marginRadiusFlightRoute around
    // the route if the data covers a circle of radius marginRadius around every
    // control point point. Exeption: For performance reasons, this guarantee is
    // lifted if the flight route contains a leg of size >
    // maximumFlightRouteLegLength.
    QProperty<QList<QGeoCoordinate>> m_controlPoints4FlightRoute;
    Q_REQUIRED_RESULT static QList<QGeoCoordinate> computeControlPoints4FlightRoute();

    // GeoJSON, for use in map
    QProperty<QByteArray> m_geoJSON;
    Q_REQUIRED_RESULT QByteArray computeGeoJSON() const;

    // Time of last update to data
    QProperty<QDateTime> m_lastUpdate;
    Q_REQUIRED_RESULT QDateTime computeLastUpdate() const;

    // Filename for loading/saving NOTAM data
    QProperty<QString> m_status;
    Q_REQUIRED_RESULT QString computeStatus() const;

    // Filename for loading/saving NOTAM data
    QString m_stdFileName { QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+u"/notam.dat"_qs };

    // NOTAM data is considered to cover the flight route if it covers a region
    // of at least marginRadiusFlightRoute around the route
    static constexpr Units::Distance maximumFlightRouteLegLength = Units::Distance::fromNM(200.0);

    // Requests for Notam data are requestRadius around given position. This is
    // the maximum that FAA API currently allows (FAA max is 100NM)
    static constexpr Units::Distance requestRadius = Units::Distance::fromNM(99.0);
};

} // namespace NOTAM

