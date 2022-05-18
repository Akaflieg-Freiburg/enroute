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

auto GeoMaps::MBTILES::attribution(const QString& fileName) -> QString
{
    QString result;

    auto databaseConnectionName = "GeoMaps::MBTILES::format " + fileName;
    { // Parenthesis necessary, because testDB needs to be deconstructed before QSqlDatabase::removeDatabase is called
        auto testDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), databaseConnectionName);
        testDB.setDatabaseName(fileName);

        if (testDB.open()) {
            QSqlQuery query(testDB);
            if (query.exec(QStringLiteral("select name, value from metadata where name='attribution';"))) {
                if (query.first()) {
                    result = query.value(1).toString();
                }
            }
            testDB.close();
        }
    }
    QSqlDatabase::removeDatabase(databaseConnectionName);

    return result;
}

auto GeoMaps::MBTILES::format(const QString& fileName) -> GeoMaps::MBTILES::Format
{
    GeoMaps::MBTILES::Format result = Unknown;

    auto databaseConnectionName = "GeoMaps::MBTILES::format " + fileName;
    { // Parenthesis necessary, because testDB needs to be deconstructed before QSqlDatabase::removeDatabase is called
        auto testDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), databaseConnectionName);
        testDB.setDatabaseName(fileName);

        if (testDB.open()) {
            QSqlQuery query(testDB);
            if (query.exec(QStringLiteral("select name, value from metadata where name='format';"))) {
                if (query.first()) {
                    auto format = query.value(1).toString();
                    if (format == QLatin1String("pbf")) {
                        result = Vector;
                    }
                    if ((format == QLatin1String("jpg")) || (format == QLatin1String("png")) || (format == QLatin1String("webp"))) {
                        result = Raster;
                    }
                }
            }
            testDB.close();
        }
    }
    QSqlDatabase::removeDatabase(databaseConnectionName);

    return result;
}


auto GeoMaps::MBTILES::info(const QString& fileName) -> QString
{
    QString result;

    auto databaseConnectionName = "GeoMaps::MBTILES::info " + fileName;
    { // Parenthesis necessary, because testDB needs to be deconstructed before QSqlDatabase::removeDatabase is called
        auto testDB = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), databaseConnectionName);
        testDB.setDatabaseName(fileName);
        if (testDB.open())
        {
            // Read metadata from database
            QSqlQuery query(testDB);
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
            testDB.close();
        }
    }
    QSqlDatabase::removeDatabase(databaseConnectionName);
    return result;
}
