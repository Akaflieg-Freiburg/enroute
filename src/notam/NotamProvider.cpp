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

#include <QFile>
#include <QJsonArray>
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
}


void NOTAM::NotamProvider::deferredInitialization()
{
    // Wire up updateData. Check NOTAM database every 10 seconds after start, every 11 minutes, and whenever the flight route changes.
    auto* timer = new QTimer(this);
    timer->start(11min);
    connect(timer, &QTimer::timeout, this, &NOTAM::NotamProvider::updateData);
    connect(navigator()->flightRoute(), &Navigation::FlightRoute::waypointsChanged, this, &NOTAM::NotamProvider::updateData);
    QTimer::singleShot(10s, this, &NOTAM::NotamProvider::updateData);

    // Wire up clean(). Clean the data every 61 minutes.
    timer = new QTimer(this);
    timer->start(61min);
    connect(timer, &QTimer::timeout, this, &NOTAM::NotamProvider::clean);

    // Save the NOTAM data every time that the database changes
    connect(this, &NOTAM::NotamProvider::dataChanged, this, &NOTAM::NotamProvider::save, Qt::QueuedConnection);

    // Load NOTAM data from file in stdFileName, then clean the data (which potentially triggers save()).
    auto inputFile = QFile(m_stdFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QDataStream inputStream(&inputFile);
        QString magicString;
        inputStream >> magicString;
        if (magicString == QStringLiteral(GIT_COMMIT))
        {
            inputStream >> m_readNotamNumbers;
            inputStream >> m_notamLists;
        }
    }
    clean();
}


NOTAM::NotamProvider::~NotamProvider()
{
    foreach(auto networkReply, m_networkReplies)
    {
        if (networkReply.isNull())
        {
            continue;
        }
        networkReply->abort();
    }

    m_notamLists.clear();
    m_networkReplies.clear();
}



//
// Getter Methods
//

QByteArray NOTAM::NotamProvider::GeoJSON() const
{
    QList<QJsonObject> result;
    QSet<QGeoCoordinate> coordinatesSeen;
    QSet<QGeoCircle> regionsCovered;

    foreach(auto notamList, m_notamLists)
    {
        foreach(auto notam, notamList.notams())
        {
            if (!notam.isValid() || notam.isOutdated())
            {
                continue;
            }
            auto coordinate = notam.coordinate();
            if (!coordinate.isValid())
            {
                continue;
            }
            if (!notamList.region().contains(coordinate))
            {
                continue;
            }


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
            result.append(notam.GeoJSON());
        }

        regionsCovered += notamList.region();
    }


    QJsonArray waypointArray;
    foreach (const auto& jsonObject, result)
    {
        waypointArray.append(jsonObject);
    }

    QJsonObject jsonObj;
    jsonObj.insert(QStringLiteral("type"), "FeatureCollection");
    jsonObj.insert(QStringLiteral("features"), waypointArray);

    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc.toJson();
}


QList<GeoMaps::Waypoint> NOTAM::NotamProvider::waypoints() const
{
    QList<GeoMaps::Waypoint> result;
    QSet<QGeoCoordinate> coordinatesSeen;
    QSet<QGeoCircle> regionsCovered;

    foreach(auto notamList, m_notamLists)
    {
        foreach(auto notam, notamList.notams())
        {
            if (!notam.isValid() || notam.isOutdated())
            {
                continue;
            }
            auto coordinate = notam.coordinate();
            if (!coordinate.isValid())
            {
                continue;
            }
            if (!notamList.region().contains(coordinate))
            {
                continue;
            }


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
                startRequest(waypoint.coordinate());
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
    startRequest(waypoint.coordinate());
    return {};
}


void NOTAM::NotamProvider::setRead(const QString& number, bool read)
{
    if (read)
    {
        m_readNotamNumbers.prepend(number);
        if (m_readNotamNumbers.size() > 50)
        {
            m_readNotamNumbers.remove(50);
        }
    }
    else
    {
        m_readNotamNumbers.removeAll(number);
    }
    save();
}



//
// Private Slots
//

void NOTAM::NotamProvider::clean()
{

    QList<NotamList> newNotamLists;
    QSet<QGeoCircle> regionsSeen;
    bool haveChange = false;

    // Iterate over notamLists, newest lists first
    foreach(auto notamList, m_notamLists)
    {
        // If this notamList is outdated, then so all all further ones. We can thus end here.
        if (notamList.isOutdated())
        {
            haveChange = true;
            break;
        }

        // If a newer notamList has the same region, then the data in this list
        // is irrelevant. Skip over this list.
        if (regionsSeen.contains(notamList.region()))
        {
            haveChange = true;
            continue;
        }

        regionsSeen += notamList.region();

        auto cleanedList = notamList.cleaned(m_cancelledNotamNumbers);
        if (cleanedList.notams().size() != notamList.notams().size())
        {
            haveChange = true;
        }
        newNotamLists.append(cleanedList);
    }
    m_cancelledNotamNumbers.clear();

    if (haveChange)
    {
        m_notamLists = newNotamLists;
        emit dataChanged();
    }
}


void NOTAM::NotamProvider::downloadFinished()
{

    bool newDataAdded = false;
    m_networkReplies.removeAll(nullptr);
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
        if (!networkReply->isFinished())
        {
            continue;
        }
        if (networkReply->error() != QNetworkReply::NoError)
        {
            qWarning() << "FAA NOTAM Server returned with an error." << networkReply->error();
            networkReply->deleteLater();
            continue;
        }

        auto region = networkReply->property("area").value<QGeoCircle>();
        auto data = networkReply->readAll();
        networkReply->deleteLater();

        auto jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull())
        {
            qWarning() << u"FAA NOTAM Server returned with invalid or empty JSON data."_qs;
            continue;
        }
        NotamList const notamList(jsonDoc, region, &m_cancelledNotamNumbers);
        qWarning() << u"FAA NOTAM Server returned with %1 NOTAMs."_qs.arg(notamList.notams().size());
        m_notamLists.prepend(notamList);
        newDataAdded = true;
    }

    if (newDataAdded)
    {
        clean();
        m_lastUpdate = QDateTime::currentDateTimeUtc();
        emit dataChanged();
    }
}


void NOTAM::NotamProvider::save() const
{
    auto outputFile = QFile(m_stdFileName);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        QDataStream outputStream(&outputFile);
        outputStream << QStringLiteral(GIT_COMMIT);
        outputStream << m_readNotamNumbers;
        outputStream << m_notamLists;
    }
}


void NOTAM::NotamProvider::updateData()
{
    // Check if Notam data is available for a circle of marginRadius around
    // the current position.
    auto position = Positioning::PositionProvider::lastValidCoordinate();
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
            QGeoCoordinate const startPoint = leg.startPoint().coordinate();
            if (startPoint.isValid())
            {
                auto _range = range(startPoint);
                if (_range < marginRadius)
                {
                    startRequest(startPoint);
                }
            }

            QGeoCoordinate const endPoint = leg.endPoint().coordinate();
            if (endPoint.isValid())
            {
                auto _range = range(endPoint);
                if (_range < marginRadius)
                {
                    startRequest(endPoint);
                }
            }

/*
            QGeoCoordinate startPoint = leg.startPoint().coordinate();
            QGeoCoordinate const endPoint = leg.endPoint().coordinate();
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
*/
        }
    }

}



//
// Private Methods
//

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
        if (!region.isValid())
        {
            continue;
        }
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


void NOTAM::NotamProvider::startRequest(const QGeoCoordinate& coordinate)
{
    if (!coordinate.isValid())
    {
        return;
    }
    const QGeoCoordinate coordinateRounded( qRound(coordinate.latitude()), qRound(coordinate.longitude()) );

    auto urlString = u"https://enroute-data.akaflieg-freiburg.de/enrouteProxy/notam.php?"
                     "locationLongitude=%1&"
                     "locationLatitude=%2&"
                     "locationRadius=%3&"
                     "pageSize=1000"_qs
                         .arg(coordinateRounded.longitude())
                         .arg(coordinateRounded.latitude())
                         .arg( qRound(1.2*requestRadius.toNM()) );
    qWarning() << "NOTAM::NotamProvider::startRequest" << urlString;
    QNetworkRequest const request(urlString);

    auto* reply = GlobalObject::networkAccessManager()->get(request);
    reply->setProperty("area", QVariant::fromValue(QGeoCircle(coordinateRounded, requestRadius.toM())) );

    m_networkReplies.append(reply);
    connect(reply, &QNetworkReply::finished, this, &NOTAM::NotamProvider::downloadFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &NOTAM::NotamProvider::downloadFinished);
}
