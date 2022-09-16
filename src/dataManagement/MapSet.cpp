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

#include "MapSet.h"


DataManagement::MapSet::MapSet(DataManagement::Downloadable* baseMap, DataManagement::Downloadable* terrainMap, QObject* parent)
    : m_baseMap(baseMap), m_terrainMap(terrainMap), QObject(parent)
{
    if (!m_baseMap.isNull()) {
        setObjectName(m_baseMap->objectName());
        m_section = m_baseMap->section();

        connect(m_baseMap, &DataManagement::Downloadable::error, this, &DataManagement::MapSet::error);
        connect(m_baseMap, &DataManagement::Downloadable::downloadingChanged, this, &DataManagement::MapSet::downloadingChanged);
        connect(m_baseMap, &DataManagement::Downloadable::hasFileChanged, this, &DataManagement::MapSet::hasFileChanged);
        connect(m_baseMap, &DataManagement::Downloadable::hasFileChanged, this, &DataManagement::MapSet::updatableChanged);
        connect(m_baseMap, &DataManagement::Downloadable::infoTextChanged, this, &DataManagement::MapSet::infoTextChanged);
        connect(m_baseMap, &DataManagement::Downloadable::updatableChanged, this, &DataManagement::MapSet::updatableChanged);
    }

    if (!m_terrainMap.isNull()) {
        setObjectName(m_terrainMap->objectName());
        m_section = m_terrainMap->section();

        connect(m_terrainMap, &DataManagement::Downloadable::error, this, &DataManagement::MapSet::error);
        connect(m_terrainMap, &DataManagement::Downloadable::downloadingChanged, this, &DataManagement::MapSet::downloadingChanged);
        connect(m_terrainMap, &DataManagement::Downloadable::hasFileChanged, this, &DataManagement::MapSet::hasFileChanged);
        connect(m_terrainMap, &DataManagement::Downloadable::hasFileChanged, this, &DataManagement::MapSet::updatableChanged);
        connect(m_terrainMap, &DataManagement::Downloadable::infoTextChanged, this, &DataManagement::MapSet::infoTextChanged);
        connect(m_terrainMap, &DataManagement::Downloadable::updatableChanged, this, &DataManagement::MapSet::updatableChanged);
    }

}


auto DataManagement::MapSet::downloading() const -> bool
{
    if (!m_baseMap.isNull() && m_baseMap->downloading())
    {
        return true;
    }

    if (!m_terrainMap.isNull() && m_terrainMap->downloading())
    {
        return true;
    }

    return false;
}


auto DataManagement::MapSet::hasFile() const -> bool
{
    if (!m_baseMap.isNull() && m_baseMap->hasFile())
    {
        return true;
    }

    if (!m_terrainMap.isNull() && m_terrainMap->hasFile())
    {
        return true;
    }

    return false;
}


auto DataManagement::MapSet::infoText() const -> QString
{
    QString result;

    if (!m_baseMap.isNull())
    {
        if (!result.isEmpty())
        {
            result += QLatin1String("<br>");
        }
        result += QStringLiteral("%1: %2").arg(tr("Base Map"), m_baseMap->infoText());
    }

    if (!m_terrainMap.isNull())
    {
        if (!result.isEmpty())
        {
            result += QLatin1String("<br>");
        }
        result += QStringLiteral("%1: %2").arg(tr("Terrain Map"), m_terrainMap->infoText());
    }

    return result;
}


auto DataManagement::MapSet::updatable() const -> bool
{
    if (!m_baseMap.isNull() && m_baseMap->updatable())
    {
        return true;
    }
    if (!m_terrainMap.isNull() && m_terrainMap->updatable())
    {
        return true;
    }

    if (!hasFile())
    {
        return false;
    }
    if (!m_baseMap.isNull() && !m_baseMap->hasFile())
    {
        return true;
    }
    if (!m_terrainMap.isNull() && !m_terrainMap->hasFile())
    {
        return true;
    }

    return false;
}


void DataManagement::MapSet::deleteFile()
{
    if (!m_baseMap.isNull())
    {
        m_baseMap->deleteFile();
    }
    if (!m_terrainMap.isNull())
    {
        m_terrainMap->deleteFile();
    }
}


void DataManagement::MapSet::startFileDownload()
{
    if (!m_baseMap.isNull())
    {
        if (!m_baseMap->hasFile() || m_baseMap->updatable())
        {
            m_baseMap->startFileDownload();
        }
    }

    if (!m_terrainMap.isNull())
    {
        if (!m_terrainMap->hasFile() || m_terrainMap->updatable())
        {
            m_terrainMap->startFileDownload();
        }
    }
}
