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

#include <QDirIterator>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QStandardPaths>
#include <utility> 
#include "MapManager.h"

MapManager::MapManager(QNetworkAccessManager *networkAccessManager, QObject *parent) :
    QObject(parent), _networkAccessManager(networkAccessManager)
{
    // Construct the Dowloadable object "_availableMapsDescription". Let it
    // point to the remote file "maps.json" and wire it up.
    _availableMapsDescription = new Downloadable(QUrl("https://cplx.vm.uni-freiburg.de/storage/enroute-GeoJSONv001/maps.json"), QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/maps.json", networkAccessManager, this);
    _availableMapsDescription->setObjectName(tr("list of aviation maps"));
    connect(_availableMapsDescription, &Downloadable::downloadingChanged, this, &MapManager::downloadingChanged);
    connect(_availableMapsDescription, &Downloadable::localFileChanged, this, &MapManager::readMapListFromDownloadedJSONFile);
    connect(_availableMapsDescription, &Downloadable::localFileChanged, this, &MapManager::readMapListFromDownloadedJSONFile);
    connect(_availableMapsDescription, &Downloadable::localFileChanged, this, &MapManager::setTimeOfLastUpdateToNow);
    connect(_availableMapsDescription, &Downloadable::error, this, &MapManager::errorReceiver);

    // Set up the automatic update timer, and check if automatic updates are due
    connect(&_autoUpdateTimer, &QTimer::timeout, this, &MapManager::startUpdateIfAutoUpdateIsDue);
    startUpdateIfAutoUpdateIsDue();

    // Now that this map manager exists, try to obtain a list of maps. We try a
    // few things. First, we try to rebuild our list from the downloaded txt
    // file.

    // If there is no downloaded txt file, then we start a download, in
    // order to obtain one.
    if (!_availableMapsDescription->hasLocalFile()) {
        _availableMapsDescription->startFileDownload();
        return;
    }

    // If there is a downloaded txt file, interpret that.
    readMapListFromDownloadedJSONFile();
}


MapManager::~MapManager()
{
    // It might be possible for whatever reason that our download directory
    // contains files that we do not know whom they belong to. We hunt down those
    // files and silently delete them.
    foreach(auto path, unattachedFiles())
        QFile::remove(path);

    // It might be possible that our download directory contains empty
    // subdirectories. We we remove them all.
    bool didDelete;
    do{
        didDelete = false;
        QDirIterator dirIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aviation_maps", QDir::Dirs|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (dirIterator.hasNext()) {
            dirIterator.next();

            QDir dir(dirIterator.filePath());
            if (dir.isEmpty()) {
                dir.removeRecursively();
                didDelete = true;
            }
        }
    }while(didDelete);
}


bool MapManager::geoMapUpdatesAvailable() const
{
    foreach(auto geoMapPtr, _geoMaps)
        if (geoMapPtr->updatable())
            return true;
    return false;
}


QString MapManager::geoMapUpdateSize() const
{
    qint64 downloadSize = 0;
    foreach(auto geoMapPtr, _geoMaps)
        if (geoMapPtr->updatable())
            downloadSize += geoMapPtr->remoteFileSize();

    return QLocale::system().formattedDataSize(downloadSize, 1, QLocale::DataSizeSIFormat);
}


QMap<QString, Downloadable*> MapManager::aviationMaps() const
{
    QMap<QString, Downloadable *> result;

    QMapIterator<QString, Downloadable *> i(_geoMaps);
    while (i.hasNext()) {
        i.next();
        if (!i.value()->fileName().endsWith(".geojson", Qt::CaseInsensitive))
            continue;
        result.insert(i.key(), i.value());
    }

    return result;
}


QMap<QString, Downloadable*> MapManager::baseMaps() const
{
    QMap<QString, Downloadable *> result;

    QMapIterator<QString, Downloadable *> i(_geoMaps);
    while (i.hasNext()) {
        i.next();
        if (!i.value()->fileName().endsWith(".mbtiles", Qt::CaseInsensitive))
            continue;
        result.insert(i.key(), i.value());
    }

    return result;
}


QList<QObject*> MapManager::aviationMapsAsObjectList() const
{
    QList<QObject*> result;
    foreach(auto geoMapPtr, _geoMaps) {
        if (!geoMapPtr->fileName().endsWith(".geojson", Qt::CaseInsensitive))
            continue;
        result.append(geoMapPtr);
    }
    return result;
}


QList<QObject*> MapManager::baseMapsAsObjectList() const
{
    QList<QObject*> result;
    foreach(auto geoMapPtr, _geoMaps) {
        if (!geoMapPtr->fileName().endsWith(".mbtiles", Qt::CaseInsensitive))
            continue;
        result.append(geoMapPtr);
    }
    return result;
}


bool MapManager::downloading() const
{
    // Paranoid safety checks
    Q_ASSERT(!_availableMapsDescription.isNull());
    if (_availableMapsDescription.isNull())
        return false;

    return _availableMapsDescription->downloading();
}


bool MapManager::hasAviationMap() const
{
    foreach(auto geoMapPtr, _geoMaps) {
        // Ignore everything but geojson files
        if (!geoMapPtr->fileName().endsWith(".geojson", Qt::CaseInsensitive))
            continue;
        if (geoMapPtr->hasLocalFile())
            return true;
    }
    return false;
}


bool MapManager::hasBaseMap() const
{
    foreach(auto geoMapPtr, _geoMaps) {
        // Ignore everything but geojson files
        if (!geoMapPtr->fileName().endsWith(".mbtiles", Qt::CaseInsensitive))
            continue;
        if (geoMapPtr->hasLocalFile())
            return true;
    }
    return false;
}


QSet<QString> MapManager::mbtileFiles() const
{
    QSet<QString> result;

    foreach(auto geoMapPtr, _geoMaps) {
        // Ignore everything but geojson files
        if (!geoMapPtr->fileName().endsWith(".mbtiles", Qt::CaseInsensitive))
            continue;
        if (!geoMapPtr->hasLocalFile())
            continue;

        result += geoMapPtr->fileName();
    }

    return result;
}


void MapManager::startUpdate()
{
    // Paranoid safety checks
    Q_ASSERT(!_availableMapsDescription.isNull());
    if (_availableMapsDescription.isNull())
        return;

    _availableMapsDescription->startFileDownload();
}


void MapManager::startMapUpdates()
{
    foreach(auto geoMapPtr, _geoMaps)
        if (geoMapPtr->updatable())
            geoMapPtr->startFileDownload();
}


void MapManager::errorReceiver(const QString&, QString message)
{
    emit error(std::move(message));
}


void MapManager::localFileOfGeoMapChanged()
{
    // Ok, a local file changed. First, we check if this means that the local file
    // of an unsupported map (=map with invalid URL) is gone. These maps are then
    // no longer wanted. We go through the list and see if we can find any
    // candidates.
    foreach(auto geoMapPtr, _geoMaps) {
        if (geoMapPtr->url().isValid())
            continue;
        if (geoMapPtr->hasLocalFile())
            continue;

        // Ok, we found an unsupported map without local file. Let's get rid of
        // that.
        _geoMaps.take(geoMapPtr->objectName())->deleteLater();
    }

    emit geoMapsChanged();
    emit geoMapUpdatesAvailableChanged();
    emit mbtileFilesChanged(mbtileFiles(), "osm");
}


bool MapManager::readMapListFromDownloadedJSONFile()
{
    // Paranoid safety checks
    Q_ASSERT(!_availableMapsDescription.isNull());
    if (_availableMapsDescription.isNull())
        return false;

    if (!_availableMapsDescription->hasLocalFile())
        return false;

    bool old_aviationMapUpdatesAvailable = geoMapUpdatesAvailable();

    // This is the central object that will replace _aviationMaps at the end.
    QMap<QString, Downloadable *> newMaps;

    // Alert all users that the list of maps is in an intermediate stage and that
    // it should not be used for the moment
    emit aboutToChangeAviationMapList();

    // To begin, we handle the maps described in the maps.json file. If these maps
    // were already present in the old list, we re-use them. Otherwise, we create
    // new Downloadable objects.
    QJsonParseError parseError{};
    auto doc = QJsonDocument::fromJson(_availableMapsDescription->localFileContent(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return false;

    auto top = doc.object();
    auto baseURL = top.value("url").toString();

    foreach(auto map, top.value("maps").toArray()) {
        auto obj = map.toObject();
        auto mapFileName = obj.value("path").toString();
        auto mapName = mapFileName.section('.',-2,-2);
        auto mapUrlName = baseURL + "/"+ obj.value("path").toString();
        auto fileModificationDateTime = QDateTime::fromString(obj.value("time").toString(), "yyyyMMdd");
        auto fileSize = obj.value("size").toInt();

        // If a map with the given name already exists in _aviationMaps, delete it.
        if (_geoMaps.contains(mapFileName))
            delete _geoMaps.take(mapFileName);

        // Construct local file name
        auto ending = mapUrlName.section(".", -1);
        auto localFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aviation_maps/"+mapFileName;
        if (!ending.isEmpty())
            localFileName += "."+ending;

        // Construct a new downloadable object.
        auto downloadable = new Downloadable(QUrl(mapUrlName), localFileName, _networkAccessManager, this);
        downloadable->setObjectName(mapName.section("/", -1 , -1));
        downloadable->setRemoteFileDate(fileModificationDateTime);
        downloadable->setRemoteFileSize(fileSize);
        connect(downloadable, &Downloadable::localFileChanged, this, &MapManager::localFileOfGeoMapChanged);
#warning A
//        connect(downloadable, &Downloadable::downloadingChanged, this, &MapManager::localFileOfGeoMapChanged);
        newMaps.insert(mapFileName, downloadable);
    }

    // Now go through all the leftover objects in the old list of aviation
    // maps. These are now aviation maps that are no longer supported. If they
    // have no local file to them, we simply delete them.  If they have a local
    // file, we keep them, but set their QUrl to invalid; this will mark them as
    // unsupported in the GUI.
    foreach(auto geoMapPtr, _geoMaps) {
        if (!geoMapPtr->hasLocalFile())
            continue;

        auto downloadable = new Downloadable(QUrl(), geoMapPtr->fileName(), _networkAccessManager, this);
        downloadable->setObjectName(geoMapPtr->objectName());
        connect(downloadable, &Downloadable::localFileChanged, this, &MapManager::localFileOfGeoMapChanged);
#warning B
        //        connect(downloadable, &Downloadable::downloadingChanged, this, &MapManager::localFileOfGeoMapChanged);
        newMaps.insert(geoMapPtr->objectName(), downloadable);
    }
    // Delete the unused aviation maps, and set to new
    qDeleteAll(_geoMaps);
    _geoMaps.clear();
    _geoMaps = newMaps;

    // Now it is still possible that the download directory contains files beloning
    // to unsupported maps. Add those to newMaps.
    foreach(auto path, unattachedFiles()) {
        // Generate proper object name from path
        QString objectName = path;
        objectName = objectName.remove(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aviation_maps/").section('.', 0, 0);

        auto downloadable = new Downloadable(QUrl(), path, _networkAccessManager, this);
        downloadable->setObjectName(objectName);
        connect(downloadable, &Downloadable::localFileChanged, this, &MapManager::localFileOfGeoMapChanged);
#warning C
        //        connect(downloadable, &Downloadable::downloadingChanged, this, &MapManager::localFileOfGeoMapChanged);
        _geoMaps.insert(objectName, downloadable);
    }

    // Set the new maps and inform our users
    if (old_aviationMapUpdatesAvailable != geoMapUpdatesAvailable())
        emit geoMapUpdatesAvailableChanged();

    return true;
}


void MapManager::setTimeOfLastUpdateToNow()
{
    // Save timestamp, so that we know when an automatic update is due
    QSettings settings;
    settings.setValue("MapManager/MapListTimeStamp", QDateTime::currentDateTimeUtc());

    // Now that we downloaded successfully, we need to check for updates only once
    // a day
    _autoUpdateTimer.start(1000*60*60*24);
}


void MapManager::startUpdateIfAutoUpdateIsDue()
{
    // If the last update is more than one day ago, automatically initiate an
    // update, so that maps stay at least roughly current.
    QSettings settings;
    QDateTime lastUpdate = settings.value("MapManager/MapListTimeStamp", QDateTime()).toDateTime();

    if (!lastUpdate.isValid() || (qAbs(lastUpdate.daysTo(QDateTime::currentDateTime()) > 0)) ) {
        // Updates are due. Check once per hour for updates until update succeeds.
        _autoUpdateTimer.start(1000*60*60*1);
        startUpdate();
        return;
    }

    // Updates are not yet due. Check again in one day.
    _autoUpdateTimer.start(1000*60*60*24);
}


QList<QString> MapManager::unattachedFiles() const
{
    QList<QString> result;

    // It might be possible for whatever reason that our download directory
    // contains files that we do not know whom they belong to. We hunt down those
    // files.
    QDirIterator fileIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aviation_maps", QDir::Files, QDirIterator::Subdirectories);
    while (fileIterator.hasNext()) {
        fileIterator.next();

        // Now check if this file exists as the local file of some aviation map
        bool isAttachedToAviationMap = false;
        foreach(auto geoMapPtr, _geoMaps) {
            if (geoMapPtr->fileInfo() == QFileInfo(fileIterator.filePath()) ) {
                isAttachedToAviationMap = true;
                break;
            }
        }
        if (!isAttachedToAviationMap)
            result.append(fileIterator.filePath());
    }

    return result;
}


