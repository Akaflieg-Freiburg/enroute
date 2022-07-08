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

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>

#include "geomaps/MBTILES.h"

GeoMaps::MBTILES::MBTILES(const QString& fileName)
    : m_fileName(fileName)
{
    m_databaseConnectionName = QStringLiteral("GeoMaps::MBTILES::format %1,%2").arg(fileName).arg((quintptr)this);
    auto m_dataBase = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), m_databaseConnectionName);
    m_dataBase.setDatabaseName(fileName);
    m_dataBase.open();

    QSqlQuery query(m_dataBase);
    if (query.exec(QStringLiteral("select name, value from metadata;")))
    {
        while(query.next())
        {
            QString key = query.value(0).toString();
            QString value = query.value(1).toString();
            m_metadata.insert(key, value);
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
            if (format == QLatin1String("pbf"))
            {
                return Vector;
            }
            if ((format == QLatin1String("jpg")) || (format == QLatin1String("png")) || (format == QLatin1String("webp")))
            {
                return Raster;
            }
        }
    }
    return Unknown;
}

auto GeoMaps::MBTILES::info() -> QString
{
    QString result;

    // Read metadata from database
    auto m_dataBase = QSqlDatabase::database(m_databaseConnectionName);
    QSqlQuery query(m_dataBase);
    QString intResult;
    if (query.exec(QStringLiteral("select name, value from metadata;")))
    {
        while (query.next())
        {
            QString key = query.value(0).toString();
            if (key == u"json")
            {
                continue;
            }
            intResult += QStringLiteral("<tr><td><strong>%1 :&nbsp;&nbsp;</strong></td><td>%2</td></tr>")
                    .arg(key, query.value(1).toString());
        }
    }
    if (!intResult.isEmpty())
    {
        result += QStringLiteral("<h4>%1</h4><table>%2</table>").arg(QObject::tr("Internal Map Data", "GeoMaps::MBTILES"), intResult);
    }

    return result;
}

auto GeoMaps::MBTILES::tile(int zoom, int x, int y) -> QByteArray
{
    auto m_dataBase = QSqlDatabase::database(m_databaseConnectionName);
    QSqlQuery query(m_dataBase);
    auto yflipped = (1<<zoom)-1-y;
    auto queryString = QStringLiteral("select tile_data from tiles where zoom_level='%1' and tile_row='%2' and tile_column='%3';").arg(zoom, x, yflipped);
    if (query.exec(queryString))
    {
        if (query.first())
        {
            return query.value(0).toByteArray();
        }
    }

    return {};
}
