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

#warning Calling MapManager::updateGeoMapList() from MapManager.qml / Flick causes infrequent crashes. Need to investigate before release


MapManager::MapManager(QNetworkAccessManager *networkAccessManager, QObject *parent) :
    QObject(parent), _networkAccessManager(networkAccessManager)
{
    // Construct the Dowloadable object "_maps_json". Let it point to the remote file "maps.json" and wire it up.
    _maps_json = new Downloadable(QUrl("https://cplx.vm.uni-freiburg.de/storage/enroute-GeoJSONv001/maps.json"),
                                  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/maps.json",
                                  networkAccessManager,
                                  this);
    _maps_json->setObjectName(tr("list of aviation maps"));
    connect(_maps_json, &Downloadable::downloadingChanged, this, &MapManager::downloadingGeoMapListChanged);
    connect(_maps_json, &Downloadable::localFileContentChanged, this, &MapManager::readGeoMapListFromJSONFile);
    connect(_maps_json, &Downloadable::localFileContentChanged, this, &MapManager::setTimeOfLastUpdateToNow);
    connect(_maps_json, &Downloadable::error, this, &MapManager::errorReceiver);

    // Wire up the DownloadableGroup _geoMaps
    connect(&_geoMaps, &DownloadableGroup::downloadablesChanged, this, &MapManager::geoMapListChanged);


    // Wire up the automatic update timer and check if automatic updates are due. The method "autoUpdateGeoMapList" will also set a reasonable timeout value for the timer and start it.
    connect(&_autoUpdateTimer, &QTimer::timeout, this, &MapManager::autoUpdateGeoMapList);
    autoUpdateGeoMapList();

    // If there is a downloaded maps.json file, we read it. Otherwise, we start a download.
    if (_maps_json->hasLocalFile())
        readGeoMapListFromJSONFile();
    else
        _maps_json->startFileDownload();
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

    // Clear and delete everything
    foreach(auto geoMapPtr, _geoMaps.downloadables())
        delete geoMapPtr;
    delete _maps_json;
}


QString MapManager::geoMapUpdateSize() const
{
    qint64 downloadSize = 0;
    foreach(auto geoMapPtr, _geoMaps.downloadables())
        if (geoMapPtr->updatable())
            downloadSize += geoMapPtr->remoteFileSize();

    return QLocale::system().formattedDataSize(downloadSize, 1, QLocale::DataSizeSIFormat);
}


QList<Downloadable*> MapManager::aviationMaps() const
{
    QList<Downloadable *> result;

    foreach(auto geoMapPtr, _geoMaps.downloadables()) {
        if (!geoMapPtr->fileName().endsWith(".geojson", Qt::CaseInsensitive))
            continue;
        result += geoMapPtr;
    }

    return result;
}


QList<QObject*> MapManager::aviationMapsAsObjectList() const
{
    QList<QObject*> result;
    foreach(auto geoMapPtr, _geoMaps.downloadables()) {
        if (!geoMapPtr->fileName().endsWith(".geojson", Qt::CaseInsensitive))
            continue;
        result.append(geoMapPtr);
    }
    qWarning() << "MapManager::aviationMapsAsObjectList()" << result;
    return result;
}


QList<Downloadable*> MapManager::baseMaps() const
{
    QList<Downloadable *> result;

    foreach(auto geoMapPtr, _geoMaps.downloadables()) {
        if (!geoMapPtr->fileName().endsWith(".mbtiles", Qt::CaseInsensitive))
            continue;
        result += geoMapPtr;
    }

    return result;
}


QList<QObject*> MapManager::baseMapsAsObjectList() const
{
    QList<QObject*> result;
    foreach(auto geoMapPtr, _geoMaps.downloadables()) {
        if (!geoMapPtr->fileName().endsWith(".mbtiles", Qt::CaseInsensitive))
            continue;
        result.append(geoMapPtr);
    }
    return result;
}


bool MapManager::downloadingGeoMapList() const
{
    // Paranoid safety checks
    Q_ASSERT(!_maps_json.isNull());
    if (_maps_json.isNull())
        return false;

    return _maps_json->downloading();
}


bool MapManager::hasAviationMap() const
{
    foreach(auto geoMapPtr, _geoMaps.downloadables()) {
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
    foreach(auto geoMapPtr, _geoMaps.downloadables()) {
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

    foreach(auto geoMapPtr, _geoMaps.downloadables()) {
        // Ignore everything but geojson files
        if (!geoMapPtr->fileName().endsWith(".mbtiles", Qt::CaseInsensitive))
            continue;
        if (!geoMapPtr->hasLocalFile())
            continue;

        result += geoMapPtr->fileName();
    }

    return result;
}


void MapManager::updateGeoMapList()
{
    qWarning() << "MapManager::updateGeoMapList()";

    // Paranoid safety checks
    Q_ASSERT(!_maps_json.isNull());
    if (_maps_json.isNull())
        return;

    QTimer::singleShot(0, _maps_json, SLOT(startFileDownload()));
}


void MapManager::updateGeoMaps()
{
    foreach(auto geoMapPtr, _geoMaps.downloadables())
        if (geoMapPtr->updatable())
            geoMapPtr->startFileDownload();
}


void MapManager::errorReceiver(const QString&, QString message)
{
    emit error(std::move(message));
}


void MapManager::localFileOfGeoMapChanged()
{
    auto oldGeoMapUpdatesAvailable = geoMapUpdatesAvailable();
    auto oldMbtileFiles = mbtileFiles();

    // Ok, a local file changed. First, we check if this means that the local file
    // of an unsupported map (=map with invalid URL) is gone. These maps are then
    // no longer wanted. We go through the list and see if we can find any
    // candidates.
    auto geoMaps = _geoMaps.downloadables();
    foreach(auto geoMapPtr, geoMaps) {
        if (geoMapPtr->url().isValid())
            continue;
        if (geoMapPtr->hasLocalFile())
            continue;

        // Ok, we found an unsupported map without local file. Let's get rid of
        // that.
        _geoMaps.removeFromGroup(geoMapPtr);
        geoMapPtr->deleteLater();
    }

    if (oldGeoMapUpdatesAvailable != geoMapUpdatesAvailable())
        emit geoMapUpdatesAvailableChanged();
    if (oldMbtileFiles != mbtileFiles())
        emit mbtileFilesChanged(mbtileFiles(), "osm");
}



void MapManager::readGeoMapListFromJSONFile()
{
    qWarning() << "MapManager::readGeoMapListFromJSONFile()";

    // Paranoid safety checks
    Q_ASSERT(!_maps_json.isNull());
    if (_maps_json.isNull())
        return;
    if (!_maps_json->hasLocalFile())
        return;

    bool old_aviationMapUpdatesAvailable = geoMapUpdatesAvailable();

    // This is the central object that will replace _aviationMaps at the end.
    QList<QPointer<Downloadable>> newMaps;

    // Alert all users that the list of maps is in an intermediate stage and that
    // it should not be used for the moment
    emit aboutToChangeAviationMapList();

    // To begin, we handle the maps described in the maps.json file. If these maps
    // were already present in the old list, we re-use them. Otherwise, we create
    // new Downloadable objects.
    QJsonParseError parseError{};
    auto doc = QJsonDocument::fromJson(_maps_json->localFileContent(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return;

    auto top = doc.object();
    auto baseURL = top.value("url").toString();

    qWarning() << "A";

    foreach(auto map, top.value("maps").toArray()) {
        auto obj = map.toObject();
        auto mapFileName = obj.value("path").toString();
        auto mapName = mapFileName.section('.',-2,-2);
        auto mapUrlName = baseURL + "/"+ obj.value("path").toString();
        auto fileModificationDateTime = QDateTime::fromString(obj.value("time").toString(), "yyyyMMdd");
        auto fileSize = obj.value("size").toInt();

        // If a map with the given name already exists in _geoMaps, delete it.
        foreach(auto geoMapPtr, _geoMaps.downloadables()) {
            if (geoMapPtr->objectName() == mapFileName)
                delete geoMapPtr;
        }

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
        connect(downloadable, &Downloadable::localFileContentChanged, this, &MapManager::localFileOfGeoMapChanged);
        newMaps += downloadable;
    }

    qWarning() << "B";

    // Now go through all the leftover objects in the old list of aviation
    // maps. These are now aviation maps that are no longer supported. If they
    // have no local file to them, we simply delete them.  If they have a local
    // file, we keep them, but set their QUrl to invalid; this will mark them as
    // unsupported in the GUI.
    foreach(auto geoMapPtr, _geoMaps.downloadables()) {
        if (!geoMapPtr->hasLocalFile())
            continue;

        auto downloadable = new Downloadable(QUrl(), geoMapPtr->fileName(), _networkAccessManager, this);
        downloadable->setObjectName(geoMapPtr->objectName());
        connect(downloadable, &Downloadable::localFileContentChanged, this, &MapManager::localFileOfGeoMapChanged);
        newMaps += downloadable;
    }
    qWarning() << "B1";

    // Delete the unused aviation maps, and set to new
    qDeleteAll(_geoMaps.downloadables());
    qWarning() << "B2";
    foreach(auto _geoMapPtr, newMaps) {
        qWarning() << "B2a";
        _geoMaps.addToGroup(_geoMapPtr);
    }

    qWarning() << "C";

    // Now it is still possible that the download directory contains files beloning
    // to unsupported maps. Add those to newMaps.
    foreach(auto path, unattachedFiles()) {
        // Generate proper object name from path
        QString objectName = path;
        objectName = objectName.remove(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aviation_maps/").section('.', 0, 0);

        auto downloadable = new Downloadable(QUrl(), path, _networkAccessManager, this);
        downloadable->setObjectName(objectName);
        connect(downloadable, &Downloadable::localFileContentChanged, this, &MapManager::localFileOfGeoMapChanged);
        _geoMaps.addToGroup(downloadable);
    }

    qWarning() << "D";

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
    QDirIterator fileIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/aviation_maps", QDir::Files, QDirIterator::Subdirectories);
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
