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

#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QLockFile>
#include <QSettings>

#include "dataManagement/UpdateNotifier.h"
#include "dataManagement/DataManager.h"
#include "geomaps/MBTILES.h"

DataManagement::DataManager::DataManager(QObject* parent) : GlobalObject(parent)
{
    // Delete funny files that might have made their way into our data directory
    cleanDataDirectory();

    // Wire up the Dowloadable object "_maps_json"
    connect(&m_mapsJSON, &DataManagement::Downloadable::downloadingChanged, this, &DataManager::downloadingRemoteItemListChanged);
    connect(&m_mapsJSON, &DataManagement::Downloadable::fileContentChanged, this, &DataManager::updateDataItemListAndWhatsNew);
    connect(&m_mapsJSON, &DataManagement::Downloadable::hasFileChanged, this, &DataManager::hasRemoteItemListChanged);
    connect(&m_mapsJSON, &DataManagement::Downloadable::fileContentChanged, this, []()
    { QSettings().setValue(QStringLiteral("DataManager/MapListTimeStamp"), QDateTime::currentDateTimeUtc()); });
    connect(&m_mapsJSON, &DataManagement::Downloadable::error, this, [this](const QString & /*unused*/, QString message)
    { emit error(std::move(message)); });

    // Wire up the DownloadableGroup _items
    connect(&m_items, &DataManagement::DownloadableGroup::filesChanged, this, &DataManager::onItemFileChanged);

    // If there is a downloaded maps.json file, we read it.
    updateDataItemListAndWhatsNew();

}

void DataManagement::DataManager::deferredInitialization()
{
    // If the last update is more than one day ago, automatically initiate an
    // update, so that maps stay at least roughly current.
    auto lastUpdate = QSettings().value(QStringLiteral("DataManager/MapListTimeStamp"), QDateTime()).toDateTime();
    if (!lastUpdate.isValid() || (qAbs(lastUpdate.daysTo(QDateTime::currentDateTime()) > 6)))
    {
        updateRemoteDataItemList();
    }

    // Set up and start the updateNotifier
    new DataManagement::UpdateNotifier(this);
}

void DataManagement::DataManager::cleanDataDirectory()
{
    QStringList misnamedFiles;
    QStringList unexpectedFiles;
    QDirIterator fileIterator(m_dataDirectory, QDir::Files, QDirIterator::Subdirectories);
    while (fileIterator.hasNext())
    {
        fileIterator.next();
        if (fileIterator.filePath().endsWith(QLatin1String(".geojson.geojson")) || fileIterator.filePath().endsWith(QLatin1String(".mbtiles.mbtiles")))
        {
            misnamedFiles += fileIterator.filePath();
        }
        if (!fileIterator.filePath().endsWith(QLatin1String(".terrain")) &&
                !fileIterator.filePath().endsWith(QLatin1String(".geojson")) &&
                !fileIterator.filePath().endsWith(QLatin1String(".mbtiles")) &&
                !fileIterator.filePath().endsWith(QLatin1String(".raster")) &&
                !fileIterator.filePath().endsWith(QLatin1String(".txt")))
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
        QDirIterator dirIterator(m_dataDirectory, QDir::Dirs | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
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

auto DataManagement::DataManager::import(const QString& fileName, const QString& newName) -> QString
{

    auto path = m_dataDirectory+"/Unsupported";
    auto newFileName = path + "/" + newName;

    GeoMaps::MBTILES mbtiles(fileName);
    switch(mbtiles.format())
    {
    case GeoMaps::MBTILES::Raster:
        newFileName += QLatin1String(".raster");
        foreach(auto downloadable, m_baseMapsVector.downloadablesWithFile())
        {
            if (!downloadable.isNull())
            {
                downloadable->deleteFile();
            }
        }
        break;
    case GeoMaps::MBTILES::Vector:
        newFileName += QLatin1String(".mbtiles");
        foreach(auto downloadable, m_baseMapsRaster.downloadablesWithFile())
        {
            if (!downloadable.isNull())
            {
                downloadable->deleteFile();
            }
        }
        break;
    case GeoMaps::MBTILES::Unknown:
        return tr("Unable to recognize map file format.");
    }

    if (!QDir().mkpath(path))
    {
        return tr("Unable to create directory '%1'.").arg(path);
    }
    QFile::remove(newFileName);
    if (!QFile::copy(fileName, newFileName))
    {
        QFile::remove(newFileName);
        updateDataItemListAndWhatsNew();
        return tr("Unable to copy map file to data directory.");
    }

    updateDataItemListAndWhatsNew();

    return {};
}

void DataManagement::DataManager::onItemFileChanged()
{
    auto items = m_items.downloadables();
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
        m_items.removeFromGroup(geoMapPtr);
        geoMapPtr->deleteLater();
    }
}

DataManagement::Downloadable *DataManagement::DataManager::createOrRecycleItem(const QUrl &url, const QString &localFileName)
{
    // If a data item with the given local file name and the given URL already exists,
    // update that remoteFileDate and remoteFileSize of that element, annd delete its
    // entry in oldMaps
    foreach (auto mapPtr, m_items.downloadables())
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
    auto* downloadable = new DataManagement::Downloadable(url, localFileName, this);
    if (localFileName.endsWith(QLatin1String("geojson")) ||
            localFileName.endsWith(QLatin1String("mbtiles")) ||
            localFileName.endsWith(QLatin1String("raster")) ||
            localFileName.endsWith(QLatin1String("terrain")))
    {
        if (url.isValid())
        {
            downloadable->setSection(url.path().section(QStringLiteral("/"), -2, -2));
        }
        else
        {
            // The noope tag "<a name>" guarantees that this section will come first alphabetically
            downloadable->setSection("<a name>"+tr("Manually Imported"));
        }
    }

    m_items.addToGroup(downloadable);
    if (localFileName.endsWith(QLatin1String("terrain")))
    {
        m_terrainMaps.addToGroup(downloadable);
    }
    if (localFileName.endsWith(QLatin1String("geojson")))
    {
        m_aviationMaps.addToGroup(downloadable);
    }
    if (localFileName.endsWith(QLatin1String("raster")))
    {
        m_baseMaps.addToGroup(downloadable);
        m_baseMapsRaster.addToGroup(downloadable);
    }
    if (localFileName.endsWith(QLatin1String("mbtiles")))
    {
        m_baseMaps.addToGroup(downloadable);
        m_baseMapsVector.addToGroup(downloadable);
    }
    if (localFileName.endsWith(QLatin1String("txt")))
    {
        m_databases.addToGroup(downloadable);
    }
    return downloadable;
}

void DataManagement::DataManager::updateDataItemListAndWhatsNew()
{
    if (!m_mapsJSON.hasFile())
    {
        return;
    }

    // Get List of file in the directory
    QList<QString> files;
    QDirIterator fileIterator(m_dataDirectory, QDir::Files, QDirIterator::Subdirectories);
    while (fileIterator.hasNext())
    {
        fileIterator.next();
        files.append(fileIterator.filePath());
    }

    // List of maps as we have them now
    auto oldMaps = m_items.downloadables();

    // To begin, we handle the maps described in the maps.json file. If these
    // maps were already present in the old list, we re-use them. Otherwise, we
    // create new Downloadable objects.
    QJsonParseError parseError{};
    auto doc = QJsonDocument::fromJson(m_mapsJSON.fileContent(), &parseError);
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
        auto localFileName = m_dataDirectory + "/" + mapFileName;
        auto mapName = mapFileName.section('.', -2, -2);
        auto mapUrlName = baseURL + "/" + obj.value(QStringLiteral("path")).toString();
        QUrl mapUrl(mapUrlName);
        auto fileModificationDateTime = QDateTime::fromString(obj.value(QStringLiteral("time")).toString(), QStringLiteral("yyyyMMdd"));
        qint64 fileSize = qRound64(obj.value(QStringLiteral("size")).toDouble());

        auto *downloadable = createOrRecycleItem(mapUrl, localFileName);
        oldMaps.removeAll(downloadable);
        downloadable->setRemoteFileDate(fileModificationDateTime);
        downloadable->setRemoteFileSize(fileSize);
        downloadable->setObjectName(mapName.section(QStringLiteral("/"), -1, -1));

        files.removeAll(localFileName);
    }

    // Next, we create or recycle items for all files that that we have found in the directory.
    foreach (auto localFileName, files)
    {
        auto *downloadable = createOrRecycleItem(QUrl(), localFileName);
        oldMaps.removeAll(downloadable);
        downloadable->setObjectName(localFileName.section(QStringLiteral("/"), -1, -1));
    }
    qDeleteAll(oldMaps);


    // Now the lists of downloadable items should be complete. Finally, we find and match up map sets.
    qDeleteAll(m_mapSets);
    m_mapSets.clear();
    foreach (auto downloadableBaseMap, m_baseMapsVector.downloadables())
    {
        if (downloadableBaseMap.isNull())
        {
            continue;
        }
        auto mapName = downloadableBaseMap->objectName();

        DataManagement::Downloadable* terrainMap = nullptr;
        foreach (auto downloadableTerrain, m_terrainMaps.downloadables())
        {
            if (downloadableTerrain.isNull())
            {
                continue;
            }
            if (mapName == downloadableTerrain->objectName())
            {
                terrainMap = downloadableTerrain;
                break;
            }
        }

        auto* mapSet = new MapSet(downloadableBaseMap, terrainMap, this);

        m_mapSets.append(mapSet);
    }

    // Update the whatsNew property
    auto newWhatsNew = top.value(QStringLiteral("whatsNew")).toString();
    if (!newWhatsNew.isEmpty() && (newWhatsNew != m_whatsNew))
    {
        m_whatsNew = newWhatsNew;
        emit whatsNewChanged();
    }
}
