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
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QSettings>

#include "GlobalSettings.h"
#include "dataManagement/DataManager.h"
#include "geomaps/MBTILES.h"
#include "geomaps/OpenAir.h"
#include <chrono>

using namespace std::chrono_literals;


DataManagement::DataManager::DataManager(QObject* parent) : GlobalObject(parent)
{
    // Delete funny files that might have made their way into our data directory
    cleanDataDirectory();

    // Wire up the Dowloadable object "_maps_json"
    connect(&m_mapList, &DataManagement::Downloadable_SingleFile::fileContentChanged, this, &DataManager::updateDataItemListAndWhatsNew);
    connect(&m_mapList, &DataManagement::Downloadable_SingleFile::fileContentChanged, this, []()
    { QSettings().setValue(QStringLiteral("DataManager/MapListTimeStamp"), QDateTime::currentDateTimeUtc()); });
    connect(&m_mapList, &DataManagement::Downloadable_SingleFile::error, this, [this](const QString & /*unused*/, const QString& message)
    { emit error(message); });

    // Wire up the DownloadableGroup _items
    connect(&m_items, &DataManagement::Downloadable_MultiFile::filesChanged, this, &DataManager::onItemFileChanged);

    m_mapsAndData.add(&m_mapSets);
    m_mapsAndData.add(&m_databases);
}


void DataManagement::DataManager::deferredInitialization()
{    
    // If there is a downloaded maps.json file, we read it.
    updateDataItemListAndWhatsNew();

    // If the last update is more than one day ago, automatically initiate an
    // update, so that maps stay at least roughly current.
    auto lastUpdate = QSettings().value(QStringLiteral("DataManager/MapListTimeStamp"), QDateTime()).toDateTime();
    if (!lastUpdate.isValid() || (qAbs(lastUpdate.daysTo(QDateTime::currentDateTime()) > 0)))
    {
        updateRemoteDataItemList();
    }

    // Setup approach charts
    QStringList filesToDelete;
    auto approachChartsDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/approach_charts";
    QDirIterator fileIterator(approachChartsDir, QDir::Files);
    while (fileIterator.hasNext())
    {
        fileIterator.next();
        auto fileName = fileIterator.fileName();
        auto idx = fileName.lastIndexOf(u"-geo_"_qs, -1);
        auto bBox = bBoxFromFileName(fileName);

        if (!fileName.endsWith(u"png"_qs) ||
            (idx == -1) ||
            !bBox.isValid())
        {
            filesToDelete += fileIterator.filePath();
            continue;
        }

        auto* downloadable = new DataManagement::Downloadable_SingleFile({}, fileIterator.filePath(), bBox, this);
        downloadable->setObjectName(fileName.left(idx));
        m_approachCharts.add(downloadable);
    }
    foreach (auto fileToDelete, filesToDelete)
    {
        QFile::remove(fileToDelete);
    }
}


void DataManagement::DataManager::cleanDataDirectory()
{

    QStringList misnamedFiles;
    QStringList unexpectedFiles;
    QDirIterator fileIterator(m_dataDirectory, QDir::Files, QDirIterator::Subdirectories);
    while (fileIterator.hasNext())
    {
        fileIterator.next();
        if (fileIterator.filePath().endsWith(u".geojson.geojson"_qs)
                || fileIterator.filePath().endsWith(u".mbtiles.mbtiles"_qs))
        {
            misnamedFiles += fileIterator.filePath();
        }
        if (!fileIterator.filePath().endsWith(u".terrain"_qs) &&
                !fileIterator.filePath().endsWith(u".geojson"_qs) &&
                !fileIterator.filePath().endsWith(u".mbtiles"_qs) &&
                !fileIterator.filePath().endsWith(u".raster"_qs) &&
                !fileIterator.filePath().endsWith(u".txt"_qs))
        {
            unexpectedFiles += fileIterator.filePath();
        }

        // Delete aviation map files that are no longer supported, though they existed in earlier versions of this app
        if (fileIterator.filePath().endsWith(u"Ireland and Northern Ireland.terrain"_qs) ||
                fileIterator.filePath().endsWith(u"Slowenia.geojson"_qs) ||
                fileIterator.filePath().endsWith(u"United Kingdom.geojson"_qs) ||
                fileIterator.filePath().endsWith(u"Canada.geojson"_qs) ||
                fileIterator.filePath().endsWith(u"United States.geojson"_qs))
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
        newFileName += u".raster"_qs;
        break;
    case GeoMaps::MBTILES::Vector:
        newFileName += u".mbtiles"_qs;
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


auto DataManagement::DataManager::importOpenAir(const QString& fileName, const QString& newName) -> QString
{

    auto path = m_dataDirectory+"/Unsupported";
    auto newFileName = path + "/" + newName;

    QStringList errors;
    QStringList warnings;
    auto json = GeoMaps::openAir::parse(fileName, errors, warnings);

    if (!errors.isEmpty())
    {
        QString info;
        info += u"<p>"_qs + tr("Errors") + u"</p>"_qs;
        info += u"<ul style='margin-left:-25px;'>"_qs;
        foreach(auto error, errors)
        {
            info += u"<li>"_qs + error + u"</li>"_qs;
        }
        info += u"</ul>"_qs;
        return info;
    }

    if (!QDir().mkpath(path))
    {
        return tr("Unable to create directory '%1'.").arg(path);
    }
    newFileName = newFileName+u".geojson"_qs;
    QFile::remove(newFileName);
    QFile file(newFileName);
    file.open(QIODeviceBase::WriteOnly);
    file.write(json.toJson());
    file.close();
    if (file.error() != QFileDevice::NoError)
    {
        QFile::remove(newFileName);
        updateDataItemListAndWhatsNew();
        return tr("Error writing file '%1': %2.").arg(newFileName, file.errorString());
    }
    updateDataItemListAndWhatsNew();
    return {};
}


QGeoRectangle DataManagement::DataManager::bBoxFromFileName(const QString& fileName)
{
    auto list = fileName.chopped(4).split('_');
    if (list.size() < 4)
    {
        return {};
    }
    list = list.last(4);

    QGeoCoordinate topLeft(list[1].toDouble(), list[0].toDouble());
    QGeoCoordinate bottomRight(list[3].toDouble(), list[2].toDouble());
    return QGeoRectangle(topLeft, bottomRight);
}


void DataManagement::DataManager::onItemFileChanged()
{
    auto items = m_items.downloadables();
    foreach (auto geoMapPtrX, items)
    {
        auto* geoMapPtr = qobject_cast<DataManagement::Downloadable_SingleFile*>(geoMapPtrX);
        if (geoMapPtr == nullptr)
        {
            continue;
        }
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
        m_items.remove(geoMapPtr);
        delete geoMapPtr;
        QTimer::singleShot(100ms, this, &DataManagement::DataManager::updateDataItemListAndWhatsNew);
    }
}


auto DataManagement::DataManager::createOrRecycleItem(const QUrl& url, const QString& localFileName, const QGeoRectangle& bBox) -> DataManagement::Downloadable_SingleFile*
{
    // If a data item with the given local file name and the given URL already exists,
    // update that remoteFileDate and remoteFileSize of that element, annd delete its
    // entry in oldMaps
    foreach (auto mapPtrX, m_items.downloadables())
    {
        auto* mapPtr = qobject_cast<DataManagement::Downloadable_SingleFile*>(mapPtrX);
        if (mapPtr == nullptr)
        {
            continue;
        }
        if ((mapPtr->fileName() == localFileName) && (mapPtr->url() == url))
        {
            return mapPtr;
        }
    }

    // Construct a new downloadable object and add to appropriate groups
    auto* downloadable = new DataManagement::Downloadable_SingleFile(url, localFileName, bBox, this);
    downloadable->setObjectName(localFileName.section(QStringLiteral("/"), -1, -1).section(QStringLiteral("."), 0, -2));
    if (localFileName.endsWith(u"geojson"_qs) ||
            localFileName.endsWith(u"mbtiles"_qs) ||
            localFileName.endsWith(u"raster"_qs) ||
            localFileName.endsWith(u"terrain"_qs))
    {
        if (url.isValid())
        {
            downloadable->setSection(url.path().section(QStringLiteral("/"), -2, -2));
        }
        else
        {
            // The noops tag "<a name>" guarantees that this section will come first alphabetically
            downloadable->setSection("<a name>"+tr("Manually Imported"));
        }

        bool hasMapSet = false;
        foreach(auto mapSetX, m_mapSets.downloadables())
        {
            auto* mapSet = qobject_cast<DataManagement::Downloadable_MultiFile*>(mapSetX);
            if (mapSet == nullptr)
            {
                continue;
            }
            if ((mapSet->objectName() == downloadable->objectName()) && (mapSet->section() == downloadable->section()))
            {
                mapSet->add(downloadable);
                hasMapSet = true;
                break;
            }
        }
        if (!hasMapSet)
        {
            auto* newMapSet = new DataManagement::Downloadable_MultiFile(Downloadable_MultiFile::MultiUpdate, this);
            newMapSet->add(downloadable);
            m_mapSets.add(newMapSet);
        }
    }

    m_items.add(downloadable);
    if (localFileName.endsWith(u"terrain"_qs))
    {
        m_terrainMaps.add(downloadable);
    }
    if (localFileName.endsWith(u"geojson"_qs))
    {
        m_aviationMaps.add(downloadable);
    }
    if (localFileName.endsWith(u"raster"_qs))
    {
        m_baseMapsRaster.add(downloadable);
        m_baseMaps.add(downloadable);
    }
    if (localFileName.endsWith(u"mbtiles"_qs))
    {
        m_baseMapsVector.add(downloadable);
        m_baseMaps.add(downloadable);
    }
    if (localFileName.endsWith(u"txt"_qs))
    {
        m_databases.add(downloadable);
    }
    return downloadable;
}


void DataManagement::DataManager::updateDataItemListAndWhatsNew()
{
    if (!m_mapList.hasFile())
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
    auto doc = QJsonDocument::fromJson(m_mapList.fileContent(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        return;
    }

    auto top = doc.object();

    // Prepare strings to check if the present version
    auto minVersionString = top.value(QStringLiteral("minAppVersion")).toString();
    QString currentVersionString(QStringLiteral(PROJECT_VERSION));
    { // Ensure that strings have the format xx.yy.zz
        if ((minVersionString.size() >= 2) && (minVersionString[1] == '.'))
        {
            minVersionString.prepend('0');
        }
        if ((minVersionString.size() >= 5) && (minVersionString[4] == '.'))
        {
            minVersionString.insert(3, '0');
        }
        if (minVersionString.size() < 8)
        {
            minVersionString.insert(7, '0');
        }
        if (currentVersionString[1] == '.')
        {
            currentVersionString.prepend('0');
        }
        if (currentVersionString[4] == '.')
        {
            currentVersionString.insert(3, '0');
        }
        if (currentVersionString.size() < 8)
        {
            currentVersionString.insert(7, '0');
        }
    }

    if (minVersionString > currentVersionString)
    {
        if (!m_appUpdateRequired)
        {
            m_appUpdateRequired = true;
            emit appUpdateRequiredChanged();
        }
    }
    else
    {
        if (m_appUpdateRequired)
        {
            m_appUpdateRequired = false;
            emit appUpdateRequiredChanged();
        }

        auto FAA_ID = top.value(QStringLiteral("FAA_ID")).toString();
        auto FAA_KEY = top.value(QStringLiteral("FAA_KEY")).toString();
        if (!FAA_ID.isEmpty() && !FAA_KEY.isEmpty())
        {
            globalSettings()->setFAAData(FAA_ID, FAA_KEY);
        }

        auto baseURL = top.value(QStringLiteral("url")).toString();
        foreach (auto map, top.value(QStringLiteral("maps")).toArray())
        {
            auto obj = map.toObject();
            auto mapFileName = obj.value(QStringLiteral("path")).toString();
            auto localFileName = m_dataDirectory + "/" + mapFileName;
            auto mapUrlName = baseURL + "/" + obj.value(QStringLiteral("path")).toString();
            QUrl mapUrl(mapUrlName);
            auto fileModificationDateTime = QDateTime::fromString(obj.value(QStringLiteral("time")).toString(), QStringLiteral("yyyyMMdd"));
            qint64 fileSize = qRound64(obj.value(QStringLiteral("size")).toDouble());


            QGeoRectangle bbox;
            if (obj.contains(u"bbox"_qs))
            {
                auto bboxData = obj.value(u"bbox"_qs).toArray();
                auto left = bboxData.at(0).toDouble();
                auto bottom = bboxData.at(1).toDouble();

                auto right = bboxData.at(2).toDouble();
                auto top = bboxData.at(3).toDouble();
                bbox.setTopLeft( {top, left} );
                bbox.setBottomRight( {bottom, right} );
            }

            auto* downloadable = createOrRecycleItem(mapUrl, localFileName, bbox);
            oldMaps.removeAll(downloadable);
            downloadable->setRemoteFileDate(fileModificationDateTime);
            downloadable->setRemoteFileSize(fileSize);

            files.removeAll(localFileName);
        }
    }

    // Next, we create or recycle items for all files that that we have found in the directory.
    foreach (auto localFileName, files)
    {
        auto *downloadable = createOrRecycleItem(QUrl(), localFileName, {});
        oldMaps.removeAll(downloadable);
        downloadable->setObjectName(localFileName.section(QStringLiteral("/"), -1, -1));
    }
    qDeleteAll(oldMaps);

    // Delete empty map sets
    QVector<DataManagement::Downloadable_MultiFile*> dump;
    foreach (auto mapSetX, m_mapSets.downloadables())
    {
        auto* mapSet = qobject_cast<DataManagement::Downloadable_MultiFile*>(mapSetX);
        if (mapSet == nullptr)
        {
            continue;
        }
        if (mapSet->downloadables().isEmpty())
        {
            dump << mapSet;
        }

    }
    foreach(auto mapSet, dump)
    {
        m_mapSets.remove(mapSet);
    }

    // Update the whatsNew property
    auto newWhatsNew = top.value(QStringLiteral("whatsNew")).toString();
    if (!newWhatsNew.isEmpty() && (newWhatsNew != m_whatsNew))
    {
        m_whatsNew = newWhatsNew;
        emit whatsNewChanged();
    }
}
