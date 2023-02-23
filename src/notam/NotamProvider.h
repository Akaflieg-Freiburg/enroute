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
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate NOTIFY lastUpdateChanged)

    /*! \brief Waypoints with Notam items, for presentation in a map */
    Q_PROPERTY(QList<GeoMaps::Waypoint> waypoints READ waypoints NOTIFY waypointsChanged)



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


signals:
    /*! \brief Notifier signal */
    void lastUpdateChanged();

    /*! \brief Notifier signal */
    void waypointsChanged();

private slots:
    // This slot is connected to signals QNetworkReply::finished and
    // QNetworkReply::errorOccurred of the QNetworkReply contained in the list
    // in m_networkReply. This method reads the incoming data and adds it to the
    // database
    void downloadFinished();

    void autoUpdate();

    // Save NOTAM data to file whose name is found in m_stdFileName. There are
    // no error checks of any kind.
    void save() const;

    void clearOldEntries();

private:
    Q_DISABLE_COPY_MOVE(NotamProvider)

    void startRequest(const GeoMaps::Waypoint& waypoint);
    Units::Distance range(const QGeoCoordinate& waypoint);

    // List of pending network requests
    QList<QSharedPointer<QNetworkReply>> m_networkReplies;
    QList<NotamList> m_notamLists;



    QDateTime m_lastUpdate;

    static constexpr Units::Distance requestRadius = Units::Distance::fromNM(20.0);
    static constexpr Units::Distance marginRadius = Units::Distance::fromNM(5.0);

    // Filename for loading/saving NOTAM data
    QString m_stdFileName;
};

} // namespace NOTAM

