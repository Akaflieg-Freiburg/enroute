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

#include "geomaps/Waypoint.h"
#include "notam/NotamProvider.h"


NOTAM::NotamProvider::NotamProvider(QObject* parent) : GlobalObject(parent)
{
/*    auto stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/notam.dat";
    auto inputFile = QFile(stdFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QDataStream inputStream(&inputFile);
        inputStream >> m_notamLists;
    }
    */
}


void NOTAM::NotamProvider::deferredInitialization()
{

}


NOTAM::NotamList NOTAM::NotamProvider::notams(const GeoMaps::Waypoint& waypoint)
{
    NotamList result;

    // Check if notams for the location are present in our database
    foreach (auto notamList, m_notamLists)
    {
        if (notamList.covers(waypoint))
        {
            return notamList.restrict(waypoint);
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

        foreach(auto notam, notamList.m_notams)
        {
            qWarning() << region.center().distanceTo(notam.m_coordinates) << notam.m_text;
        }

        m_notamLists.prepend(notamList);
        m_lastUpdate = QDateTime::currentDateTimeUtc();
        emit lastUpdateChanged();
        emit waypointsChanged();
    }

    auto stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/notam.dat";
    auto outputFile = QFile(stdFileName);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        QDataStream outputStream(&outputFile);
        outputStream << m_notamLists;
    }

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
