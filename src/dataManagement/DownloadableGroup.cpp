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

#include "DownloadableGroup.h"


DataManagement::DownloadableGroup::DownloadableGroup(QObject *parent)
    : DownloadableGroupWatcher(parent)
{
}


void DataManagement::DownloadableGroup::addToGroup(Downloadable* downloadable)
{
    // Avoid double entries
    if (_downloadables.contains(downloadable))
    {
        return;
    }

    // Add element to group
    _downloadables.append(downloadable);
    std::sort(_downloadables.begin(), _downloadables.end(), [](Downloadable* a, Downloadable* b)
    {
        if (a->section() != b->section()) {
            return (a->section() < b->section());
        }
        return (a->fileName() < b->fileName());
    }
    );


    connect(downloadable, &Downloadable::downloadingChanged, this, &DownloadableGroup::checkAndEmitSignals);
    connect(downloadable, &Downloadable::updatableChanged, this, &DownloadableGroup::checkAndEmitSignals);
    connect(downloadable, &Downloadable::hasFileChanged, this, &DownloadableGroup::checkAndEmitSignals);
    connect(downloadable, &Downloadable::fileContentChanged, this, &DownloadableGroup::localFileContentChanged);
    connect(downloadable, &QObject::destroyed, this, &DownloadableGroup::cleanUp);
    checkAndEmitSignals();

    emit downloadablesChanged();
    if (downloadable->hasFile())
    {
        emit localFileContentChanged();
    }
}


void DataManagement::DownloadableGroup::removeFromGroup(Downloadable *downloadable)
{
    auto index = _downloadables.indexOf(downloadable);

    // Avoid double entries
    if (index < 0) {
        return;
    }

    _downloadables.takeAt(index);
    disconnect(downloadable, nullptr, this, nullptr);
    checkAndEmitSignals();
    emit downloadablesChanged();
}
