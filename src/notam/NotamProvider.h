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

#include <QNetworkReply>
#include <QQmlEngine>

#include "GlobalObject.h"
#include "notam/NotamList.h"

namespace NOTAM {

/*! \brief This extremely simple class holds a the data item of a NOTAM */

class NotamProvider : public GlobalObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit NotamProvider(QObject* parent = nullptr);

    // deferred initialization
    void deferredInitialization() override;

    // No default constructor, important for QML singleton
    explicit NotamProvider() = delete;

    /*! \brief Standard destructor */
    ~NotamProvider() override;

    // factory function for QML singleton
    static NOTAM::NotamProvider* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::notamProvider();
    }



    //
    // Properties
    //

    /*! \brief Time of last database update */
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate NOTIFY dataChanged)

    /*! \brief Waypoints with Notam items, for presentation in a map */
    Q_PROPERTY(QList<GeoMaps::Waypoint> waypoints READ waypoints NOTIFY dataChanged)



    //
    // Getter Methods
    //

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastUpdate
     */
    Q_REQUIRED_RESULT QDateTime lastUpdate() const { return m_lastUpdate; }

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property waypoints
     */
    Q_REQUIRED_RESULT QList<GeoMaps::Waypoint> waypoints() const;



    //
    // Methods
    //

    /*! \brief Notams for a given waypoint
     *
     *  The returned list is empty and has a valid property "retrieved" if the
     *  NotamProvider is sure that there are no relevant notams for the given
     *  waypoint.
     *
     *  The returned list is empty and has an invalid property "retrieved" if
     *  the NotamProvider has no data.
     *
     *  Calling this method might trigger an update of the Notam database.
     *  Consumers can watch the property lastUpdate to learn about database
     *  updates.
     *
     *  @param waypoint Waypoint for which the notam list is compiled
     *
     *  @returns List of Notams relevant for the waypoint
     */
    Q_INVOKABLE Q_REQUIRED_RESULT NOTAM::NotamList notams(const GeoMaps::Waypoint& waypoint);

    /*! \brief Check is a notam number is registred as read
     *
     *  @param number Notam number
     *
     *  @returns True is notam is known as read
     */
    Q_INVOKABLE Q_REQUIRED_RESULT bool isRead(const QString& number) { return m_readNotamNumbers.contains(number); }

    /*! \brief Register notam number as read or unread
     *
     *  @param number Notam number
     *
     *  @param read True if notam is to be registred as read
     */
    Q_INVOKABLE void setRead(const QString& number, bool read);

signals:
    /*! \brief Notifier signal */
    void dataChanged();

private slots:   
    // Removes outdated and irrelevant data from the database. This slot is called
    // once per hour.
    void clean();

    // Clear all data and upateData(). This is called when API keys change.
    void clearAllAndUpdate();

    // This slot is connected to signals QNetworkReply::finished and
    // QNetworkReply::errorOccurred of the QNetworkReply contained in the list
    // in m_networkReply. This method reads the incoming data and adds it to the
    // database
    void downloadFinished();

    // Save NOTAM data to file whose name is found in m_stdFileName. There are
    // no error checks of any kind.
    void save() const;

    // Checks if NOTAM data is available for an area of marginRadius around the
    // current position and around the current flight route. If not, requests
    // the data.
    void updateData();

private:
    Q_DISABLE_COPY_MOVE(NotamProvider)

    // Compute the radius of the circle around the waypoint that is covered by
    // existing or requested notam data. Returns Units::Distance::fromM(-1) if
    // the waypoint is not covered by data.
    Units::Distance range(const QGeoCoordinate& position);

    // Request Notam data from the FAA, for a circle of radius requestRadius
    // around the coordinate.
    void startRequest(const QGeoCoordinate& coordinate);

    // List with number of read notams
    QList<QString> m_readNotamNumbers;

    // List of pending network requests
    QList<QPointer<QNetworkReply>> m_networkReplies;

    // List of NotamLists, sorted so that newest lists come first
    QList<NotamList> m_notamLists;

    // Time of last update to data
    QDateTime m_lastUpdate;

    // Filename for loading/saving NOTAM data
    QString m_stdFileName;

    // The method updateDate() ensures that data is requested for marginRadius around
    // own position and current flight route.
    static constexpr Units::Distance marginRadius = Units::Distance::fromNM(5.0);

    // Requests for Notam data are requestRadius around given position
    static constexpr Units::Distance requestRadius = Units::Distance::fromNM(20.0);
};

} // namespace NOTAM

