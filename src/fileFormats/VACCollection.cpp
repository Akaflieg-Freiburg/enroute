/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

#include <QFileInfo>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "fileFormats/VACCollection.h"

using namespace Qt::Literals::StringLiterals;


FileFormats::VACCollection::VACCollection(const QString& fileName)
    : m_fileName(QFileInfo(fileName).absoluteFilePath())
{
    m_file = openFileURL(fileName);

    m_databaseConnectionName = QStringLiteral("FileFormats::VACCollection %1,%2").arg(fileName).arg(QRandomGenerator::global()->generate());
    auto dataBase = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_databaseConnectionName);
    dataBase.setDatabaseName(m_file->fileName());
    if (!dataBase.open())
    {
        setError(QObject::tr("Unable to open database connection to VAC collection file.", "FileFormats::VACCollection"));
        return;
    }

    // Read the metadata table. This fails for files that are not SQLite
    // databases or do not follow the schema.
    QSqlQuery query(dataBase);
    if (!query.exec(QStringLiteral("select key, value from metadata;")))
    {
        setError(QObject::tr("Unable to read metadata table from VAC collection file.", "FileFormats::VACCollection"));
        return;
    }
    QMap<QString, QString> metadata;
    while (query.next())
    {
        metadata.insert(query.value(0).toString(), query.value(1).toString());
    }
    if (metadata.value(u"schemaVersion"_s) != u"1"_s)
    {
        setError(QObject::tr("VAC collection file has unsupported schema version.", "FileFormats::VACCollection"));
        return;
    }
    m_name = metadata.value(u"name"_s);
    if (m_name.isEmpty())
    {
        m_name = QFileInfo(m_fileName).baseName();
    }

    // Read the chart index. The column 'image' is deliberately not selected
    // here, so the database never touches the raster data and this constructor
    // remains lightweight.
    if (!query.exec(QStringLiteral("select name, topLeftLat, topLeftLon, topRightLat, topRightLon, "
                                   "bottomLeftLat, bottomLeftLon, bottomRightLat, bottomRightLon from charts;")))
    {
        setError(QObject::tr("Unable to read charts table from VAC collection file.", "FileFormats::VACCollection"));
        return;
    }
    while (query.next())
    {
        GeoMaps::VAC vac;
        vac.name = query.value(0).toString();
        vac.fileName = m_fileName;
        vac.collection = m_name;
        vac.topLeft = {query.value(1).toDouble(), query.value(2).toDouble()};
        vac.topRight = {query.value(3).toDouble(), query.value(4).toDouble()};
        vac.bottomLeft = {query.value(5).toDouble(), query.value(6).toDouble()};
        vac.bottomRight = {query.value(7).toDouble(), query.value(8).toDouble()};
        if (!vac.isValid())
        {
            addWarning(QObject::tr("Skipping chart entry '%1', which is invalid.", "FileFormats::VACCollection").arg(vac.name));
            continue;
        }
        m_charts.append(vac);
    }
    if (m_charts.isEmpty())
    {
        setError(QObject::tr("VAC collection file contains no valid charts.", "FileFormats::VACCollection"));
    }
}

FileFormats::VACCollection::~VACCollection()
{
    QSqlDatabase::database(m_databaseConnectionName).close();
    QSqlDatabase::removeDatabase(m_databaseConnectionName);
}

QByteArray FileFormats::VACCollection::imageData(const QString& chartName)
{
    auto dataBase = QSqlDatabase::database(m_databaseConnectionName);
    if (!dataBase.open())
    {
        return {};
    }
    QSqlQuery query(dataBase);
    query.prepare(QStringLiteral("select image from charts where name=?;"));
    query.addBindValue(chartName);
    if (query.exec() && query.first())
    {
        return query.value(0).toByteArray();
    }
    return {};
}
