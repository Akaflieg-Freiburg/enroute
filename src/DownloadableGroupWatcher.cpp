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

#include "DownloadableGroupWatcher.h"


DownloadableGroupWatcher::DownloadableGroupWatcher(QObject *parent)
    : QObject(parent), 
{
}


bool DownloadableGroupWatcher::hasFile() const
{
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        if (_downloadable->hasFile())
            return true;
    }
    return false;
}


bool DownloadableGroupWatcher::downloading() const
{
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        if (_downloadable->downloading())
            return true;
    }
    return false;
}


QStringList DownloadableGroupWatcher::files() const
{
    QStringList result;

    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        if (!_downloadable->hasFile())
            continue;
        result += _downloadable->fileName();
    }

    return result;
}


bool DownloadableGroupWatcher::updatable() const
{
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        if (_downloadable->updatable())
            return true;
    }

    return false;
}


void DownloadableGroupWatcher::cleanUp()
{
    auto idx = _downloadables.indexOf(nullptr);
    _downloadables.removeAll(nullptr);
    if (idx >= 0)
        emit downloadablesChanged();
}


void DownloadableGroupWatcher::checkAndEmitSignals()
{
    bool                          newDownloading           = downloading();
    QList<QPointer<Downloadable>> newDownloadablesWithFile = downloadablesWithFile();
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


QList<QPointer<Downloadable>> DownloadableGroupWatcher::downloadables() const
{
    QList<QPointer<Downloadable>> result;
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        result += _downloadable;
    }

    // Sort Downloadables according to lower boundary
    std::sort(result.begin(), result.end(), [](Downloadable* a, Downloadable* b)
    {
        if (a->section() != b->section())
            return (a->section() < b->section());
        return (a->fileName() < b->fileName());
    }
    );

    return result;
}


QList<QPointer<Downloadable>> DownloadableGroupWatcher::downloadablesWithFile() const
{
    QList<QPointer<Downloadable>> result;
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        if (!_downloadable->hasFile())
            continue;
        result += _downloadable;
    }

    // Sort Downloadables according to lower boundary
    std::sort(result.begin(), result.end(), [](Downloadable* a, Downloadable* b)
    {
        if (a->section() != b->section())
            return (a->section() < b->section());
        return (a->fileName() < b->fileName());
    }
    );

    return result;
}


QList<QObject*> DownloadableGroupWatcher::downloadablesAsObjectList() const
{
    QList<QObject*> result;
    foreach(auto downloadablePtr, downloadables())
        result.append(downloadablePtr);
    return result;
}


void DownloadableGroupWatcher::updateAll()
{
    foreach(auto downloadablePtr, _downloadables) {
        if (downloadablePtr.isNull())
            continue;
        if (downloadablePtr->updatable())
            downloadablePtr->startFileDownload();
    }
}


QString DownloadableGroupWatcher::updateSize() const
{
#warning need to implement
    return "";
}
