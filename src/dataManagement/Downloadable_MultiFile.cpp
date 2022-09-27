/***************************************************************************
 *   Copyright (C) 2022 by Stefan Kebekus                                  *
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

#include "Downloadable_MultiFile.h"


DataManagement::Downloadable_MultiFile::Downloadable_MultiFile(QObject* parent)
    : Downloadable_Abstract(parent)
{
}


auto DataManagement::Downloadable_MultiFile::description() -> QString
{
    QString result;

    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        switch(map->contentType())
        {
        case Downloadable_SingleFile::AviationMap:
            result += "<h4>"+tr("Aviation Map")+"</h4>";
            break;
        case Downloadable_SingleFile::BaseMapVector:
            result += "<h4>"+tr("Base Map (Vector)")+"</h4>";
            break;
        case Downloadable_SingleFile::BaseMapRaster:
            result += "<h4>"+tr("Base Map (Raster)")+"</h4>";
            break;
        case Downloadable_SingleFile::Data:
            result += "<h4>"+tr("Data")+"</h4>";
            break;
        case Downloadable_SingleFile::TerrainMap:
            result += "<h4>"+tr("Terrain Map")+"</h4>";
            break;
        case Downloadable_SingleFile::MapSet:
            result += "<h4>"+tr("Map Set")+"</h4>";
            break;
        }
        result += map->description();
    }
    return result;
}


auto DataManagement::Downloadable_MultiFile::downloading() -> bool
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (map->downloading())
        {
            return true;
        }
    }
    return false;
}


auto DataManagement::Downloadable_MultiFile::hasFile() -> bool
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (map->hasFile())
        {
            return true;
        }
    }
    return false;
}


auto DataManagement::Downloadable_MultiFile::infoText() -> QString
{
    QString result;

    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
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
            result += QStringLiteral("%1: %2").arg(tr("Base Map (Raster)"), map->infoText());
            break;
        case Downloadable_SingleFile::BaseMapVector:
            result += QStringLiteral("%1: %2").arg(tr("Base Map (Vector)"), map->infoText());
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


auto DataManagement::Downloadable_MultiFile::updatable() -> bool
{
    if (!hasFile())
    {
        return false;
    }

    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (map->updatable() || !map->hasFile())
        {
            return true;
        }
    }
    return false;
}


auto DataManagement::Downloadable_MultiFile::updatableSize() -> qsizetype
{
    if (!updatable())
    {
        return 0;
    }

    qsizetype result = 0;
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (map->updatable() || !map->hasFile())
        {
            result += map->remoteFileSize();
        }
    }
    return result;
}


void DataManagement::Downloadable_MultiFile::deleteFile()
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        map->deleteFile();
    }
}


void DataManagement::Downloadable_MultiFile::startFileDownload()
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        map->startFileDownload();
    }
}


void DataManagement::Downloadable_MultiFile::stopFileDownload()
{
    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        map->stopFileDownload();
    }
}


void DataManagement::Downloadable_MultiFile::update()
{
    if (!updatable())
    {
        return;
    }

    m_maps.removeAll(nullptr);
    foreach(auto map, m_maps)
    {
        if (map->updatable() || !map->hasFile())
        {
            map->startFileDownload();
        }
    }
}


void DataManagement::Downloadable_MultiFile::add(DataManagement::Downloadable_SingleFile* map)
{
    if ((map == nullptr) || m_maps.contains(map))
    {
        return;
    }

    m_maps.append(map);

    setObjectName(map->objectName());
    m_section = map->section();

    connect(map, &DataManagement::Downloadable_SingleFile::error, this, &DataManagement::Downloadable_MultiFile::error);
    connect(map, &DataManagement::Downloadable_SingleFile::downloadingChanged, this, &DataManagement::Downloadable_MultiFile::downloadingChanged);
    connect(map, &DataManagement::Downloadable_SingleFile::fileContentChanged, this, &DataManagement::Downloadable_MultiFile::descriptionChanged);
    connect(map, &DataManagement::Downloadable_SingleFile::hasFileChanged, this, &DataManagement::Downloadable_MultiFile::hasFileChanged);
    connect(map, &DataManagement::Downloadable_SingleFile::hasFileChanged, this, &DataManagement::Downloadable_MultiFile::updatableChanged);
    connect(map, &DataManagement::Downloadable_SingleFile::infoTextChanged, this, &DataManagement::Downloadable_MultiFile::infoTextChanged);
    connect(map, &DataManagement::Downloadable_SingleFile::updatableChanged, this, &DataManagement::Downloadable_MultiFile::updatableChanged);
    connect(map, &DataManagement::Downloadable_SingleFile::updatableChanged, this, &DataManagement::Downloadable_MultiFile::updatableSizeChanged);
    connect(map, &DataManagement::Downloadable_SingleFile::hasFileChanged, this, &DataManagement::Downloadable_MultiFile::updatableSizeChanged);

#warning This emits too many signals
}
