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
#include <QGeoCoordinate>
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
    ~NotamProvider() override = default;

    // factory function for QML singleton
    static NOTAM::NotamProvider* create(QQmlEngine* /*unused*/, QJSEngine* /*unused*/)
    {
        return GlobalObject::notamProvider();
    }

    Q_PROPERTY(QDateTime lastUpdate READ lastUpdate NOTIFY lastUpdateChanged)

    [[nodiscard]] QDateTime lastUpdate() const { return m_lastUpdate; }

    Q_INVOKABLE [[nodiscard]] NOTAM::NotamList notams(const GeoMaps::Waypoint& waypoint);

signals:
    void lastUpdateChanged();

    void notamsUpdated();

private slots:
    // Called when a download is finished
    void downloadFinished();

private:
    Q_DISABLE_COPY_MOVE(NotamProvider)

    void addJson(const QByteArray& data, QGeoCircle circle);

    // List of pending network requests
    QList<QPointer<QNetworkReply>> m_networkReplies;
    QList<NotamList> m_notamLists;

    QDateTime m_lastUpdate;

};

} // namespace NOTAM
