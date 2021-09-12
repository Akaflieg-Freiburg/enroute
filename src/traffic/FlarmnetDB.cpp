/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include "Global.h"
#include "geomaps/MapManager.h"
#include "traffic/FlarmnetDB.h"


Traffic::FlarmnetDB::FlarmnetDB(QObject* parent) : QObject(parent)
{
    QTimer::singleShot(0, this, &Traffic::FlarmnetDB::deferredInitialization);
}


void Traffic::FlarmnetDB::clearCache()
{
    m_cache.clear();
}


void Traffic::FlarmnetDB::deferredInitialization()
{
    connect(Global::mapManager()->databases(), &GeoMaps::DownloadableGroupWatcher::downloadablesChanged, this, &Traffic::FlarmnetDB::findFlarmnetDBDownloadable);
    findFlarmnetDBDownloadable();
}


void Traffic::FlarmnetDB::findFlarmnetDBDownloadable()
{
    // Find correct downloadable. We do this only if a QCoreApplication
    // exists, in order to avoid calling Global::mapManager during shutdown.
    QPointer<GeoMaps::Downloadable> newFlarmnetDBDownloadable;
    if (QCoreApplication::instance() != nullptr) {
        auto downloadables = Global::mapManager()->databases()->downloadables();
        foreach(auto downloadable, downloadables) {
            if (downloadable->fileName().contains("Flarmnet")) {
                newFlarmnetDBDownloadable = downloadable;
                break;
            }
        }
    }


    if (flarmnetDBDownloadable == newFlarmnetDBDownloadable) {
        return;
    }

    if (flarmnetDBDownloadable != nullptr) {
        disconnect(flarmnetDBDownloadable, &GeoMaps::Downloadable::fileContentChanged, this, &Traffic::FlarmnetDB::clearCache);
    }

    flarmnetDBDownloadable = newFlarmnetDBDownloadable;
    if (flarmnetDBDownloadable != nullptr) {
        connect(flarmnetDBDownloadable, &GeoMaps::Downloadable::fileContentChanged, this, &Traffic::FlarmnetDB::clearCache);

        // Create an empty file, if no file exists. We set the FileModificationTime
        // to a point in the past, so that it will automatically be updated at the
        // next convenience.
        if (!QFile::exists(flarmnetDBDownloadable->fileName())) {
            QFile dataFile(flarmnetDBDownloadable->fileName());
            dataFile.open(QIODevice::WriteOnly);
            dataFile.write(tr("Placeholder file.").toLatin1());
            dataFile.flush();
            dataFile.setFileTime(QDateTime( QDate(2021, 8, 21), QTime(13, 0)), QFileDevice::FileModificationTime);
        }

    }
    clearCache();

}


auto Traffic::FlarmnetDB::getRegistration(const QString& key) -> QString
{
    // Check if key exists in the cache
    auto* cachedValue = m_cache[key];
    if (cachedValue != nullptr) {
        return *cachedValue;
    }

    // If not in the cache, try to find the values in the file.
    if (flarmnetDBDownloadable == nullptr) {
        m_cache.insert(key, new QString());
        return {};
    }

    QFile dataFile(flarmnetDBDownloadable->fileName());
    if (!dataFile.open(QIODevice::ReadOnly)) {
        dataFile.open(QIODevice::WriteOnly);
        dataFile.setFileTime(QDateTime( QDate(2021, 8, 21), QTime(13, 0)), QFileDevice::FileModificationTime);

        m_cache.insert(key, new QString());
        return {};
    }
    dataFile.readLine();

    // Now to a binary search in the file
    qint64 lineSize   = 24;
    qint64 firstEntry = dataFile.pos();
    qint64 numEntries = (dataFile.size()-firstEntry)/lineSize;

    auto getKey = [firstEntry, lineSize](QFile& dataFile, qint64 entry) {
        dataFile.seek(firstEntry + entry*lineSize);
        return QString::fromLatin1(dataFile.readLine(6));
    };
    auto getVal = [firstEntry, lineSize](QFile& dataFile, qint64 entry) {
        dataFile.seek(firstEntry + entry*lineSize + 7);
        return QString::fromLatin1(dataFile.readLine(16)).simplified();
    };

    qint64 startIndex = 0;
    qint64 endIndex   = numEntries-1;
    do{
        if (getKey(dataFile, startIndex) == key) {
            auto value = getVal(dataFile, startIndex);
            m_cache.insert(key, new QString(value));
            return value;
        }
        if (getKey(dataFile, endIndex) == key) {
            auto value = getVal(dataFile, endIndex);
            m_cache.insert(key, new QString(value));
            return value;
        }
        qint64 midIndex = (startIndex + endIndex)/2;
        if (midIndex == startIndex) {
            return {};
        }
        if (getKey(dataFile, endIndex) > key) {
            endIndex = midIndex;
        } else {
            startIndex = midIndex;
        }
    }while(startIndex != endIndex);

    m_cache.insert(key, new QString());
    return {};
}
