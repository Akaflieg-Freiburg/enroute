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

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QRandomGenerator>
#include <QVariant>

#include "fileFormats/DataFileAbstract.h"
#include "geomaps/MBTILES.h"

GeoMaps::MBTILES::MBTILES(const QString& fileName, QObject *parent)
    : QObject(parent), m_fileName(fileName)
{
    m_file = FileFormats::DataFileAbstract::openFileURL(fileName);

    m_databaseConnectionName = QStringLiteral("GeoMaps::MBTILES::format %1,%2").arg(fileName).arg(QRandomGenerator::global()->generate());
    auto m_dataBase = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_databaseConnectionName);
    m_dataBase.setDatabaseName(m_file->fileName());
    m_dataBase.open();

    QSqlQuery query(m_dataBase);
    if (query.exec(QStringLiteral("select name, value from metadata;")))
    {
        while(query.next())
        {
            QString const key = query.value(0).toString();
            QString const value = query.value(1).toString();
            m_metadata.insert(key, value);
        }
    }

    // If the metadata does not contain a minzoom entry, then generate one by looking at the
    // lowest zoom_level that exists in the "tiles" table.
    if (!m_metadata.contains(u"minzoom"_qs))
    {
        if (query.exec(QStringLiteral("select min(zoom_level) from tiles;")))
        {
            while(query.next())
            {
                QString const minzoom = query.value(0).toString();
                m_metadata.insert(u"minzoom"_qs, minzoom);
            }
        }
    }

    // If the metadata does not contain a maxzoom entry, then generate one by looking at the
    // highest zoom_level that exists in the "tiles" table.
    if (!m_metadata.contains(u"maxzoom"_qs))
    {
        if (query.exec(QStringLiteral("select max(zoom_level) from tiles;")))
        {
            while(query.next())
            {
                QString const maxzoom = query.value(0).toString();
                m_metadata.insert(u"maxzoom"_qs, maxzoom);
            }
        }
    }
}

GeoMaps::MBTILES::~MBTILES()
{
    QSqlDatabase::database(m_databaseConnectionName).close();
    QSqlDatabase::removeDatabase(m_databaseConnectionName);
}

auto GeoMaps::MBTILES::attribution() -> QString
{
    auto m_dataBase = QSqlDatabase::database(m_databaseConnectionName);
    QSqlQuery query(m_dataBase);
    if (query.exec(QStringLiteral("select name, value from metadata where name='attribution';")))
    {
        if (query.first())
        {
            return query.value(1).toString();
        }
    }
    return {};
}

auto GeoMaps::MBTILES::format() -> GeoMaps::MBTILES::Format
{
    auto m_dataBase = QSqlDatabase::database(m_databaseConnectionName);
    QSqlQuery query(m_dataBase);
    if (query.exec(QStringLiteral("select name, value from metadata where name='format';")))
    {
        if (query.first())
        {
            auto format = query.value(1).toString();
            if (format == u"pbf"_qs)
            {
                return Vector;
            }
            if ((format == u"jpg"_qs) || (format == u"png"_qs) || (format == u"webp"_qs))
            {
                return Raster;
            }
        }
    }
    return Unknown;
}

auto GeoMaps::MBTILES::info() -> QString
{
    QMapIterator<QString, QString> i(m_metadata);
    QString intResult;
    while (i.hasNext()) {
        i.next();

        if (i.key() == u"json")
        {
            continue;
        }
        intResult += QStringLiteral("<tr><td><strong>%1 :&nbsp;&nbsp;</strong></td><td>%2</td></tr>")
                         .arg(i.key(), i.value());
    }

    QString result;
    if (!intResult.isEmpty())
    {
        result += QStringLiteral("<table>%1</table>").arg(intResult);
    }
    return result;
}


auto GeoMaps::MBTILES::tile(int zoom, int x, int y) -> QByteArray
{
    auto m_dataBase = QSqlDatabase::database(m_databaseConnectionName);
    QSqlQuery query(m_dataBase);
    auto yflipped = (1<<zoom)-1-y;
    auto queryString = QStringLiteral("select tile_data from tiles where zoom_level=%1 and tile_row=%3 and tile_column=%2;").arg(zoom).arg(x).arg(yflipped);
    if (query.exec(queryString))
    {
        if (query.first())
        {
            return query.value(0).toByteArray();
        }
    }

    return {};
}
