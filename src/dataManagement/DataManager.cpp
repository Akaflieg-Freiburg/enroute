/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

#include <chrono>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QLockFile>
#include <QStandardPaths>

#include "dataManagement/UpdateNotifier.h"
#include "dataManagement/DataManager.h"
#include "geomaps/MBTILES.h"
#include "Settings.h"

using namespace std::chrono_literals;

DataManagement::DataManager::DataManager(QObject *parent) : GlobalObject(parent),
    _maps_json(QUrl(QStringLiteral("https://cplx.vm.uni-freiburg.de/storage/enroute-GeoJSONv003/maps.json")),
               QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/maps.json", this)
{
    // Clean the data directory.
    //
    // - delete all files with unexpected file names
    //
    // - earlier versions of this program constructed files with names ending in
    //   ".geojson.geojson" or ".mbtiles.mbtiles". We correct those file names
    //   here.
    //
    // - remove all empty sub directories
    {
        QStringList misnamedFiles;
        QStringList unexpectedFiles;
        QDirIterator fileIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/aviation_maps",
                                  QDir::Files, QDirIterator::Subdirectories);
        while (fileIterator.hasNext())
        {
            fileIterator.next();
            if (fileIterator.filePath().endsWith(QLatin1String(".geojson.geojson")) || fileIterator.filePath().endsWith(QLatin1String(".mbtiles.mbtiles")))
            {
                misnamedFiles += fileIterator.filePath();
            }
            if (!fileIterator.filePath().endsWith(QLatin1String(".geojson")) &&
                    !fileIterator.filePath().endsWith(QLatin1String(".mbtiles")))
            {
                unexpectedFiles += fileIterator.filePath();
            }
        }
        foreach (auto misnamedFile, misnamedFiles)
            QFile::rename(misnamedFile, misnamedFile.section('.', 0, -2));
        foreach (auto unexpectedFile, unexpectedFiles)
            QFile::remove(unexpectedFile);

        // delete all empty directories
        bool didDelete = false;
        do
        {
            didDelete = false;
            QDirIterator dirIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
                                     "/aviation_maps",
                                     QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
            while (dirIterator.hasNext())
            {
                dirIterator.next();

                QDir dir(dirIterator.filePath());
                if (dir.isEmpty())
                {
                    dir.removeRecursively();
                    didDelete = true;
                }
            }
        } while (didDelete);
    }

    // Construct the Dowloadable object "_maps_json". Let it point to the remote
    // file "maps.json" and wire it up.
    connect(&_maps_json, &DataManagement::Downloadable::downloadingChanged, this, &DataManager::downloadingRemoteItemListChanged);
    connect(&_maps_json, &DataManagement::Downloadable::fileContentChanged, this, &DataManager::readGeoMapListFromJSONFile);
    connect(&_maps_json, &DataManagement::Downloadable::fileContentChanged, this, &DataManager::setTimeOfLastUpdateToNow);
    connect(&_maps_json, &DataManagement::Downloadable::error, this, &DataManager::errorReceiver);

    // Wire up the DownloadableGroup _items
    connect(&_items, &DataManagement::DownloadableGroup::downloadablesChanged, this, &DataManager::geoMapListChanged);
    connect(&_items, &DataManagement::DownloadableGroup::filesChanged, this, &DataManager::localFileOfGeoMapChanged);

    // If there is a downloaded maps.json file, we read it.
    readGeoMapListFromJSONFile();
}


void DataManagement::DataManager::deferredInitialization()
{

    // Wire up the automatic update timer and check if automatic updates are
    // due. The method "autoUpdateGeoMapList" will also set a reasonable timeout
    // value for the timer and start it.
    connect(&_autoUpdateTimer, &QTimer::timeout, this, &DataManager::autoUpdateGeoMapList);
    connect(GlobalObject::settings(), &Settings::acceptedTermsChanged, this, &DataManager::updateRemoteDataItemList);
    if (GlobalObject::settings()->acceptedTerms() != 0)
    {

        autoUpdateGeoMapList();

        // If there is no downloaded maps.json file, be sure to start a download.
        if (!_maps_json.hasFile() && (GlobalObject::settings()->acceptedTerms() != 0))
        {
            _maps_json.startFileDownload();
        }
    }

    // Set up and start the updateNotifier
    new DataManagement::UpdateNotifier(this);
}


auto DataManagement::DataManager::describeDataItem(const QString& fileName) -> QString
{
    QFileInfo fi(fileName);
    if (!fi.exists())
    {
        return tr("No information available.");
    }
    QString result = QStringLiteral("<table><tr><td><strong>%1 :&nbsp;&nbsp;</strong></td><td>%2</td></tr><tr><td><strong>%3 :&nbsp;&nbsp;</strong></td><td>%4</td></tr></table>")
            .arg(tr("Installed"),
                 fi.lastModified().toUTC().toString(),
                 tr("File Size"),
                 QLocale::system().formattedDataSize(fi.size(), 1, QLocale::DataSizeSIFormat));

    // Extract infomation from GeoJSON
    if (fileName.endsWith(u".geojson"))
    {
        QLockFile lockFile(fileName + ".lock");
        lockFile.lock();
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        auto document = QJsonDocument::fromJson(file.readAll());
        file.close();
        lockFile.unlock();
        QString concatInfoString = document.object()[QStringLiteral("info")].toString();
        if (!concatInfoString.isEmpty())
        {
            result += "<p>" + tr("The map data was compiled from the following sources.") + "</p><ul>";
            auto infoStrings = concatInfoString.split(QStringLiteral(";"));
            foreach (auto infoString, infoStrings)
                result += "<li>" + infoString + "</li>";
            result += u"</ul>";
        }
    }

    // Extract infomation from MBTILES
    if (fileName.endsWith(u".mbtiles"))
    {
        result += GeoMaps::MBTILES::info(fileName);
    }

    // Extract infomation from text file - this is simply the first line
    if (fileName.endsWith(u".txt"))
    {
        // Open file and read first line
        QFile dataFile(fileName);
        dataFile.open(QIODevice::ReadOnly);
        auto description = dataFile.readLine();
        result += QStringLiteral("<p>%1</p>").arg(QString::fromLatin1(description));
    }

    return result;
}

void DataManagement::DataManager::updateRemoteDataItemList()
{
    _maps_json.startFileDownload();
}

void DataManagement::DataManager::errorReceiver(const QString& /*unused*/, QString message)
{
    emit error(std::move(message));
}

void DataManagement::DataManager::localFileOfGeoMapChanged()
{
    // Ok, a local file changed. First, we check if this means that the local
    // file of an unsupported map (=map with invalid URL) is gone. These maps
    // are then no longer wanted. We go through the list and see if we can find
    // any candidates.
    auto items = _items.downloadables();
    foreach (auto geoMapPtr, items)
    {
        if (geoMapPtr->url().isValid())
        {
            continue;
        }
        if (geoMapPtr->hasFile())
        {
            continue;
        }

        // Ok, we found an unsupported map without local file. Let's get rid of
        // that.
        _items.removeFromGroup(geoMapPtr);
        geoMapPtr->deleteLater();
    }
}

DataManagement::Downloadable* DataManagement::DataManager::createOrRecycleItem(const QUrl& url, const QString& localFileName)
{
    // If a data item with the given local file name and the given URL already exists,
    // update that remoteFileDate and remoteFileSize of that element, annd delete its
    // entry in oldMaps
    foreach (auto mapPtr, _items.downloadables())
    {
        if (mapPtr.isNull())
        {
            continue;
        }
        if ((mapPtr->fileName() == localFileName) && (mapPtr->url() == url))
        {
            return mapPtr;
        }
    }

    // Construct a new downloadable object and add to appropriate groups
    auto *downloadable = new DataManagement::Downloadable(url, localFileName, this);
    _items.addToGroup(downloadable);
    if (localFileName.endsWith(QLatin1String("geojson")))
    {
        _aviationMaps.addToGroup(downloadable);
    }
    if (localFileName.endsWith(QLatin1String("mbtiles")))
    {
        _baseMaps.addToGroup(downloadable);
    }
    if (localFileName.endsWith(QLatin1String("txt")))
    {
        _databases.addToGroup(downloadable);
    }
    return downloadable;
}


void DataManagement::DataManager::readGeoMapListFromJSONFile()
{
    if (!_maps_json.hasFile())
    {
        return;
    }

    // Get List of file in the directory
    QList<QString> files;
    QDirIterator fileIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/aviation_maps",
                              QDir::Files, QDirIterator::Subdirectories);
    while (fileIterator.hasNext())
    {
        fileIterator.next();
        files.append(fileIterator.filePath());
    }


    // List of maps as we have them now
    auto oldMaps = _items.downloadables();

    // To begin, we handle the maps described in the maps.json file. If these
    // maps were already present in the old list, we re-use them. Otherwise, we
    // create new Downloadable objects.
    QJsonParseError parseError{};
    auto doc = QJsonDocument::fromJson(_maps_json.fileContent(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        return;
    }

    auto top = doc.object();
    auto baseURL = top.value(QStringLiteral("url")).toString();

    foreach (auto map, top.value(QStringLiteral("maps")).toArray())
    {
        auto obj = map.toObject();
        auto mapFileName = obj.value(QStringLiteral("path")).toString();
        auto localFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/aviation_maps/" + mapFileName;
        auto mapName = mapFileName.section('.', -2, -2);
        auto mapUrlName = baseURL + "/" + obj.value(QStringLiteral("path")).toString();
        QUrl mapUrl(mapUrlName);
        auto fileModificationDateTime = QDateTime::fromString(obj.value(QStringLiteral("time")).toString(), QStringLiteral("yyyyMMdd"));
        auto fileSize = obj.value(QStringLiteral("size")).toInt();

        auto* downloadable = createOrRecycleItem(mapUrl, localFileName);
        oldMaps.removeAll(downloadable);
        downloadable->setRemoteFileDate(fileModificationDateTime);
        downloadable->setRemoteFileSize(fileSize);
        downloadable->setObjectName(mapName.section(QStringLiteral("/"), -1, -1));
        downloadable->setSection(mapName.section(QStringLiteral("/"), -2, -2));

        files.removeAll(localFileName);
    }


    foreach(auto localFileName, files)
    {
        auto* downloadable = createOrRecycleItem(QUrl(), localFileName);
        oldMaps.removeAll(downloadable);
        downloadable->setObjectName(localFileName.section(QStringLiteral("/"), -1, -1));
        downloadable->setSection(tr("Unsupported"));
    }

    qDeleteAll(oldMaps);

    // Update the whatsNew property
    auto newWhatsNew = top.value(QStringLiteral("whatsNew")).toString();
    if (!newWhatsNew.isEmpty() && (newWhatsNew != _whatsNew))
    {
        _whatsNew = newWhatsNew;
        emit whatsNewChanged();
    }
}


void DataManagement::DataManager::setTimeOfLastUpdateToNow()
{
    // Save timestamp, so that we know when an automatic update is due
    QSettings settings;
    settings.setValue(QStringLiteral("DataManager/MapListTimeStamp"), QDateTime::currentDateTimeUtc());

    // Now that we downloaded successfully, we need to check for updates only once
    // a day
    _autoUpdateTimer.start(24h);
}

void DataManagement::DataManager::autoUpdateGeoMapList()
{
    // If the last update is more than one day ago, automatically initiate an
    // update, so that maps stay at least roughly current.
    QSettings settings;
    QDateTime lastUpdate = settings.value(QStringLiteral("DataManager/MapListTimeStamp"), QDateTime()).toDateTime();

    if (!lastUpdate.isValid() || (qAbs(lastUpdate.daysTo(QDateTime::currentDateTime()) > 6)))
    {
        // Updates are due. Check again in one hour if the update went well or
        // if we need to try again.
        _autoUpdateTimer.start(1h);
        updateRemoteDataItemList();
        return;
    }

    // Updates are not yet due. Check again in one day.
    _autoUpdateTimer.start(24h);
}

auto DataManagement::DataManager::unattachedFiles() const -> QList<QString>
{
    QList<QString> result;

    // It might be possible for whatever reason that our download directory
    // contains files that we do not know whom they belong to. We hunt down
    // those files.
    QDirIterator fileIterator(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/aviation_maps",
                              QDir::Files, QDirIterator::Subdirectories);
    while (fileIterator.hasNext())
    {
        fileIterator.next();

        // Now check if this file exists as the local file of some geographic
        // map
        bool isAttachedToAviationMap = false;
        foreach (auto geoMapPtr, _items.downloadables())
        {
            if (geoMapPtr->fileName() == QFileInfo(fileIterator.filePath()).absoluteFilePath())
            {
                isAttachedToAviationMap = true;
                break;
            }
        }
        if (!isAttachedToAviationMap)
        {
            result.append(fileIterator.filePath());
        }
    }

    return result;
}
