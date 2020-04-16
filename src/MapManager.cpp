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
    QObject(parent),
    _maps_json(QUrl("https://cplx.vm.uni-freiburg.de/storage/enroute-GeoJSONv001/maps.json"),
                QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/maps.json",
                networkAccessManager),
    _networkAccessManager(networkAccessManager)
{
    QStringList offendingFiles;
    QDirIterator fileIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aviation_maps",
                              QDir::Files, QDirIterator::Subdirectories);
    while (fileIterator.hasNext()) {
        fileIterator.next();
        if (fileIterator.filePath().endsWith(".geojson.geojson") || fileIterator.filePath().endsWith(".mbtiles.mbtiles"))
            offendingFiles += fileIterator.filePath();
    }
    foreach(auto offendingFile, offendingFiles)
        QFile::rename(offendingFile, offendingFile.section('.', 0, -2));


    // Construct the Dowloadable object "_maps_json". Let it point to the remote file "maps.json" and wire it up.
    connect(&_maps_json, &Downloadable::downloadingChanged, this, &MapManager::downloadingGeoMapListChanged);
    connect(&_maps_json, &Downloadable::fileContentChanged, this, &MapManager::readGeoMapListFromJSONFile);
    connect(&_maps_json, &Downloadable::fileContentChanged, this, &MapManager::setTimeOfLastUpdateToNow);
    connect(&_maps_json, &Downloadable::error, this, &MapManager::errorReceiver);

    // Wire up the DownloadableGroup _geoMaps
    connect(&_geoMaps, &DownloadableGroup::downloadablesChanged, this, &MapManager::geoMapListChanged);
    connect(&_geoMaps, &DownloadableGroup::downloadingChanged, this, &MapManager::downloadingGeoMapsChanged);
    connect(&_geoMaps, &DownloadableGroup::filesChanged, this, &MapManager::localFileOfGeoMapChanged);
    connect(&_geoMaps, &DownloadableGroup::localFileContentChanged, this, &MapManager::geoMapFileContentChanged);

    // Wire up the DownloadableGroup _aviationMaps
    connect(&_aviationMaps, &DownloadableGroup::downloadablesChanged, this, &MapManager::aviationMapsChanged);

    // Wire up the DownloadableGroup _baseMaps
    connect(&_baseMaps, &DownloadableGroup::downloadablesChanged, this, &MapManager::baseMapsChanged);

    // Wire up the automatic update timer and check if automatic updates are
    // due. The method "autoUpdateGeoMapList" will also set a reasonable timeout
    // value for the timer and start it.
    connect(&_autoUpdateTimer, &QTimer::timeout, this, &MapManager::autoUpdateGeoMapList);
    autoUpdateGeoMapList();

    // If there is a downloaded maps.json file, we read it. Otherwise, we start a download.
    if (_maps_json.hasFile())
        readGeoMapListFromJSONFile();
    else
        _maps_json.startFileDownload();
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
        QDirIterator dirIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
				 "/aviation_maps", QDir::Dirs|QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
        while (dirIterator.hasNext()) {
            dirIterator.next();

            QDir dir(dirIterator.filePath());
            if (dir.isEmpty()) {
                dir.removeRecursively();
                didDelete = true;
            }
        }
    }while(didDelete);

    // Clear and delete everything
    foreach(auto geoMapPtr, _geoMaps.downloadables())
        delete geoMapPtr;
}


void MapManager::updateGeoMapList()
{
    QTimer::singleShot(0, &_maps_json, SLOT(startFileDownload()));
}


void MapManager::errorReceiver(const QString&, QString message)
{
    emit error(std::move(message));
}


void MapManager::localFileOfGeoMapChanged()
{
    auto oldGeoMapUpdatesAvailable = geoMapUpdatesAvailable();
    auto oldMbtileFiles = baseMapsWithFiles();

    // Ok, a local file changed. First, we check if this means that the local file
    // of an unsupported map (=map with invalid URL) is gone. These maps are then
    // no longer wanted. We go through the list and see if we can find any
    // candidates.
    auto geoMaps = _geoMaps.downloadables();
    foreach(auto geoMapPtr, geoMaps) {
        if (geoMapPtr->url().isValid())
            continue;
        if (geoMapPtr->hasFile())
            continue;

        // Ok, we found an unsupported map without local file. Let's get rid of
        // that.
        _geoMaps.removeFromGroup(geoMapPtr);
        geoMapPtr->deleteLater();
    }

    if (oldGeoMapUpdatesAvailable != geoMapUpdatesAvailable())
        emit geoMapUpdatesAvailableChanged();
#warning leave this to specialised DownloadableGroup
    if (oldMbtileFiles != baseMapsWithFiles())
        emit baseMapsWithFilesChanged();
    emit geoMapFilesChanged();
}


void MapManager::readGeoMapListFromJSONFile()
{
    if (!_maps_json.hasFile())
        return;

    bool old_aviationMapUpdatesAvailable = geoMapUpdatesAvailable();

    // List of maps as we have them now
    QList<QPointer<Downloadable>> oldMaps = _geoMaps.downloadables();

    // To begin, we handle the maps described in the maps.json file. If these maps
    // were already present in the old list, we re-use them. Otherwise, we create
    // new Downloadable objects.
    QJsonParseError parseError{};
    auto doc = QJsonDocument::fromJson(_maps_json.fileContent(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return;

    auto top = doc.object();
    auto baseURL = top.value("url").toString();

    foreach(auto map, top.value("maps").toArray()) {
        auto obj = map.toObject();
        auto mapFileName = obj.value("path").toString();
        auto mapName = mapFileName.section('.',-2,-2);
        auto mapUrlName = baseURL + "/"+ obj.value("path").toString();
        auto fileModificationDateTime = QDateTime::fromString(obj.value("time").toString(), "yyyyMMdd");
        auto fileSize = obj.value("size").toInt();

        // If a map with the given name already exists, update that element, delete its entry in oldMaps
        Downloadable *mapPtr = nullptr;
        foreach(auto geoMapPtr, oldMaps) {
            if (geoMapPtr->objectName() == mapName.section("/", -1 , -1)) {
                mapPtr = geoMapPtr;
                break;
            }
        }

        if (mapPtr) {
            // Map exists
            oldMaps.removeAll(mapPtr);
            mapPtr->setRemoteFileDate(fileModificationDateTime);
            mapPtr->setRemoteFileSize(fileSize);
        } else {
            // Construct local file name
            auto localFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/aviation_maps/"+mapFileName;

            // Construct a new downloadable object.
            auto downloadable = new Downloadable(QUrl(mapUrlName), localFileName, _networkAccessManager, this);
            downloadable->setObjectName(mapName.section("/", -1, -1));
            downloadable->setSection(mapName.section("/", -2, -2));
            downloadable->setRemoteFileDate(fileModificationDateTime);
            downloadable->setRemoteFileSize(fileSize);
            _geoMaps.addToGroup(downloadable);
            if (localFileName.endsWith("geojson"))
                _aviationMaps.addToGroup(downloadable);
            if (localFileName.endsWith("mbtiles"))
                _baseMaps.addToGroup(downloadable);
        }

    }

    // Now go through all the leftover objects in the old list of aviation
    // maps. These are now aviation maps that are no longer supported. If they
    // have no local file to them, we simply delete them.  If they have a local
    // file, we keep them, but set their QUrl to invalid; this will mark them as
    // unsupported in the GUI.
    foreach(auto geoMapPtr, oldMaps) {
        if (geoMapPtr->hasFile())
            continue;
        delete geoMapPtr;
    }

    // Now it is still possible that the download directory contains files beloning
    // to unsupported maps. Add those to newMaps.
    foreach(auto path, unattachedFiles()) {
        // Generate proper object name from path
        QString objectName = path;
        objectName = objectName.remove(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
				       "/aviation_maps/").section('.', 0, 0);

        auto downloadable = new Downloadable(QUrl(), path, _networkAccessManager, this);
        downloadable->setSection("Unsupported Maps");
        downloadable->setObjectName(objectName);
        _geoMaps.addToGroup(downloadable);
    }

    // Set the new maps and inform our users
    if (old_aviationMapUpdatesAvailable != geoMapUpdatesAvailable())
        emit geoMapUpdatesAvailableChanged();

    return;
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


void MapManager::autoUpdateGeoMapList()
{
    // If the last update is more than one day ago, automatically initiate an
    // update, so that maps stay at least roughly current.
    QSettings settings;
    QDateTime lastUpdate = settings.value("MapManager/MapListTimeStamp", QDateTime()).toDateTime();

    if (!lastUpdate.isValid() || (qAbs(lastUpdate.daysTo(QDateTime::currentDateTime()) > 6)) ) {
        // Updates are due. Check again in one hour if the update went well or if we need to try again.
        _autoUpdateTimer.start(1000*60*60*1);
        updateGeoMapList();
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
    QDirIterator fileIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aviation_maps",
			      QDir::Files, QDirIterator::Subdirectories);
    while (fileIterator.hasNext()) {
        fileIterator.next();

        // Now check if this file exists as the local file of some geographic map
        bool isAttachedToAviationMap = false;
        foreach(auto geoMapPtr, _geoMaps.downloadables()) {
            if (geoMapPtr->fileName() == QFileInfo(fileIterator.filePath()).absoluteFilePath()) {
                isAttachedToAviationMap = true;
                break;
            }
        }
        if (!isAttachedToAviationMap)
            result.append(fileIterator.filePath());
    }

    return result;
}
