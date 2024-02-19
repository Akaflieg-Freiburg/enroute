/***************************************************************************
 *   Copyright (C) 2022-2024 by Stefan Kebekus                             *
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

#include <QPointer>

#include "Downloadable_MultiFile.h"


DataManagement::Downloadable_MultiFile::Downloadable_MultiFile(DataManagement::Downloadable_MultiFile::UpdatePolicy updatePolicy, QObject* parent)
    : Downloadable_Abstract(parent), m_updatePolicy(updatePolicy)
{
    m_contentType = MapSet;

    connect(this, &DataManagement::Downloadable_MultiFile::downloadingChanged, this, &DataManagement::Downloadable_MultiFile::evaluateUpdateSize, Qt::QueuedConnection);
}



//
// Getter methods
//

auto DataManagement::Downloadable_MultiFile::description() -> QString
{
    QString result;

    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        switch(map->contentType())
        {
        case Downloadable_Abstract::VAC:
            result += "<h3>"+tr("Visual Approach Chart")+"</h3>";
            break;
        case Downloadable_Abstract::AviationMap:
            result += "<h3>"+tr("Aviation Map")+"</h3>";
            break;
        case Downloadable_Abstract::BaseMapVector:
            result += "<h3>"+tr("Base Map")+"</h3>";
            break;
        case Downloadable_Abstract::BaseMapRaster:
            result += "<h3>"+tr("Raster Map")+"</h3>";
            break;
        case Downloadable_Abstract::Data:
            result += "<h3>"+tr("Data")+"</h3>";
            break;
        case Downloadable_Abstract::TerrainMap:
            result += "<h3>"+tr("Terrain Map")+"</h3>";
            break;
        case Downloadable_Abstract::MapSet:
            result += "<h3>"+tr("Map Set")+"</h3>";
            break;
        }
        result += map->description();
    }
    return result;
}


auto DataManagement::Downloadable_MultiFile::infoText() -> QString
{
    QString result;

    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        if (!result.isEmpty())
        {
            result += u"<br>"_qs;
        }
        switch(map->contentType())
        {
        case Downloadable_Abstract::AviationMap:
            result += QStringLiteral("%1: %2").arg(tr("Aviation Map"), map->infoText());
            break;
        case Downloadable_Abstract::BaseMapRaster:
            result += QStringLiteral("%1: %2").arg(tr("Raster Map"), map->infoText());
            break;
        case Downloadable_Abstract::BaseMapVector:
            result += QStringLiteral("%1: %2").arg(tr("Base Map"), map->infoText());
            break;
        case Downloadable_Abstract::Data:
            result += QStringLiteral("%1: %2").arg(tr("Data"), map->infoText());
            break;
        case Downloadable_Abstract::MapSet:
            result += QStringLiteral("%1: %2").arg(tr("Map Set"), map->infoText());
            break;
        case Downloadable_Abstract::TerrainMap:
            result += QStringLiteral("%1: %2").arg(tr("Terrain Map"), map->infoText());
            break;
        case Downloadable_Abstract::VAC:
            result += QStringLiteral("%1: %2").arg(tr("Visual Approach ChartTerrain Map"), map->infoText());
            break;
        }
    }
    return result;
}



//
// Methods
//

void DataManagement::Downloadable_MultiFile::add(DataManagement::Downloadable_Abstract* map)
{
    if (rawAdd(map))
    {
        evaluateDownloading();
        evaluateFiles();
        evaluateHasFile();
        evaluateRemoteFileSize();
        evaluateUpdateSize();

        emit downloadablesChanged();
        emit fileContentChanged();
    }
}


void DataManagement::Downloadable_MultiFile::add(const QVector<DataManagement::Downloadable_Abstract*>& maps)
{
    auto cache = m_downloadables.size();
    foreach(auto map, maps)
    {
        rawAdd(map);
    }

    if (cache != m_downloadables.size())
    {
        evaluateDownloading();
        evaluateFiles();
        evaluateHasFile();
        evaluateRemoteFileSize();
        evaluateUpdateSize();

        emit downloadablesChanged();
        emit fileContentChanged();
    }
}


void DataManagement::Downloadable_MultiFile::clear()
{
    if (m_downloadables.isEmpty())
    {
        return;
    }

    foreach (auto map, m_downloadables) {
        disconnect(map, nullptr, this, nullptr);
    }
    m_downloadables.clear();

    emit descriptionChanged();
    emit downloadablesChanged();
    emit infoTextChanged();
    evaluateDownloading();
    evaluateFiles();
    evaluateHasFile();
    evaluateRemoteFileSize();
    evaluateUpdateSize();
}


void DataManagement::Downloadable_MultiFile::deleteFiles()
{
    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        map->deleteFiles();
    }
}


auto DataManagement::Downloadable_MultiFile::downloadables() -> QVector<DataManagement::Downloadable_Abstract*>
{
    QVector<DataManagement::Downloadable_Abstract*> result;
    m_downloadables.removeAll(nullptr);
    foreach(auto downloadable, m_downloadables)
    {
        result += downloadable;
    }

    // Sort Downloadables according to section name and object name
    std::sort(result.begin(), result.end(), [](Downloadable_Abstract* first, Downloadable_Abstract* second)
    {
        if (first->section() != second->section()) {
            return (first->section() < second->section());
        }
        if (first->objectName() != second->objectName())
        {
            return first->objectName() < second->objectName();
        }
        return (first->contentType() < second->contentType());
    }
    );

    return result;
}


auto DataManagement::Downloadable_MultiFile::downloadables4Location(const QGeoCoordinate& location) -> QVector<DataManagement::Downloadable_Abstract*>
{
    if (!location.isValid())
    {
        return {};
    }

    QVector<DataManagement::Downloadable_Abstract*> result;
    m_downloadables.removeAll(nullptr);
    foreach(auto downloadable, m_downloadables)
    {
        auto bbox = downloadable->boundingBox();
        if (bbox.isValid() && bbox.contains(location))
        {
            result += downloadable;
        }
    }

    // Sort Downloadables according to section name and object name
    std::sort(result.begin(), result.end(), [](Downloadable_Abstract* first, Downloadable_Abstract* second)
    {
        if (first->objectName() != second->objectName())
        {
            return first->objectName() < second->objectName();
        }
        return (first->contentType() < second->contentType());
    }
    );

    return result;
}


auto DataManagement::Downloadable_MultiFile::downloadablesByDistance(const QGeoCoordinate& location) -> QVector<DataManagement::Downloadable_Abstract*>
{
    if (!location.isValid())
    {
        return downloadables();
    }

    QVector<DataManagement::Downloadable_Abstract*> result;
    m_downloadables.removeAll(nullptr);
    foreach(auto downloadable, m_downloadables)
    {
        result += downloadable;
    }

    // Sort Downloadables according to section name and object name
    std::sort(result.begin(), result.end(), [location](Downloadable_Abstract* first, Downloadable_Abstract* second)
              { return first->boundingBox().center().distanceTo(location) < second->boundingBox().center().distanceTo(location); }
              );

    return result;
}


void DataManagement::Downloadable_MultiFile::startDownload()
{
    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        map->startDownload();
    }
}


void DataManagement::Downloadable_MultiFile::stopDownload()
{
    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        map->stopDownload();
    }
}


void DataManagement::Downloadable_MultiFile::update()
{
    if (updateSize() == 0)
    {
        return;
    }

    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        if (map->hasFile())
        {
            map->update();
        }
        else
        {
            if (m_updatePolicy == MultiUpdate)
            {
                map->startDownload();
            }
        }
    }
}


void DataManagement::Downloadable_MultiFile::remove(DataManagement::Downloadable_Abstract* map)
{
    if (!m_downloadables.contains(map))
    {
        return;
    }

    disconnect(map, nullptr, this, nullptr);
    m_downloadables.removeAll(map);

    emit descriptionChanged();
    emit downloadablesChanged();
    emit infoTextChanged();
    evaluateDownloading();
    evaluateFiles();
    evaluateHasFile();
    evaluateRemoteFileSize();
    evaluateUpdateSize();
}



//
// Private
//

void DataManagement::Downloadable_MultiFile::evaluateDownloading()
{
    bool newDownloading = false;
    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        if (map->downloading())
        {
            newDownloading = true;
            break;
        }
    }
    if (newDownloading != m_downloading)
    {
        m_downloading = newDownloading;
        emit downloadingChanged();
    }
}


void DataManagement::Downloadable_MultiFile::evaluateFiles()
{
    QStringList newFiles;
    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        newFiles << map->files();
    }
    if (newFiles != m_files)
    {
        m_files = newFiles;
        emit filesChanged();
    }
}


void DataManagement::Downloadable_MultiFile::evaluateHasFile()
{
    bool newHasFile = false;
    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        if (map->hasFile())
        {
            newHasFile = true;
            break;
        }
    }
    if (newHasFile != m_hasFile)
    {
        m_hasFile = newHasFile;
        emit hasFileChanged();
    }
}


void DataManagement::Downloadable_MultiFile::evaluateRemoteFileSize()
{
    qint64 newRemoteFileSize = -1;
    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        auto intermediate = map->remoteFileSize();
        if (intermediate == -1)
        {
            newRemoteFileSize = -1;
            break;
        }
        newRemoteFileSize += intermediate;
    }
    if (newRemoteFileSize != m_remoteFileSize)
    {
        m_remoteFileSize = newRemoteFileSize;
        emit remoteFileSizeChanged();
    }
}


void DataManagement::Downloadable_MultiFile::evaluateUpdateSize()
{
    qint64 newUpdateSize = 0;
    if (!downloading() && hasFile())
    {
        m_downloadables.removeAll(nullptr);
        foreach(auto map, m_downloadables)
        {
            if (map->hasFile())
            {
                newUpdateSize += qint64(map->updateSize());
            }
            else
            {
                if (m_updatePolicy == MultiUpdate)
                {
                    newUpdateSize += qint64(map->remoteFileSize());
                }
            }
        }
    }

    if (newUpdateSize != m_updateSize)
    {
        m_updateSize = newUpdateSize;
        emit updateSizeChanged();
    }

}


bool DataManagement::Downloadable_MultiFile::rawAdd(DataManagement::Downloadable_Abstract* map)
{
    // Safety checks
    if (map == nullptr)
    {
        return false;
    }
    if (m_downloadables.contains(map))
    {
        return false;
    }

    // Wire up: These properties will always change if they change in one of the members. We can therefore
    // directly connect the notifier signals of our new member to the notifier signals of this instance
    connect(map, &DataManagement::Downloadable_Abstract::descriptionChanged, this, &DataManagement::Downloadable_Abstract::descriptionChanged, Qt::QueuedConnection);
    connect(map, &DataManagement::Downloadable_Abstract::infoTextChanged, this, &DataManagement::Downloadable_MultiFile::infoTextChanged, Qt::QueuedConnection);

    // Wire up: These properties might or might not change if they change in one of the members. We therefore
    // connect the notifier signals of our new member to the slot updateMembers(), which re-evaluates the
    // properties and emits notifier signals when appropriate.
    connect(map, &DataManagement::Downloadable_Abstract::downloadingChanged, this, &DataManagement::Downloadable_MultiFile::evaluateDownloading, Qt::QueuedConnection);
    connect(map, &DataManagement::Downloadable_Abstract::filesChanged, this, &DataManagement::Downloadable_MultiFile::evaluateFiles, Qt::QueuedConnection);
    connect(map, &DataManagement::Downloadable_Abstract::hasFileChanged, this, &DataManagement::Downloadable_MultiFile::evaluateHasFile, Qt::QueuedConnection);
    connect(map, &DataManagement::Downloadable_Abstract::remoteFileSizeChanged, this, &DataManagement::Downloadable_MultiFile::evaluateRemoteFileSize, Qt::QueuedConnection);
    connect(map, &DataManagement::Downloadable_Abstract::hasFileChanged, this, &DataManagement::Downloadable_MultiFile::evaluateUpdateSize, Qt::QueuedConnection);
    connect(map, &DataManagement::Downloadable_Abstract::remoteFileSizeChanged, this, &DataManagement::Downloadable_MultiFile::evaluateUpdateSize, Qt::QueuedConnection);
    connect(map, &DataManagement::Downloadable_Abstract::updateSizeChanged, this, &DataManagement::Downloadable_MultiFile::evaluateUpdateSize, Qt::QueuedConnection);

    // Wire up: when the new member gets destroyed, we need to change all properties.
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::descriptionChanged);
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::downloadablesChanged);
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::infoTextChanged);
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::evaluateDownloading);
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::evaluateFiles);
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::evaluateHasFile);
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::evaluateRemoteFileSize);
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::evaluateUpdateSize);

    // Wire up: directly forward error messages and file content changed signals
    connect(map, &DataManagement::Downloadable_Abstract::error, this, &DataManagement::Downloadable_MultiFile::error);
    connect(map, &DataManagement::Downloadable_Abstract::fileContentChanged, this, &DataManagement::Downloadable_MultiFile::fileContentChanged);

    // Copy downloadable metadata into this
    setObjectName(map->objectName());
    setSection(map->section());
    m_boundingBox = map->boundingBox();

    // Add downloadable
    m_downloadables.append(map);

    return true;
}
