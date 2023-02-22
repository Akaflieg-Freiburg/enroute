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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QTimer>

#include "geomaps/Waypoint.h"
#include "notam/NotamProvider.h"
#include <chrono>

using namespace std::chrono_literals;


NOTAM::NotamProvider::NotamProvider(QObject* parent) :
    GlobalObject(parent),
    stdFileName(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+u"/notam.dat"_qs)

{
}

void NOTAM::NotamProvider::deferredInitialization()
{
    load();

    QTimer::singleShot(10s, this, &NOTAM::NotamProvider::autoUpdate);

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &NOTAM::NotamProvider::autoUpdate);
    timer->setInterval(10*60*1000);
    timer->start();
}


NOTAM::NotamList NOTAM::NotamProvider::notams(const GeoMaps::Waypoint& waypoint)
{
    NotamList result;

    // Check if notams for the location are present in our database
    foreach (auto notamList, m_notamLists)
    {
        result = notamList.restrict(waypoint);
        if (!result.m_notams.isEmpty())
        {
            return result;
        }
    }

    // Check if internet requests notams for the location are pending.
    // In that case, return an empty list.
    foreach(auto networkReply, m_networkReplies)
    {
        // Paranoid safety checks
        if (networkReply.isNull())
        {
            continue;
        }
        auto area = networkReply->property("area").value<QGeoCircle>();
        if (!area.isValid())
        {
            continue;
        }
        if (area.contains(waypoint.coordinate()))
        {
            return {};
        }
    }

    startRequest(waypoint);
    return {};
}


void NOTAM::NotamProvider::downloadFinished()
{
    qWarning() << "downloadFinished()";

    m_networkReplies.removeAll(nullptr);

    // Read all replies and store the data in respective maps
    foreach(auto networkReply, m_networkReplies)
    {
        // Paranoid safety checks
        if (networkReply.isNull())
        {
            continue;
        }
        if (networkReply->isRunning())
        {
            continue;
        }
        if (networkReply->error() != QNetworkReply::NoError)
        {
            networkReply->deleteLater();
            continue;
        }

        auto region = networkReply->property("area").value<QGeoCircle>();
        NotamList notamList(networkReply->readAll(), region);

        m_notamLists.prepend(notamList);
        m_lastUpdate = QDateTime::currentDateTimeUtc();
        emit lastUpdateChanged();
        emit waypointsChanged();
    }

    clearOldEntries();
    save();
}


QList<GeoMaps::Waypoint> NOTAM::NotamProvider::waypoints() const
{
    QList<GeoMaps::Waypoint> result;
    foreach (auto notamList, m_notamLists)
    {
        foreach (auto notam, notamList.m_notams)
        {
            result.append(GeoMaps::Waypoint(notam.m_coordinates));
        }
    }
    return result;
}


void NOTAM::NotamProvider::load()
{
    auto inputFile = QFile(stdFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QDataStream inputStream(&inputFile);
        inputStream >> m_notamLists;
    }
    clearOldEntries();
    save();

    emit lastUpdateChanged();
    emit waypointsChanged();
}

void NOTAM::NotamProvider::save() const
{
    auto outputFile = QFile(stdFileName);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        QDataStream outputStream(&outputFile);
        outputStream << m_notamLists;
    }
}

void NOTAM::NotamProvider::clearOldEntries()
{
    if (m_notamLists.isEmpty())
    {
        return;
    }


    QSet<QGeoCircle> seenCircles;
    bool haveChange = false;
    for(auto i = 0; i<m_notamLists.size(); i++)
    {
        if (m_notamLists[i].m_retrieved.secsTo(QDateTime::currentDateTimeUtc()) > 60*60*24)
        {
            qWarning() << "Delete NotamList, all from here";
            m_notamLists.remove(i, m_notamLists.size()-i);
            haveChange = true;
            continue;
        }
        if (seenCircles.contains(m_notamLists[i].m_region))
        {
            qWarning() << "Delete NotamList, circle known";
            m_notamLists.remove(i);
            haveChange = true;
            i--;
        }
        seenCircles += m_notamLists[i].m_region;

        m_notamLists[i].removeExpiredEntries();
    }

    if (m_notamLists.isEmpty()) {
        m_lastUpdate = {};
    }


    emit waypointsChanged();
        emit lastUpdateChanged();

}


void NOTAM::NotamProvider::startRequest(const GeoMaps::Waypoint& waypoint)
{
    // Otherwise, NOTAMS are not present and have not been requested,
    // so we need to request them now.
    qWarning() << "Request NOTAMs for " << waypoint.ICAOCode();

    auto urlString = u"https://external-api.faa.gov/notamapi/v1/notams?"
                     "locationLongitude=%1&locationLatitude=%2&locationRadius=%3"_qs
            .arg(waypoint.coordinate().longitude())
            .arg(waypoint.coordinate().latitude())
            .arg(1.2*requestRadius.toNM());
    QNetworkRequest request( urlString );
    request.setRawHeader("client_id", "bcd5e948c6654d3284ebeba68012a9eb");
    request.setRawHeader("client_secret", "1FCE4b44ED8f4C328C1b6341D5b1a55c");

    auto* reply = GlobalObject::networkAccessManager()->get(request);
    reply->setProperty("area", QVariant::fromValue(QGeoCircle(waypoint.coordinate(), requestRadius.toM())) );

    m_networkReplies.append(reply);
    connect(reply, &QNetworkReply::finished, this, &NOTAM::NotamProvider::downloadFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &NOTAM::NotamProvider::downloadFinished);


}


#include "positioning/PositionProvider.h"
#include <chrono>

using namespace std::chrono_literals;

void NOTAM::NotamProvider::autoUpdate()
{
    auto position = positionProvider()->lastValidCoordinate();

    // If we have a NOTAM list that contains the position
    // within half its radius, then stop.
    foreach (auto notamList, m_notamLists)
    {
        auto region = notamList.m_region;
        if (position.distanceTo(region.center()) < 0.5*region.radius())
        {
            return;
        }
    }

    // If we have a NOTAM list that contains the position
    // within half its radius and is less than 12h old, then stop.
    foreach (auto notamList, m_notamLists)
    {
        auto region = notamList.m_region;
        if ((position.distanceTo(region.center()) < 0.5*region.radius())
                && (notamList.m_retrieved.secsTo(QDateTime::currentDateTimeUtc())) < 60*60*12)
        {
            return;
        }
    }

    // If one pending request contains the position
    // within half its radius, then stop.
    foreach(auto networkReply, m_networkReplies)
    {
        // Paranoid safety checks
        if (networkReply.isNull())
        {
            continue;
        }

        auto region = networkReply->property("area").value<QGeoCircle>();
        if (!region.isValid())
        {
            continue;
        }
        if (position.distanceTo(region.center()) < 0.5*region.radius())
        {
            return;
        }
    }


    startRequest(position);
}

