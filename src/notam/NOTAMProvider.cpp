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
#include <QTimer>
#include <chrono>

#include "config.h"
#include "navigation/Navigator.h"
#include "notam/NOTAMProvider.h"
#include "positioning/PositionProvider.h"

using namespace std::chrono_literals;
using namespace Qt::Literals::StringLiterals;


//
// Constructor/Destructor
//

NOTAM::NOTAMProvider::NOTAMProvider(QObject* parent) :
    GlobalObject(parent)
{
}

void NOTAM::NOTAMProvider::deferredInitialization()
{
    // Load NOTAM data from file in stdFileName, then clean the data (which potentially triggers save()).
    QList<NOTAMList> newNotamLists;
    auto inputFile = QFile(m_stdFileName);
    if (inputFile.open(QIODevice::ReadOnly))
    {
        QDataStream inputStream(&inputFile);
        QString magicString;
        inputStream >> magicString;
        if (magicString == QStringLiteral(GIT_COMMIT))
        {
            inputStream >> m_readNotamNumbers;
            inputStream >> newNotamLists;
        }
    }
    m_notamLists = cleaned(newNotamLists);

    // Wire up updateData. Check NOTAM database after start, and whenever the flight route changes.
    QTimer::singleShot(0, this, &NOTAMProvider::updateData);
    connect(navigator()->flightRoute(), &Navigation::FlightRoute::waypointsChanged, this, &NOTAMProvider::updateData);
    connect(GlobalObject::positionProvider(), &Positioning::PositionProvider::approximateLastValidCoordinateChanged, this, &NOTAMProvider::updateData);

    // Clean and check the NOTAM database the data every hour.
    auto* timer = new QTimer(this);
    timer->start(59min);
    connect(timer, &QTimer::timeout, this, [this]() {
        updateData();
        m_notamLists = cleaned(m_notamLists);
    });


    // Setup Bindings
    m_controlPoints4FlightRoute.setBinding([this]() {return computeControlPoints4FlightRoute();});
    m_geoJSON.setBinding([this]() {return computeGeoJSON();});
    m_lastUpdate.setBinding([this]() {return computeLastUpdate();});
    m_status.setBinding([this]() {return computeStatus();});

    // Setup Notifiers
    // -- Save the NOTAM data every time that the database changes
    m_saveNotifier = m_notamLists.addNotifier([this]() {save();});
}

NOTAM::NOTAMProvider::~NOTAMProvider()
{
    for(const auto& networkReply : m_networkReplies)
    {
        if (networkReply.isNull())
        {
            continue;
        }
        disconnect(networkReply, nullptr, this, nullptr);
        networkReply->abort();
        delete networkReply;
    }
    m_networkReplies.clear();
}


//
// Methods
//

NOTAM::NOTAMList NOTAM::NOTAMProvider::notams(const GeoMaps::Waypoint& waypoint)
{
    // Paranoid safety checks
    if (!waypoint.coordinate().isValid())
    {
        return {};
    }

    // Check if notams for the location are present in our database.
    // Go through the database, oldest to newest.
    for(const auto& notamList : m_notamLists.value())
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

    // Check if internet requests NOTAMs for the location are pending.
    // In that case, return an empty list.
    for(const auto& networkReply : m_networkReplies)
    {
        // Paranoid safety checks
        if (networkReply.isNull())
        {
            continue;
        }
        if (!networkReply->isRunning())
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

void NOTAM::NOTAMProvider::setRead(const QString& number, bool read)
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
// Private Methods
//

QList<NOTAM::NOTAMList> NOTAM::NOTAMProvider::cleaned(const QList<NOTAMList>& notamLists, const QSet<QString>& cancelledNotams)
{
    QList<NOTAMList> newNotamLists;
    QSet<QGeoCircle> regionsSeen;

    // Iterate over notamLists, newest lists first
    for(const auto& notamList : notamLists)
    {
        // If this notamList is outdated, then so all all further ones. We can thus end here.
        if (notamList.isOutdated())
        {
            break;
        }

        // If a newer notamList has the same region, then the data in this list
        // is irrelevant. Skip over this list.
        if (regionsSeen.contains(notamList.region()))
        {
            continue;
        }

        regionsSeen += notamList.region();

        auto cleanedList = notamList.cleaned(cancelledNotams);
        newNotamLists.append(cleanedList);
    }

    return newNotamLists;
}

void NOTAM::NOTAMProvider::downloadFinished()
{
    auto newNotamList = m_notamLists.value();
    QSet<QString> cancelledNotams;

    m_networkReplies.removeAll(nullptr);
    for(const auto& networkReply : m_networkReplies)
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
            // Network error? Then try again in 5 minutes.
            QTimer::singleShot(5min, this, &NOTAMProvider::updateData);
            networkReply->deleteLater();
            continue;
        }

        auto region = networkReply->property("area").value<QGeoCircle>();
        auto data = networkReply->readAll();
        networkReply->deleteLater();

        auto jsonDoc = QJsonDocument::fromJson(data);
        if (jsonDoc.isNull())
        {
            continue;
        }
        NOTAMList const notamList(jsonDoc, region, &cancelledNotams);
        newNotamList.prepend(notamList);
    }
    m_notamLists = cleaned(newNotamList, cancelledNotams);
}

bool NOTAM::NOTAMProvider::hasDataForPosition(const QGeoCoordinate& position, bool includeDataThatNeedsUpdate, bool includeRunningDownloads) const
{
    if (!position.isValid())
    {
        return true;
    }

    for(const auto& notamList : m_notamLists.value())
    {
        if (notamList.isOutdated())
        {
            continue;
        }
        if (notamList.needsUpdate() && !includeDataThatNeedsUpdate)
        {
            continue;
        }

        const auto region = notamList.region();
        if (!region.isValid())
        {
            continue;
        }
        if (region.radius() - region.center().distanceTo(position) >= minimumRadiusPoint.toM())
        {
            return true;
        }
    }

    if (includeRunningDownloads)
    {
        for(const auto& networkReply : m_networkReplies)
        {
            // Paranoid safety checks
            if (networkReply.isNull())
            {
                continue;
            }
            if (!networkReply->isRunning())
            {
                continue;
            }
            auto region = networkReply->property("area").value<QGeoCircle>();
            if (region.radius() - region.center().distanceTo(position) >= minimumRadiusPoint.toM())
            {
                return true;
            }
        }
    }

    return false;
}

void NOTAM::NOTAMProvider::save() const
{
    auto outputFile = QFile(m_stdFileName);
    if (outputFile.open(QIODevice::WriteOnly))
    {
        QDataStream outputStream(&outputFile);
        outputStream << QStringLiteral(GIT_COMMIT);
        outputStream << m_readNotamNumbers;
        outputStream << m_notamLists.value();
    }
}

void NOTAM::NOTAMProvider::startRequest(const QGeoCoordinate& coordinate)
{
    if (!coordinate.isValid())
    {
        return;
    }

    // If data exists for marginRadius around that coordinate or if that data is already
    // requested for download, then return immediately.
    if (hasDataForPosition(coordinate, false, true))
    {
        return;
    }

    const QGeoCoordinate coordinateRounded( qRound(coordinate.latitude()), qRound(coordinate.longitude()) );
    auto urlString = u"https://enroute-data.akaflieg-freiburg.de/enrouteProxy/notam.php?"
                     "locationLongitude=%1&"
                     "locationLatitude=%2&"
                     "locationRadius=%3&"
                     "pageSize=1000"_s
                         .arg(coordinateRounded.longitude())
                         .arg(coordinateRounded.latitude())
                         .arg( qRound(requestRadius.toNM()) );
    QNetworkRequest const request(urlString);

    auto* reply = GlobalObject::networkAccessManager()->get(request);
    reply->setProperty("area", QVariant::fromValue(QGeoCircle(coordinateRounded, requestRadius.toM())) );

    connect(reply, &QNetworkReply::finished, this, &NOTAMProvider::downloadFinished);
    connect(reply, &QNetworkReply::errorOccurred, this, &NOTAMProvider::downloadFinished);

    m_networkReplies.append(reply);
}

void NOTAM::NOTAMProvider::updateData()
{
    startRequest(GlobalObject::positionProvider()->approximateLastValidCoordinate());
    for(const auto& pos : m_controlPoints4FlightRoute.value())
    {
        startRequest(pos);
    }
}


//
// Private Members and Member Computing Methods
//

QList<QGeoCoordinate> NOTAM::NOTAMProvider::computeControlPoints4FlightRoute()
{
    auto* route = navigator()->flightRoute();
    if (route == nullptr)
    {
        return {};
    }

    auto minDistControlPoints = (minimumRadiusPoint-minimumRadiusFlightRoute)*2.0;

    QList<QGeoCoordinate> result;
    result += route->geoPath();
    for (const auto& leg : route->legs())
    {
        if (leg.distance() > maximumFlightRouteLegLength)
        {
            continue;
        }

        auto startCoordinate = leg.startPoint().coordinate();
        auto endCoordinate = leg.endPoint().coordinate();
        while (startCoordinate.distanceTo(endCoordinate) > minDistControlPoints.toM())
        {
            auto azimuth = startCoordinate.azimuthTo(endCoordinate);
            startCoordinate = startCoordinate.atDistanceAndAzimuth(minDistControlPoints.toM(), azimuth);
            result += startCoordinate;
        }
    }

    return result;
}

QByteArray NOTAM::NOTAMProvider::computeGeoJSON() const
{
    QList<QJsonObject> result;
    QSet<QGeoCoordinate> coordinatesSeen;

    for(const auto& notamList : m_notamLists.value())
    {
        for(const auto& notam : notamList.notams())
        {
            auto coordinate = notam.coordinate();
            if (!coordinate.isValid())
            {
                continue;
            }

            // If we already have a waypoint for that coordinate, then don't add another one.
            if (coordinatesSeen.contains(coordinate))
            {
                continue;
            }
            coordinatesSeen += coordinate;
            result.append(notam.GeoJSON());
        }
    }

    QJsonArray waypointArray;
    for(const auto& jsonObject : result)
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

QDateTime NOTAM::NOTAMProvider::computeLastUpdate() const
{
    auto notamLists = m_notamLists.value();
    if (notamLists.isEmpty())
    {
        return {};
    }
    return notamLists[0].retrieved();
}

QString NOTAM::NOTAMProvider::computeStatus() const
{
    if (!hasDataForPosition(GlobalObject::positionProvider()->approximateLastValidCoordinate(), true, false))
    {
        return tr("NOTAMs not current around own position, requesting update");
    }

    for(const auto& pos : m_controlPoints4FlightRoute.value())
    {
        if (!hasDataForPosition(pos, true, false))
        {
            return tr("NOTAMs not current around waypoint, requesting update");
        }
    }
    return {};
}






//
// Private Methods
//

