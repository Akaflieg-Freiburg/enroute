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

    /*! \brief List of NOTAM points
     *
     *  This property holds GeoJSON, to describe points where NOTAMs are active.
     */
    Q_PROPERTY(QByteArray geoJSON READ geoJSON BINDABLE bindableGeoJSON)

    /*! \brief Time of last database update */
    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate BINDABLE bindableLastUpdate)

    /*! \brief Status
     *
     *  This is a translated, human-readable text with warnings about incomplete NOTAM data,
     *  or an empty string in case of no warning.
     */
    Q_PROPERTY(QString status READ status BINDABLE bindableStatus)


    //
    // Getter Methods
    //

    /*! \brief Getter function for property with the same name
     *
     * @returns Property GeoJSON
     */
    Q_REQUIRED_RESULT QByteArray geoJSON() const {return {m_geoJSON};}
    Q_REQUIRED_RESULT QBindable<QByteArray> bindableGeoJSON() {return &m_geoJSON;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property lastUpdate
     */
    Q_REQUIRED_RESULT QDateTime lastUpdate() const {return {m_lastUpdate};}
    Q_REQUIRED_RESULT QBindable<QDateTime> bindableLastUpdate() {return &m_lastUpdate;}

    /*! \brief Getter function for the property with the same name
     *
     *  @returns Property status
     */
    Q_REQUIRED_RESULT QString status() const {return {m_status};}
    Q_REQUIRED_RESULT QBindable<QString> bindableStatus() {return &m_status;}


    //
    // Methods
    //

    /*! \brief NOTAMs for a given waypoint
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
    [[nodiscard]] Q_INVOKABLE NOTAM::NotamList notams(const GeoMaps::Waypoint& waypoint);

    /*! \brief Check if a NOTAM number is registred as read
     *
     *  @param number Notam number
     *
     *  @returns True is notam is known as read
     */
    [[nodiscard]] Q_INVOKABLE bool isRead(const QString& number) { return m_readNotamNumbers.contains(number); }

    /*! \brief Register NOTAM number as read or unread
     *
     *  @param number Notam number
     *
     *  @param read True if notam is to be registred as read
     */
    Q_INVOKABLE void setRead(const QString& number, bool read);

signals:
    /*! \brief Notifier signal */
    void dataChanged();

private:
    // Property bindings
    QByteArray computeGeoJSON();
    QDateTime computeLastUpdate();
    QString computeStatus();

private slots:   
    // Removes outdated and irrelevant data from the database. This slot is called
    // once per hour.
    void clean();

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

    QPropertyNotifier m_saveNotifier;

    // Compute the radius of the circle around the waypoint that is covered by
    // existing or requested notam data. Returns Units::Distance::fromM(-1) if
    // the waypoint is not covered by data.
    Units::Distance range(const QGeoCoordinate& position);

    // Request Notam data from the FAA, for a circle of radius requestRadius
    // around the coordinate.
    void startRequest(const QGeoCoordinate& coordinate);

    // List with numbers of notams that have been marked as read
    QList<QString> m_readNotamNumbers;

    // Set with numbers of notams that have been cancelled
    QSet<QString> m_cancelledNotamNumbers;

    // List of pending network requests
    QList<QPointer<QNetworkReply>> m_networkReplies;

    // List of NotamLists, sorted so that newest lists come first
    QProperty<QList<NotamList>> m_notamLists;

    // GeoJSON, for use in map
    QProperty<QByteArray> m_geoJSON;

    // Time of last update to data
    QProperty<QDateTime> m_lastUpdate;

    // Filename for loading/saving NOTAM data
    QProperty<QString> m_status;

    // Filename for loading/saving NOTAM data
    QString m_stdFileName { QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+u"/notam.dat"_qs };

    // The method updateDate() ensures that data is requested for marginRadius around
    // own position and current flight route.
    static constexpr Units::Distance marginRadius = Units::Distance::fromNM(5.0);

    // Requests for Notam data are requestRadius around given position.
    // This is the maximum that FAA API currently allows (FAA max is 100NM, but
    // the number here is multiplied internally with a factor of 1.2)
    static constexpr Units::Distance requestRadius = Units::Distance::fromNM(83.0);
};

} // namespace NOTAM

