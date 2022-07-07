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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QSqlError>
#include <QSqlQuery>

#include <qhttpengine/socket.h>

#include "TileHandler.h"
#include "dataManagement/Downloadable.h"

QRegularExpression tileQueryPattern(QStringLiteral("[0-9]{1,2}/[0-9]{1,4}/[0-9]{1,4}"));

GeoMaps::TileHandler::TileHandler(const QVector<QPointer<DataManagement::Downloadable>>& mbtileFiles, const QString& baseURL, QObject *parent)
    : Handler(parent)
{
    // Go through mbtile files and find real values
    foreach (auto mbtileFile, mbtileFiles)
    {
        // Check that file really exists
        if (!QFile::exists(mbtileFile->fileName()))
        {
            hasDBError = true;
            return;
        }
        connect(mbtileFile, &DataManagement::Downloadable::aboutToChangeFile, this, &TileHandler::removeFile);

        // Open database
        auto databaseConnectionName = baseURL+"-"+mbtileFile->fileName();
        auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), databaseConnectionName);
        db.setDatabaseName(mbtileFile->fileName());
        db.open();
        if (db.isOpenError())
        {
            hasDBError = true;
            return;
        }
        databaseConnections += databaseConnectionName;

        // Read metadata from database
        QSqlQuery query(db);
        if (!query.exec(QStringLiteral("select name, value from metadata;")))
        {
            hasDBError = true;
            return;
        }
        while(query.next())
        {
            QString key = query.value(0).toString();
            if (key == QLatin1String("name")) {
                _name = query.value(1).toString();
            }
            if (key == QLatin1String("encoding")) {
                _encoding = query.value(1).toString();
            }
            if (key == QLatin1String("format")) {
                _format = query.value(1).toString();
            }
            if (key == QLatin1String("description")) {
                _description = query.value(1).toString();
            }
            if (key == QLatin1String("version")) {
                _version = query.value(1).toString();
            }
            if (key == QLatin1String("attribution")) {
                _attribution = query.value(1).toString();
            }
            if (key == QLatin1String("maxzoom")) {
               _maxzoom = query.value(1).toInt();
            }
            if (key == QLatin1String("minzoom")) {
                _minzoom = query.value(1).toInt();
            }
        }
        _tiles = baseURL+"/{z}/{x}/{y}."+_format;

        // Safety check
        if (_minzoom > _maxzoom)
        {
            _maxzoom = -1;
            _minzoom = -1;
        }
    }
}


GeoMaps::TileHandler::~TileHandler()
{
    foreach(auto databaseConnectionName, databaseConnections)
        QSqlDatabase::removeDatabase(databaseConnectionName);
}


void GeoMaps::TileHandler::removeFile(const QString& localFileName)
{
    QString connectionToRemove;
    foreach(auto databaseConnectionName, databaseConnections)
    {
        if (!databaseConnectionName.endsWith(localFileName))
        {
            continue;
        }
        connectionToRemove = databaseConnectionName;
        break;
    }

    QSqlDatabase::removeDatabase(connectionToRemove);
    databaseConnections.remove(connectionToRemove);
}


void GeoMaps::TileHandler::process(QHttpEngine::Socket *socket, const QString &path)
{
    // Serve tileJSON file, if requested
    if (path.isEmpty() || path.endsWith(QLatin1String("json"), Qt::CaseInsensitive))
    {
        socket->setHeader("Content-Type", "application/json");
        QByteArray json = tileJSON();
        socket->setHeader("Content-Length", QByteArray::number(json.length()));
        socket->write(json);
        socket->close();
        return;
    }

    // Serve tile, if requested
    QRegularExpressionMatch match = tileQueryPattern.match(path);
    if (match.hasMatch())
    {
        // Retrieve tile data from the database
        quint32 z       = path.section('/', 1, 1).toInt();
        QString x       = path.section('/', 2, 2);
        quint32 y       = path.section('/', 3, 3).section('.', 0, 0).toInt();
        quint32 yflipped = ((quint32(1) <<z)-1)-y;
        QString queryString = QStringLiteral("select zoom_level, tile_column, tile_row, tile_data from tiles where zoom_level=%1 and tile_column=%2 and tile_row=%3;").arg(z).arg(x).arg(yflipped);

        foreach(auto databaseConnection, databaseConnections)
        {
            auto db = QSqlDatabase::database(databaseConnection);

            QSqlQuery query(db);
            query.exec(queryString);

            // Error handling
            if (!query.first())
            {
                continue;
            }

            // Get data
            QByteArray tileData = query.value(3).toByteArray();

            // Set the headers and write the content
            socket->setHeader("Content-Type", "application/octet-stream");
            if (_format == QLatin1String("pbf"))
            {
                socket->setHeader("Content-Encoding", "gzip");
            }
            socket->setHeader("Content-Length", QByteArray::number(tileData.length()));
            socket->write(tileData);
            socket->close();
            return;
        }
    }

    // Unknown request, responding with 'not found'
    socket->writeError(QHttpEngine::Socket::NotFound);
    socket->close();
}


auto GeoMaps::TileHandler::tileJSON() const -> QByteArray
{
    QJsonObject result;
    result.insert(QStringLiteral("tilejson"), "2.2.0");

    // Insert tiles
    QJsonArray tiles;
    tiles.append(_tiles);
    result.insert(QStringLiteral("tiles"), tiles);

    if (!_name.isEmpty())
    {
        result.insert(QStringLiteral("name"), _name);
    }
    if (!_description.isEmpty())
    {
        result.insert(QStringLiteral("description"), _description);
    }
    if (!_encoding.isEmpty())
    {
        result.insert(QStringLiteral("encoding"), _encoding);
    }
    if (!_format.isEmpty())
    {
        result.insert(QStringLiteral("format"), _format);
    }
    if (!_version.isEmpty())
    {
        result.insert(QStringLiteral("version"), _version);
    }
    if (!_attribution.isEmpty())
    {
        result.insert(QStringLiteral("attribution"), _attribution);
    }
    if (_maxzoom >= 0)
    {
        result.insert(QStringLiteral("maxzoom"), _maxzoom);
    }
    if (_minzoom >= 0)
    {
        result.insert(QStringLiteral("minzoom"), _minzoom);
    }

    QJsonDocument tileJSONDocument;
    tileJSONDocument.setObject(result);
    return tileJSONDocument.toJson();
}
