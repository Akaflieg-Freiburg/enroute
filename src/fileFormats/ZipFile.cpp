/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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

#include <QDataStream>
#include <QFile>
#include <QString>
#include <zip.h>

#include "fileFormats/ZipFile.h"


FileFormats::ZipFile::ZipFile(const QString& fileName)
{
    m_file = openFileURL(fileName);

    int error = 0;
    m_zip = zip_open(m_file->fileName().toUtf8().data(), ZIP_RDONLY, &error);
    if (m_zip == nullptr)
    {
        setError(QObject::tr("Cannot open zip file %1 for reading.", "FileFormats::ZipFile").arg(fileName));
        return;
    }

    // Get number of files in this archive
    auto numEntries = zip_get_num_entries(static_cast<zip_t*>(m_zip), 0);
    m_fileNames.reserve(numEntries);
    m_fileSizes.reserve(numEntries);

    // Read info for every file
    for(auto i=0; i<numEntries; i++)
    {
        struct zip_stat zStat {};
        auto error = zip_stat_index(static_cast<zip_t*>(m_zip), i, 0, &zStat);
        if (error != 0)
        {
            return;
        }
        if ( ((zStat.valid&ZIP_STAT_NAME) == 0) || ((zStat.valid&ZIP_STAT_SIZE) == 0))
        {
            return;
        }
        if (zStat.size > static_cast<zip_uint64_t>(maxEntrySize))
        {
            setError(QObject::tr("Zip file %1 contains an entry that is too large.", "FileFormats::ZipFile").arg(fileName));
            return;
        }
        m_fileNames += QString::fromUtf8(zStat.name);
        m_fileSizes += static_cast<qsizetype>(zStat.size);
    }
}


FileFormats::ZipFile::~ZipFile()
{
    if (m_zip != nullptr)
    {
        zip_close( static_cast<zip_t*>(m_zip));
        m_zip = nullptr;
    }
    m_file.clear();
}


QByteArray FileFormats::ZipFile::extract(qsizetype index)
{
    if (m_zip == nullptr)
    {
        return {};
    }
    if ((index < 0) || (index >= m_fileNames.size()))
    {
        return {};
    }

    auto* zipFile = zip_fopen_index(static_cast<zip_t*>(m_zip), index, 0);
    if (zipFile == nullptr)
    {
        return {};
    }

    // Read in chunks instead of allocating the size announced in the archive
    // headers up front. The announced size only serves as a consistency check.
    const auto expectedSize = m_fileSizes.at(index);
    QByteArray data;
    QByteArray buffer(1024*1024, Qt::Uninitialized);
    bool ok = true;
    while (true)
    {
        auto numBytesRead = zip_fread(zipFile, buffer.data(), buffer.size());
        if (numBytesRead < 0)
        {
            ok = false;
            break;
        }
        if (numBytesRead == 0)
        {
            break;
        }
        if (data.size() + numBytesRead > maxEntrySize)
        {
            ok = false;
            break;
        }
        data.append(buffer.constData(), static_cast<qsizetype>(numBytesRead));
    }
    zip_fclose(zipFile);

    if (!ok || (data.size() != expectedSize))
    {
        return {};
    }
    return data;
}


QByteArray FileFormats::ZipFile::extract(const QString& fileName)
{
    auto idx = m_fileNames.indexOf(fileName);
    if (idx == -1)
    {
        auto windowsStyleFileName = fileName;
        windowsStyleFileName.replace('/', '\\');
        idx = m_fileNames.indexOf(windowsStyleFileName);
    }
    return extract(idx);
}
