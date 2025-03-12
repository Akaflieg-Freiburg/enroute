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

using namespace Qt::Literals::StringLiterals;


DataManagement::Downloadable_MultiFile::Downloadable_MultiFile(DataManagement::Downloadable_MultiFile::UpdatePolicy updatePolicy, QObject* parent)
    : Downloadable_Abstract(parent), m_updatePolicy(updatePolicy)
{
    setContentType(MapSet);

    connect(this, &DataManagement::Downloadable_MultiFile::downloadingChanged, this, &DataManagement::Downloadable_MultiFile::evaluateUpdateSize, Qt::QueuedConnection);

    m_hasFile.setBinding([this]() {return computeHasFile();});
}



//
// Getter methods
//

auto DataManagement::Downloadable_MultiFile::description() -> QString
{
    QString result;
    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }
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

    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }
        if (!result.isEmpty())
        {
            result += u"<br>"_s;
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
        evaluateRemoteFileSize();
        evaluateUpdateSize();

        emit downloadablesChanged();
        emit fileContentChanged();
    }
}


void DataManagement::Downloadable_MultiFile::add(const QVector<DataManagement::Downloadable_Abstract*>& maps)
{
    auto cache = m_downloadables.value().size();
    foreach(auto map, maps)
    {
        rawAdd(map);
    }

    if (cache != m_downloadables.value().size())
    {
        evaluateDownloading();
        evaluateFiles();
        evaluateRemoteFileSize();
        evaluateUpdateSize();

        emit downloadablesChanged();
        emit fileContentChanged();
    }
}


void DataManagement::Downloadable_MultiFile::clear()
{
    if (m_downloadables.value().isEmpty())
    {
        return;
    }

    foreach (auto map, m_downloadables.value())
    {
        disconnect(map, nullptr, this, nullptr);
    }
    m_downloadables = QVector<QPointer<DataManagement::Downloadable_Abstract>>();

    emit descriptionChanged();
    emit downloadablesChanged();
    emit infoTextChanged();
    evaluateDownloading();
    evaluateFiles();
    evaluateRemoteFileSize();
    evaluateUpdateSize();
}


void DataManagement::Downloadable_MultiFile::deleteFiles()
{
    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }
        map->deleteFiles();
    }
}


QVector<DataManagement::Downloadable_Abstract*> DataManagement::Downloadable_MultiFile::downloadables()
{
    QVector<DataManagement::Downloadable_Abstract*> result;
    foreach(auto downloadable, m_downloadables.value())
    {
        if (downloadable.isNull())
        {
            continue;
        }

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
    foreach(auto downloadable, m_downloadables.value())
    {
        if (downloadable.isNull())
        {
            continue;
        }
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
    foreach(auto downloadable, m_downloadables.value())
    {
        if (downloadable.isNull())
        {
            continue;
        }
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
    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }
        map->startDownload();
    }
}


void DataManagement::Downloadable_MultiFile::stopDownload()
{
    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }
        map->stopDownload();
    }
}


void DataManagement::Downloadable_MultiFile::update()
{
    if (updateSize() == 0)
    {
        return;
    }

    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }

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
    if (!m_downloadables.value().contains(map))
    {
        return;
    }

    disconnect(map, nullptr, this, nullptr);
    auto tmp = m_downloadables.value();
    tmp.removeAll(map);
    m_downloadables = tmp;

    emit descriptionChanged();
    emit downloadablesChanged();
    emit infoTextChanged();
    evaluateDownloading();
    evaluateFiles();
    evaluateRemoteFileSize();
    evaluateUpdateSize();
}



//
// Private
//

void DataManagement::Downloadable_MultiFile::evaluateDownloading()
{
    bool newDownloading = false;
    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }

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
    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }
        newFiles << map->files();
    }
    if (newFiles != m_files)
    {
        m_files = newFiles;
        emit filesChanged();
    }
}


bool DataManagement::Downloadable_MultiFile::computeHasFile()
{
    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }
        if (map->hasFile())
        {
            return true;
        }
    }
    return false;
}


void DataManagement::Downloadable_MultiFile::evaluateRemoteFileSize()
{
    qint64 newRemoteFileSize = -1;
    foreach(auto map, m_downloadables.value())
    {
        if (map.isNull())
        {
            continue;
        }

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
        foreach(auto map, m_downloadables.value())
        {
            if (map.isNull())
            {
                continue;
            }

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
    if (m_downloadables.value().contains(map))
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
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::evaluateRemoteFileSize);
    connect(map, &QObject::destroyed, this, &DataManagement::Downloadable_MultiFile::evaluateUpdateSize);

    // Wire up: directly forward error messages and file content changed signals
    connect(map, &DataManagement::Downloadable_Abstract::error, this, &DataManagement::Downloadable_MultiFile::error);
    connect(map, &DataManagement::Downloadable_Abstract::fileContentChanged, this, &DataManagement::Downloadable_MultiFile::fileContentChanged);

    // Copy downloadable metadata into this
    setObjectName(map->objectName());
    setSection(map->section());
    setBoundingBox(map->boundingBox());

    // Add downloadable
    auto tmp = m_downloadables.value();
    tmp.append(map);
    m_downloadables = tmp;

    return true;
}
