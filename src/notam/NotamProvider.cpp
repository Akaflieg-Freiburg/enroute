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
#include <QStandardPaths>
#include <QTimer>
#include <chrono>

#include "navigation/Navigator.h"
#include "notam/NotamProvider.h"
#include "positioning/PositionProvider.h"

using namespace std::chrono_literals;



//
// Constructor/Destructor
//

NOTAM::NotamProvider::NotamProvider(QObject* parent) :
    GlobalObject(parent)
{
    // Set stdFileName for saving and loading NOTAM data
    m_stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+u"/notam.dat"_qs;

    // Load NOTAM data from file in stdFileName, clean data and save.
    auto inputFile = QFile(m_stdFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QDataStream inputStream(&inputFile);
        QString magicString;
        inputStream >> magicString;
        if (magicString == QStringLiteral(GIT_COMMIT))
        {
            inputStream >> m_notamLists;
        }
    }
    clean();
    save();
}


void NOTAM::NotamProvider::deferredInitialization()
{
    // Wire up autoUpdate. Check NOTAM database every 10 seconds after start, every 11 minutes, and whenever the flight route changes.
    auto* timer = new QTimer(this);
    timer->start(10min);
    connect(timer, &QTimer::timeout, this, &NOTAM::NotamProvider::updateData);
    connect(navigator()->flightRoute(), &Navigation::FlightRoute::waypointsChanged, this, &NOTAM::NotamProvider::updateData);
    QTimer::singleShot(10s, this, &NOTAM::NotamProvider::updateData);

    // Save the NOTAM data every time that the database changes
    connect(this, &NOTAM::NotamProvider::waypointsChanged, this, &NOTAM::NotamProvider::save, Qt::QueuedConnection);
}


NOTAM::NotamProvider::~NotamProvider()
{
    m_notamLists.clear();
    m_networkReplies.clear();
}



//
// Getter Methods
//

QList<GeoMaps::Waypoint> NOTAM::NotamProvider::waypoints() const
{
    QList<GeoMaps::Waypoint> result;
    QSet<QGeoCoordinate> coordinatesSeen;
    QSet<QGeoCircle> regionsCovered;

    foreach(auto notamList, m_notamLists)
    {
        foreach(auto notam, notamList.notams())
        {
            auto coordinate = notam.coordinate();

            // If we already have a waypoint for that coordinate, then don't add another one.
            if (coordinatesSeen.contains(coordinate))
            {
                continue;
            }

            // If the coordinate has already been handled by an earlier (=newer) notamList,
            // then don't add it here.
            bool hasBeenCovered = false;
            foreach(auto region, regionsCovered)
            {
                if (region.contains(coordinate))
                {
                    hasBeenCovered = true;
                    break;
                }
            }
            if (hasBeenCovered)
            {
                continue;
            }

            coordinatesSeen += coordinate;
            result.append(coordinate);
        }

        regionsCovered += notamList.region();
    }
    return result;
}



//
// Methods
//

NOTAM::NotamList NOTAM::NotamProvider::notams(const GeoMaps::Waypoint& waypoint)
{
    // Paranoid safety checks
    if (!waypoint.coordinate().isValid())
    {
        return {};
    }

    // Check if notams for the location are present in our database.
    // Go through the database, oldest to newest.
    foreach (auto notamList, m_notamLists)
    {
        // Disregard outdated notamLists
        if (notamList.isOutdated())
        {
            continue;
        }

        if (notamList.region().contains(waypoint.coordinate()))
        {
            // If motamList needs an update, then ask for an update
            if (notamList.needsUpdate())
            {
                startRequest(waypoint);
            }
            return notamList.restricted(waypoint);
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

    // We have no data for the waypoint and no pending internet requests. So,
    // start a new internet request and return an empty list.
    startRequest(waypoint);
    return {};
}



//
// Private Slots
//

void NOTAM::NotamProvider::downloadFinished()
{

    bool newDataAdded = false;
    m_networkReplies.removeAll(nullptr);
    auto networkReplies = m_networkReplies; // Make copy to avoid iteration while container changes
    foreach(auto networkReply, networkReplies)
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
        m_networkReplies.removeAll(networkReply);
        if (networkReply->error() != QNetworkReply::NoError)
        {
            continue;
        }

        auto region = networkReply->property("area").value<QGeoCircle>();
        NotamList notamList(networkReply->readAll(), region);
        m_notamLists.prepend(notamList);
        newDataAdded = true;
    }

    if (newDataAdded)
    {
        clean();
        m_lastUpdate = QDateTime::currentDateTimeUtc();
        emit lastUpdateChanged();
        emit waypointsChanged();
    }
}


void NOTAM::NotamProvider::save() const
{
    auto outputFile = QFile(m_stdFileName);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        QDataStream outputStream(&outputFile);
        outputStream << QStringLiteral(GIT_COMMIT);
        outputStream << m_notamLists;
    }
}


void NOTAM::NotamProvider::updateData()
{
    // Check if Notam data is available for a circle of marginRadius around
    // the current position.
    auto position = positionProvider()->lastValidCoordinate();
    if (position.isValid())
    {
        auto _range = range(position);
        if (_range < marginRadius)
        {
            startRequest(position);
        }
    }

    auto* route = navigator()->flightRoute();
    if (route != nullptr)
    {
        foreach(auto leg, route->legs())
        {
            QGeoCoordinate startPoint = leg.startPoint().coordinate();
            QGeoCoordinate endPoint = leg.endPoint().coordinate();
            if (!startPoint.isValid() || !endPoint.isValid())
            {
                continue;
            }

            while(true)
            {
                // Check if the range at the startPoint at least marginRadius+1NM
                // If not, request data for startPoint and start over
                auto rangeAtStartPoint = range(startPoint);
                if (rangeAtStartPoint < marginRadius+Units::Distance::fromNM(1))
                {
                    startRequest(startPoint);
                    continue;
                }

                // Check if every point between startPoint and endPoint has a range
                // of at least marginRadius. If so, there is nothing to do for this
                // and we continue with the next leg.
                auto distanceToEndPoint = Units::Distance::fromM(startPoint.distanceTo(endPoint));
                if (rangeAtStartPoint > distanceToEndPoint+marginRadius)
                {
                    break;
                }

                // Move the startPoint closer to the endPoint, so all points between
                // the new and the old startPoint have a range of at least
                // marginRadius. Then start over.
                auto azimuth = startPoint.azimuthTo(endPoint);
                startPoint = startPoint.atDistanceAndAzimuth((rangeAtStartPoint-marginRadius).toM(), azimuth);
            }
        }
    }

}

// --------------------------

void NOTAM::NotamProvider::clean()
{
    if (m_notamLists.isEmpty())
    {
        return;
    }


    QSet<QGeoCircle> seenCircles;
    for(auto i=0; i<m_notamLists.size(); i++)
    {
        if (m_notamLists[i].retrieved().secsTo(QDateTime::currentDateTimeUtc()) > 60*60*24)
        {
            qWarning() << "Delete NotamList, all from here";
            m_notamLists.remove(i, m_notamLists.size()-i);
            continue;
        }
        if (seenCircles.contains(m_notamLists[i].region()))
        {
            qWarning() << "Delete NotamList, circle known" << m_notamLists[i].region().center();
            m_notamLists.remove(i);
            i--;
        }
        seenCircles += m_notamLists[i].region();

        m_notamLists[i] = m_notamLists[i].cleaned();
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
    qWarning() << "Request NOTAMs for " << waypoint.extendedName();
    auto urlString = u"https://external-api.faa.gov/notamapi/v1/notams?"
                     "locationLongitude=%1&locationLatitude=%2&locationRadius=%3"_qs
            .arg(waypoint.coordinate().longitude())
            .arg(waypoint.coordinate().latitude())
            .arg(1.2*requestRadius.toNM());
    QNetworkRequest request( urlString );
    request.setRawHeader("client_id", "bcd5e948c6654d3284ebeba68012a9eb");
    request.setRawHeader("client_secret", "1FCE4b44ED8f4C328C1b6341D5b1a55c");

    QSharedPointer<QNetworkReply> reply(GlobalObject::networkAccessManager()->get(request));
    reply->setProperty("area", QVariant::fromValue(QGeoCircle(waypoint.coordinate(), requestRadius.toM())) );

    m_networkReplies.append(reply);
    connect(reply.data(), &QNetworkReply::finished, this, &NOTAM::NotamProvider::downloadFinished);
    connect(reply.data(), &QNetworkReply::errorOccurred, this, &NOTAM::NotamProvider::downloadFinished);
}



Units::Distance NOTAM::NotamProvider::range(const QGeoCoordinate& position)
{
    auto result = Units::Distance::fromM(-1.0);

    if (!position.isValid())
    {
        return result;
    }

    // If we have a NOTAM list that contains the position
    // within half its radius, then stop.
    foreach (auto notamList, m_notamLists)
    {
        if (notamList.isOutdated())
        {
            continue;
        }

        auto region = notamList.region();
        auto rangeInM = region.radius() - region.center().distanceTo(position);
        result = qMax(result, Units::Distance::fromM(rangeInM));
    }

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
        auto rangeInM = region.radius() - region.center().distanceTo(position);
        result = qMax(result, Units::Distance::fromM(rangeInM));
    }

    return result;
}
