/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include <QLocale>

#include "DownloadableGroupWatcher.h"
#include <chrono>

using namespace std::chrono_literals;


DataManagement::DownloadableGroupWatcher::DownloadableGroupWatcher(QObject *parent)
    : DataManagement::Downloadable_Abstract(parent)
{
    m_contentType = DataManagement::Downloadable_Abstract::MapSet;

    emitLocalFileContentChanged_delayedTimer.setInterval(2s);
    connect(this, &DownloadableGroupWatcher::localFileContentChanged, &emitLocalFileContentChanged_delayedTimer, qOverload<>(&QTimer::start));
    connect(&emitLocalFileContentChanged_delayedTimer, &QTimer::timeout, this, &DownloadableGroupWatcher::emitLocalFileContentChanged_delayed);
}



//
// Getter Methods
//


auto DataManagement::DownloadableGroupWatcher::description() -> QString
{
#warning Duplicates material from MultiFile
    QString result;

    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        switch(map->contentType())
        {
        case Downloadable_SingleFile::AviationMap:
            result += "<h3>"+tr("Aviation Map")+"</h3>";
            break;
        case Downloadable_SingleFile::BaseMapVector:
            result += "<h3>"+tr("Base Map")+"</h3>";
            break;
        case Downloadable_SingleFile::BaseMapRaster:
            result += "<h3>"+tr("Raster Map")+"</h3>";
            break;
        case Downloadable_SingleFile::Data:
            result += "<h3>"+tr("Data")+"</h3>";
            break;
        case Downloadable_SingleFile::TerrainMap:
            result += "<h3>"+tr("Terrain Map")+"</h3>";
            break;
        case Downloadable_SingleFile::MapSet:
            result += "<h3>"+tr("Map Set")+"</h3>";
            break;
        }
        result += map->description();
    }
    return result;
}


auto DataManagement::DownloadableGroupWatcher::downloadables() const -> QVector<QPointer<Downloadable_Abstract>>
{
    QVector<QPointer<Downloadable_Abstract>> result;
    foreach(auto _downloadable, m_downloadables) {
        if (_downloadable == nullptr) {
            continue;
        }
        result += _downloadable;
    }

    // Sort Downloadables according to section name and file name
    std::sort(result.begin(), result.end(), [](Downloadable_Abstract* a, Downloadable_Abstract* b)
    {
        if (a->section() != b->section()) {
            return (a->section() < b->section());
        }
        if (a->objectName() != b->objectName())
        {
            return a->objectName() < b->objectName();
        }
        return (a->contentType() < b->contentType());
    }
    );

    return result;
}


auto DataManagement::DownloadableGroupWatcher::downloadablesAsObjectList() const -> QVector<QObject*>
{
    QVector<QObject*> result;
    result.reserve(m_downloadables.size());
    foreach(auto downloadablePtr, downloadables())
        result.append(downloadablePtr);
    return result;
}


auto DataManagement::DownloadableGroupWatcher::downloadablesWithFile() const -> QVector<QPointer<Downloadable_SingleFile>>
{
    QVector<QPointer<Downloadable_SingleFile>> result;
    foreach(auto _downloadableX, m_downloadables) {
#warning ugly
        auto* _downloadable = qobject_cast<DataManagement::Downloadable_SingleFile*>(_downloadableX);

        if (_downloadable == nullptr) {
            continue;
        }
        if (!_downloadable->hasFile()) {
            continue;
        }
        result += _downloadable;
    }

    // Sort Downloadables according to lower boundary
    std::sort(result.begin(), result.end(), [](Downloadable_SingleFile* a, Downloadable_SingleFile* b)
    {
        if (a->section() != b->section()) {
            return (a->section() < b->section());
        }
        if (a->objectName() != b->objectName())
        {
            return a->objectName() < b->objectName();
        }
        return (a->contentType() < b->contentType());
    }
    );

    return result;
}


auto DataManagement::DownloadableGroupWatcher::downloading() -> bool
{
    foreach(auto _downloadable, m_downloadables) {
        if (_downloadable.isNull()) {
            continue;
        }
        if (_downloadable->downloading()) {
            return true;
        }
    }
    return false;
}


auto DataManagement::DownloadableGroupWatcher::files() const -> QStringList
{
    QStringList result;

    foreach(auto _downloadableX, m_downloadables) {
#warning ugly
        auto _downloadable = qobject_cast<DataManagement::Downloadable_SingleFile*>(_downloadableX);
        if (_downloadable == nullptr) {
            continue;
        }
        if (!_downloadable->hasFile()) {
            continue;
        }
        result += _downloadable->fileName();
    }

    return result;
}


auto DataManagement::DownloadableGroupWatcher::hasFile() -> bool
{
    foreach(auto _downloadable, m_downloadables) {
        if (_downloadable.isNull()) {
            continue;
        }
        if (_downloadable->hasFile()) {
            return true;
        }
    }
    return false;
}


auto DataManagement::DownloadableGroupWatcher::infoText() -> QString
{
    QString result;

    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        if (!result.isEmpty())
        {
            result += QLatin1String("<br>");
        }
        switch(map->contentType())
        {
        case Downloadable_SingleFile::AviationMap:
            result += QStringLiteral("%1: %2").arg(tr("Aviation Map"), map->infoText());
            break;
        case Downloadable_SingleFile::BaseMapRaster:
            result += QStringLiteral("%1: %2").arg(tr("Raster Map"), map->infoText());
            break;
        case Downloadable_SingleFile::BaseMapVector:
            result += QStringLiteral("%1: %2").arg(tr("Base Map"), map->infoText());
            break;
        case Downloadable_SingleFile::Data:
            result += QStringLiteral("%1: %2").arg(tr("Data"), map->infoText());
            break;
        case Downloadable_SingleFile::MapSet:
            result += QStringLiteral("%1: %2").arg(tr("Map Set"), map->infoText());
            break;
        case Downloadable_SingleFile::TerrainMap:
            result += QStringLiteral("%1: %2").arg(tr("Terrain Map"), map->infoText());
            break;
        }
    }
    return result;
}


auto DataManagement::DownloadableGroupWatcher::updateSize() -> qint64
{
    qsizetype size = 0;
    foreach(auto downloadable, m_downloadables)
    {
        size += downloadable->updateSize();
    }
    return size;
}



//
// Methods
//

void DataManagement::DownloadableGroupWatcher::deleteFiles()
{
    foreach(auto _downloadable, m_downloadables)
    {
        if (_downloadable.isNull()) {
            continue;
        }
        _downloadable->deleteFiles();
    }
}


auto DataManagement::DownloadableGroupWatcher::numberOfFilesTotal() const -> int
{
    int nFilesTotal = 0;
    foreach(auto _downloadable, m_downloadables) {
        if (_downloadable.isNull()) {
            continue;
        }
        if (_downloadable->hasFile()) {
            nFilesTotal += 1;
        } else {
            if (_downloadable->downloading()) {
                nFilesTotal += 1;
            }
        }
    }
    return nFilesTotal;
}


void DataManagement::DownloadableGroupWatcher::startDownload()
{
    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        map->startDownload();
    }
}


void DataManagement::DownloadableGroupWatcher::stopDownload()
{
    m_downloadables.removeAll(nullptr);
    foreach(auto map, m_downloadables)
    {
        map->stopDownload();
    }
}


void DataManagement::DownloadableGroupWatcher::update()
{
    foreach(auto downloadablePtr, m_downloadables) {
        if (downloadablePtr.isNull()) {
            continue;
        }
        if (downloadablePtr->updateSize() != 0) {
            downloadablePtr->startDownload();
        }
    }
}


//
// Protected Methods
//

void DataManagement::DownloadableGroupWatcher::checkAndEmitSignals()
{
    auto newDownloading           = downloading();
    auto newDownloadablesWithFile = downloadablesWithFile();
    auto newFiles                 = files();
    auto newHasFile               = hasFile();
    auto newUpdateSize            = updateSize();

    if (newDownloadablesWithFile != m_cachedDownloadablesWithFile) {
        m_cachedDownloadablesWithFile = newDownloadablesWithFile;
        emit downloadablesWithFileChanged(newDownloadablesWithFile);
    }

    if (newDownloading != m_cachedDownloading) {
        m_cachedDownloading = newDownloading;
        emit downloadingChanged();
    }

    if (newDownloading != m_cachedDownloading) {
        m_cachedDownloading = newDownloading;
        emit downloadingChanged();
    }

    if (newFiles != m_cachedFiles) {
        m_cachedFiles = newFiles;
        emit filesChanged(newFiles);
    }

    if (newHasFile != m_cachedHasFile) {
        m_cachedHasFile = newHasFile;
        emit hasFileChanged();
    }

    if (newUpdateSize != m_cachedUpdateSize) {
        m_cachedUpdateSize = newUpdateSize;
        emit updateSizeChanged();
    }
}


void DataManagement::DownloadableGroupWatcher::cleanUp()
{
    auto idx = m_downloadables.indexOf(nullptr);
    m_downloadables.removeAll(nullptr);
    if (idx >= 0) {
        emit downloadablesChanged();
    }
}



//
// Private Methods
//

void DataManagement::DownloadableGroupWatcher::emitLocalFileContentChanged_delayed()
{
    if (downloading()) {
        return;
    }
    emitLocalFileContentChanged_delayedTimer.stop();
    emit localFileContentChanged_delayed();
}


