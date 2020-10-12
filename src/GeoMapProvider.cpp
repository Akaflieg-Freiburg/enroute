/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#include <QApplication>
#include <QtConcurrent/QtConcurrent>
#include <QGeoCoordinate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlEngine>
#include <QRandomGenerator>

#include "Clock.h"
#include "GeoMapProvider.h"
#include "Waypoint.h"


GeoMapProvider::GeoMapProvider(MapManager *manager, GlobalSettings* globalSettings, Librarian *librarian, QObject *parent)
    : QObject(parent),
      _globalSettings(globalSettings),
      _manager(manager),
      _librarian(librarian),
      _tileServer(QUrl()),
      _styleFile(nullptr)
{
    // Initialize _combinedGeoJSON_ with an empty document
    QJsonObject resultObject;
    resultObject.insert("type", "FeatureCollection");
    resultObject.insert("features", QJsonArray());
    QJsonDocument geoDoc(resultObject);
    _combinedGeoJSON_ = geoDoc.toJson(QJsonDocument::JsonFormat::Compact);

    _tileServer.listen(QHostAddress("127.0.0.1"));
}


QList<QObject*> GeoMapProvider::airspaces(const QGeoCoordinate& position)
{
    // Lock data
    QMutexLocker lock(&_aviationDataMutex);

    QList<Airspace*> result;
    foreach(auto airspace, _airspaces_) {
        if (airspace.isNull()) // Paranoid safety
            continue;
        if (airspace->polygon().contains(position))
            result.append(airspace);
    }

    // Sort airspaces according to lower boundary
    std::sort(result.begin(), result.end(), [](Airspace* a, Airspace* b) {return (a->estimatedLowerBoundInFtMSL() > b->estimatedLowerBoundInFtMSL()); });

    QList<QObject*> final;
    foreach(auto airspace, result)
        final.append(airspace);

    return final;
}


QObject* GeoMapProvider::closestWaypoint(QGeoCoordinate position, const QGeoCoordinate& distPosition)
{
    position.setAltitude(qQNaN());

    auto wps = waypoints();
    if (wps.isEmpty())
        return nullptr;

    auto result = wps[0];
    foreach(auto wp, wps) {
        if (wp.isNull())
            continue;
        if (position.distanceTo(wp->coordinate()) < position.distanceTo(result->coordinate()))
            result = wp;
    }

    if (position.distanceTo(result->coordinate()) > position.distanceTo(distPosition)) {
        auto wp = new Waypoint(position, this);
        return wp;
    }

    return result;
}


QList<QObject*> GeoMapProvider::filteredWaypointObjects(const QString &filter)
{
    auto wps = waypoints();

    QStringList filterWords;
    foreach(auto word, filter.simplified().split(' ', Qt::SkipEmptyParts)) {
        QString simplifiedWord = _librarian->simplifySpecialChars(word);
        if (simplifiedWord.isEmpty())
            continue;
        filterWords.append(simplifiedWord);
    }

    QList<QObject*> result;
    foreach(auto wp, wps) {
        if (wp.isNull())
            continue;
        bool allWordsFound = true;
        foreach(auto word, filterWords) {
            QString fullName = _librarian->simplifySpecialChars(wp->getPropery("NAM").toString());
            QString codeName = _librarian->simplifySpecialChars(wp->getPropery("COD").toString());
            QString wordx = _librarian->simplifySpecialChars(word);

            if (!fullName.contains(wordx, Qt::CaseInsensitive) && !codeName.contains(wordx, Qt::CaseInsensitive)) {
                allWordsFound = false;
                break;
            }
        }
        if (allWordsFound)
            result.append(wp);
    }

    return result;
}


Waypoint* GeoMapProvider::findByID(const QString &id)
{
    auto wps = waypoints();

    foreach(auto wp, wps) {
        if (wp.isNull())
            continue;
        if (wp->getPropery("COD").toString() == id)
            return wp;
    }
    return nullptr;
}


QList<QObject*> GeoMapProvider::nearbyWaypoints(const QGeoCoordinate& position, const QString& type)
{
    auto wps = waypoints();

    QList<Waypoint*> tWps;
    foreach(auto wp, wps) {
        if (wp.isNull())
            continue;
        if (wp->getPropery("TYP").toString() != type)
            continue;
        tWps.append(wp);
    }

    std::sort(tWps.begin(), tWps.end(), [position](Waypoint* a, Waypoint* b) {return position.distanceTo(a->coordinate()) < position.distanceTo(b->coordinate()); });

    QList<QObject*> result;
    int sz = 0;
    foreach(auto ad, tWps) {
        result.append(ad);
        sz++;
        if (sz == 20)
            break;
    }

    return result;
}


QString GeoMapProvider::styleFileURL() const
{
    if (_styleFile.isNull())
        return ":/flightMap/empty.json";
    return "file://"+_styleFile->fileName();
}


void GeoMapProvider::aviationMapsChanged()
{
    // Paranoid safety checks
    if (_manager.isNull())
        return;
    if (_aviationDataCacheFuture.isRunning()) {
        _aviationDataCacheTimer.start();
        return;
    }

    //
    // Generate new GeoJSON array and new list of waypoints
    //
    QStringList JSONFileNames;
    foreach(auto geoMapPtr, _manager->aviationMaps()->downloadables()) {
        // Ignore everything but geojson files
        if (!geoMapPtr->fileName().endsWith(".geojson", Qt::CaseInsensitive))
            continue;
        if (!geoMapPtr->hasFile())
            continue;
        JSONFileNames += geoMapPtr->fileName();
    }

    _aviationDataCacheFuture = QtConcurrent::run(this, &GeoMapProvider::fillAviationDataCache, JSONFileNames, _globalSettings->hideUpperAirspaces());
}


void GeoMapProvider::baseMapsChanged()
{
    // Paranoid safety checks
    if (_manager.isNull())
        return;

    // Delete old style file, stop serving tiles
    delete _styleFile;
    _tileServer.removeMbtilesFileSet(_currentPath);

    // Serve new tile set under new name
    _currentPath = QString::number(QRandomGenerator::global()->bounded(static_cast<quint32>(1000000000)));
    _tileServer.addMbtilesFileSet(_manager->baseMaps()->downloadablesWithFile(), _currentPath);

    // Generate new mapbox style file
    _styleFile = new QTemporaryFile(this);
    QFile file(":/flightMap/osm-liberty.json");
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    data.replace("%URL%", (_tileServer.serverUrl()+"/"+_currentPath).toLatin1());
    data.replace("%URL2%", _tileServer.serverUrl().toLatin1());
    _styleFile->open();
    _styleFile->write(data);
    _styleFile->close();

    emit styleFileURLChanged();
}


void GeoMapProvider::fillAviationDataCache(const QStringList& JSONFileNames, bool hideUpperAirspaces)
{
    //
    // Generate new GeoJSON array and new list of waypoints
    //

    // First, create a set of JSON objects, in order to avoid duplicated entries
    QSet<QJsonObject> objectSet;
    foreach(auto JSONFileName, JSONFileNames) {
        // Read the lock file
        QLockFile lockFile(JSONFileName+".lock");
        lockFile.lock();
        QFile file(JSONFileName);
        file.open(QIODevice::ReadOnly);
        auto document = QJsonDocument::fromJson(file.readAll());
        file.close();
        lockFile.unlock();

        foreach(auto value, document.object()["features"].toArray()) {
            auto object = value.toObject();

            // If 'hideUpperAirspaces' is set, ignore all objects that are airspaces
            // and that begin at FL100 or above.
            if (hideUpperAirspaces) {
                Airspace airspaceTest(object);
                if (airspaceTest.isUpper())
                    continue;
            }
            objectSet += object;
        }
    }


    // Then, create a new JSONArray of features and a new list of waypoints
    QJsonArray newFeatures;
    QList<QPointer<Airspace>> newAirspaces;
    QList<QPointer<Waypoint>> newWaypoints;
    foreach(auto object, objectSet) {
        newFeatures += object;

        // Check if the current object is a waypoint. If so, add it to the list of waypoints.
        auto wp = new Waypoint(object);
        wp->moveToThread(QApplication::instance()->thread());
        wp->setParent(this);
        wp->setMeteorologist(_meteorologist);
        if (wp->isValid()) {
            QQmlEngine::setObjectOwnership(wp, QQmlEngine::CppOwnership);
            newWaypoints.append(wp);
            continue;
        }
        delete wp;

        // Check if the current object is an airspace. If so, add it to the list of airspaces.
        auto as = new Airspace(object);
        as->moveToThread(QApplication::instance()->thread());
        as->setParent(this);
        if (as->isValid()) {
            QQmlEngine::setObjectOwnership(as, QQmlEngine::CppOwnership);
            newAirspaces.append(as);
            continue;
        }
        delete as;
    }
    QJsonObject resultObject;
    resultObject.insert("type", "FeatureCollection");
    resultObject.insert("features", newFeatures);
    QJsonDocument geoDoc(resultObject);

    // Sort waypoints by name
    std::sort(newWaypoints.begin(), newWaypoints.end(), [](Waypoint* a, Waypoint* b) {return a->getPropery("NAM").toString() < b->getPropery("NAM").toString(); });

    _aviationDataMutex.lock();
    foreach(auto airspace, _airspaces_) {
        if (airspace.isNull())
            continue;
        airspace->deleteLater();
    }
    foreach(auto waypoint, _waypoints_) {
        if (waypoint.isNull())
            continue;
        waypoint->deleteLater();
    }
    _airspaces_ = newAirspaces;
    _waypoints_ = newWaypoints;
    _combinedGeoJSON_ = geoDoc.toJson(QJsonDocument::JsonFormat::Compact);
    _aviationDataMutex.unlock();

    emit geoJSONChanged();
}


void GeoMapProvider::setMeteorologist(Meteorologist *meteorologist)
{
    if (meteorologist == nullptr)
        return;
    _meteorologist = meteorologist;


    // Connect the Downloadmanager, so aviation maps will be generated
    connect(_manager->aviationMaps(), &DownloadableGroup::localFileContentChanged_delayed, this, &GeoMapProvider::aviationMapsChanged);
    connect(_manager->baseMaps(), &DownloadableGroup::localFileContentChanged_delayed, this, &GeoMapProvider::baseMapsChanged);
    connect(_globalSettings, &GlobalSettings::hideUpperAirspacesChanged, this, &GeoMapProvider::aviationMapsChanged);

    _aviationDataCacheTimer.setSingleShot(true);
    _aviationDataCacheTimer.setInterval(3*1000);
    connect(&_aviationDataCacheTimer, &QTimer::timeout, this, &GeoMapProvider::aviationMapsChanged);

    aviationMapsChanged();
    baseMapsChanged();
}
