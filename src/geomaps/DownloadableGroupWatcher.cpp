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


GeoMaps::DownloadableGroupWatcher::DownloadableGroupWatcher(QObject *parent)
    : QObject(parent)
{
    emitLocalFileContentChanged_delayedTimer.setInterval(2s);
    connect(this, &DownloadableGroupWatcher::localFileContentChanged, &emitLocalFileContentChanged_delayedTimer, qOverload<>(&QTimer::start));
    connect(&emitLocalFileContentChanged_delayedTimer, &QTimer::timeout, this, &DownloadableGroupWatcher::emitLocalFileContentChanged_delayed);
}


void GeoMaps::DownloadableGroupWatcher::emitLocalFileContentChanged_delayed()
{
    if (downloading()) {
        return;
    }
    emitLocalFileContentChanged_delayedTimer.stop();
    emit localFileContentChanged_delayed();
}


auto GeoMaps::DownloadableGroupWatcher::hasFile() const -> bool
{
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull()) {
            continue;
        }
        if (_downloadable->hasFile()) {
            return true;
        }
    }
    return false;
}


auto GeoMaps::DownloadableGroupWatcher::downloading() const -> bool
{
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull()) {
            continue;
        }
        if (_downloadable->downloading()) {
            return true;
        }
    }
    return false;
}


auto GeoMaps::DownloadableGroupWatcher::files() const -> QStringList
{
    QStringList result;

    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull()) {
            continue;
        }
        if (!_downloadable->hasFile()) {
            continue;
        }
        result += _downloadable->fileName();
    }

    return result;
}


auto GeoMaps::DownloadableGroupWatcher::updatable() const -> bool
{
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull()) {
            continue;
        }
        if (_downloadable->updatable()) {
            return true;
        }
    }

    return false;
}


void GeoMaps::DownloadableGroupWatcher::cleanUp()
{
    auto idx = _downloadables.indexOf(nullptr);
    _downloadables.removeAll(nullptr);
    if (idx >= 0) {
        emit downloadablesChanged();
    }
}


void GeoMaps::DownloadableGroupWatcher::checkAndEmitSignals()
{
    bool                          newDownloading           = downloading();
    QVector<QPointer<Downloadable>> newDownloadablesWithFile = downloadablesWithFile();
    QStringList                   newFiles                 = files();
    bool                          newHasFile               = hasFile();
    bool                          newUpdatable             = updatable();
    QString                       newUpdateSize            = updateSize();

    if (newDownloadablesWithFile != _cachedDownloadablesWithFile) {
        _cachedDownloadablesWithFile = newDownloadablesWithFile;
        emit downloadablesWithFileChanged(newDownloadablesWithFile);
    }

    if (newDownloading != _cachedDownloading) {
        _cachedDownloading = newDownloading;
        emit downloadingChanged(newDownloading);
    }

    if (newDownloading != _cachedDownloading) {
        _cachedDownloading = newDownloading;
        emit downloadingChanged(newDownloading);
    }

    if (newFiles != _cachedFiles) {
        _cachedFiles = newFiles;
        emit filesChanged(newFiles);
    }

    if (newHasFile != _cachedHasFile) {
        _cachedHasFile = newHasFile;
        emit hasFileChanged(newHasFile);
    }

    if (newUpdatable != _cachedUpdatable) {
        _cachedUpdatable = newUpdatable;
        emit updatableChanged(newUpdatable);
    }

    if (newUpdateSize != _cachedUpdateSize) {
        _cachedUpdateSize = newUpdateSize;
        emit updateSizeChanged(newUpdateSize);
    }
}


auto GeoMaps::DownloadableGroupWatcher::downloadables() const -> QVector<QPointer<Downloadable>>
{
    QVector<QPointer<Downloadable>> result;
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull()) {
            continue;
        }
        result += _downloadable;
    }

    // Sort Downloadables according to lower boundary
    std::sort(result.begin(), result.end(), [](Downloadable* a, Downloadable* b)
    {
        if (a->section() != b->section()) {
            return (a->section() < b->section());
        }
        return (a->fileName() < b->fileName());
    }
    );

    return result;
}


auto GeoMaps::DownloadableGroupWatcher::downloadablesWithFile() const -> QVector<QPointer<Downloadable>>
{
    QVector<QPointer<Downloadable>> result;
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull()) {
            continue;
        }
        if (!_downloadable->hasFile()) {
            continue;
        }
        result += _downloadable;
    }

    // Sort Downloadables according to lower boundary
    std::sort(result.begin(), result.end(), [](Downloadable* a, Downloadable* b)
    {
        if (a->section() != b->section()) {
            return (a->section() < b->section());
        }
        return (a->fileName() < b->fileName());
    }
    );

    return result;
}


auto GeoMaps::DownloadableGroupWatcher::downloadablesAsObjectList() const -> QVector<QObject*>
{
    QVector<QObject*> result;
    foreach(auto downloadablePtr, downloadables())
        result.append(downloadablePtr);
    return result;
}


void GeoMaps::DownloadableGroupWatcher::updateAll()
{
    foreach(auto downloadablePtr, _downloadables) {
        if (downloadablePtr.isNull()) {
            continue;
        }
        if (downloadablePtr->updatable()) {
            downloadablePtr->startFileDownload();
        }
    }
}


auto GeoMaps::DownloadableGroupWatcher::updateSize() const -> QString
{
    qint64 downloadSize = 0;
    foreach(auto downloadable, _downloadables)
        if (downloadable->updatable()) {
            downloadSize += downloadable->remoteFileSize();
        }

    return QLocale::system().formattedDataSize(downloadSize, 1, QLocale::DataSizeSIFormat);
}


auto GeoMaps::DownloadableGroupWatcher::numberOfFilesTotal() const -> int
{
    int nFilesTotal = 0;
    foreach(auto _downloadable, _downloadables) {
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
