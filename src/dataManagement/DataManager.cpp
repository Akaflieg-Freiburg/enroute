/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include <QCoreApplication>
#include <QDirIterator>
#include <QGuiApplication>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QSettings>
#include <QStack>
#include <QTemporaryDir>

#include "config.h"
#include "dataManagement/DataManager.h"
#include "fileFormats/MBTILES.h"
#include "geomaps/OpenAir.h"

using namespace std::chrono_literals;
using namespace Qt::Literals::StringLiterals;


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

    // Update maps.json file if that is too old. Check that whenever the app comes forward.
    updateRemoteDataItemListIfOutdated();
    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this,
            [this](Qt::ApplicationState state)
            {
                if (state == Qt::ApplicationActive)
                {
                    updateRemoteDataItemListIfOutdated();
                }
            });
}


void DataManagement::DataManager::cleanDataDirectory()
{
    QStringList misnamedFiles;
    QStringList unexpectedFiles;
    QDirIterator fileIterator(m_dataDirectory, QDir::Files, QDirIterator::Subdirectories);
    while (fileIterator.hasNext())
    {
        fileIterator.next();
        if (fileIterator.filePath().endsWith(u".geojson.geojson"_s)
                || fileIterator.filePath().endsWith(u".mbtiles.mbtiles"_s))
        {
            misnamedFiles += fileIterator.filePath();
        }
        if (!fileIterator.filePath().endsWith(u".terrain"_s) &&
                !fileIterator.filePath().endsWith(u".geojson"_s) &&
                !fileIterator.filePath().endsWith(u".mbtiles"_s) &&
                !fileIterator.filePath().endsWith(u".raster"_s) &&
                !fileIterator.filePath().endsWith(u".txt"_s))
        {
            unexpectedFiles += fileIterator.filePath();
        }

        // Delete aviation map files that are no longer supported, though they existed in earlier versions of this app
        if (fileIterator.filePath().endsWith(u"Ireland and Northern Ireland.terrain"_s) ||
                fileIterator.filePath().endsWith(u"Slowenia.geojson"_s) ||
                fileIterator.filePath().endsWith(u"United Kingdom.geojson"_s) ||
                fileIterator.filePath().endsWith(u"Canada.geojson"_s) ||
                fileIterator.filePath().endsWith(u"United States.geojson"_s))
        {
            unexpectedFiles += fileIterator.filePath();
        }
    }
    foreach (auto misnamedFile, misnamedFiles)
    {
        QFile::rename(misnamedFile, misnamedFile.section('.', 0, -2));
    }
    foreach (auto unexpectedFile, unexpectedFiles)
    {
        QFile::remove(unexpectedFile);
    }

    // Recurse into m_dataDirectory and delete all empty (sub)directories.
    // Also delete directories that became empty because
    // some of its subdirectories got deleted in the process.
    bool directoryDeleted = true;
    while(directoryDeleted)
    {
        directoryDeleted = false;
        QStack<QString> stack;
        stack.push(m_dataDirectory);

        while (!stack.isEmpty()) {
            const QString currentPath = stack.pop();
            const QDir dir(currentPath);

            if (dir.exists()) {
                const QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
                for (const QFileInfo& entry : entries)
                {
                    stack.push(entry.filePath());
                }

                if (dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries).isEmpty())
                {
                    dir.rmdir(currentPath);
                    directoryDeleted = true;
                }
            }
        }
    }
}


QString DataManagement::DataManager::import(const QString& fileName, const QString& newName)
{
    auto localFile = FileFormats::DataFileAbstract::openFileURL(fileName);

    auto path = m_dataDirectory+"/Unsupported";
    auto newFileName = path + "/" + newName;

    FileFormats::MBTILES mbtiles(localFile->fileName());
    switch(mbtiles.format())
    {
    case FileFormats::MBTILES::Raster:
        newFileName += u".raster"_s;
        break;
    case FileFormats::MBTILES::Vector:
        newFileName += u".mbtiles"_s;
        break;
    case FileFormats::MBTILES::Unknown:
        return tr("Unable to recognize map file format.");
    }

    if (!QDir().mkpath(path))
    {
        return tr("Unable to create directory '%1'.").arg(path);
    }
    QFile::remove(newFileName);
    if (!localFile->copy(newFileName))
    {
        QFile::remove(newFileName);
        updateDataItemListAndWhatsNew();
        return tr("Unable to copy map file to data directory.");
    }

    updateDataItemListAndWhatsNew();

    return {};
}


QString DataManagement::DataManager::importOpenAir(const QString& fileName, const QString& newName)
{

    auto path = m_dataDirectory+"/Unsupported";
    auto newFileName = path + "/" + newName;

    QStringList errors;
    QStringList warnings;
    auto json = GeoMaps::openAir::parse(fileName, errors, warnings);

    if (!errors.isEmpty())
    {
        QString info;
        info += u"<p>"_s + tr("Errors") + u"</p>"_s;
        info += u"<ul style='margin-left:-25px;'>"_s;
        foreach(auto error, errors)
        {
            info += u"<li>"_s + error + u"</li>"_s;
        }
        info += u"</ul>"_s;
        return info;
    }

    if (!QDir().mkpath(path))
    {
        return tr("Unable to create directory '%1'.").arg(path);
    }
    newFileName = newFileName+u".geojson"_s;
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


DataManagement::Downloadable_SingleFile* DataManagement::DataManager::createOrRecycleItem(const QUrl& url, const QString& localFileName, const QGeoRectangle& bBox)
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
    if (localFileName.endsWith(u"geojson"_s) ||
            localFileName.endsWith(u"mbtiles"_s) ||
            localFileName.endsWith(u"raster"_s) ||
            localFileName.endsWith(u"terrain"_s))
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
    if (localFileName.endsWith(u"terrain"_s))
    {
        m_terrainMaps.add(downloadable);
    }
    if (localFileName.endsWith(u"geojson"_s))
    {
        m_aviationMaps.add(downloadable);
    }
    if (localFileName.endsWith(u"raster"_s))
    {
        m_baseMapsRaster.add(downloadable);
        m_baseMaps.add(downloadable);
    }
    if (localFileName.endsWith(u"mbtiles"_s))
    {
        m_baseMapsVector.add(downloadable);
        m_baseMaps.add(downloadable);
    }
    if (localFileName.endsWith(u"txt"_s))
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
    QString currentVersionString(QStringLiteral(ENROUTE_VERSION_STRING));
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
            minVersionString.insert(6, '0');
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
            currentVersionString.insert(6, '0');
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

        auto baseURL = top.value(QStringLiteral("url")).toString();
        foreach (auto map, top.value(QStringLiteral("maps")).toArray())
        {
            auto obj = map.toObject();
            auto mapFileName = obj.value(QStringLiteral("path")).toString();
            auto localFileName = m_dataDirectory + "/" + mapFileName;
            auto mapUrlName = baseURL + "/" + obj.value(QStringLiteral("path")).toString();
            QUrl const mapUrl(mapUrlName);
            auto fileModificationDateTime = QDateTime::fromString(obj.value(QStringLiteral("time")).toString(), QStringLiteral("yyyyMMdd"));
            qint64 const fileSize = qRound64(obj.value(QStringLiteral("size")).toDouble());

            QGeoRectangle bbox;
            if (obj.contains(u"bbox"_s))
            {
                auto bboxData = obj.value(u"bbox"_s).toArray();
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


void DataManagement::DataManager::updateRemoteDataItemListIfOutdated()
{
    // If the last update is more than one day ago, automatically initiate an
    // update, so that maps stay at least roughly current.
    auto lastUpdate = QSettings().value(QStringLiteral("DataManager/MapListTimeStamp"), QDateTime()).toDateTime();
    if (!lastUpdate.isValid() || (qAbs(lastUpdate.daysTo(QDateTime::currentDateTime()) > 0)))
    {
        m_mapList.startDownload();
    }
}
