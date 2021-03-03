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
#include "geomaps/Downloadable.h"

GeoMaps::TileHandler::TileHandler(const QVector<QPointer<Downloadable>>& mbtileFiles, const QString& baseURL, QObject *parent)
    : Handler(parent)
{
    // Initialize with default values
    _name        = "empty";
    _format      = "pbf";
    _description = "empty tile set";
    _version     = "3.6.1";
    _attribution = "";
    _maxzoom     = 10;
    _minzoom     = 0;
    _tiles       = baseURL+"/{z}/{x}/{y}."+_format;

    // Go through mbtile files and find real values
    foreach (auto mbtileFile, mbtileFiles) {
        // Check that file really exists
        if (!QFile::exists(mbtileFile->fileName())) {
            hasDBError = true;
            return;
        }
        connect(mbtileFile, &Downloadable::aboutToChangeFile, this, &TileHandler::removeFile);

        // Open database
        auto databaseConnectionName = baseURL+"-"+mbtileFile->fileName();
        auto db = QSqlDatabase::addDatabase("QSQLITE", databaseConnectionName);
        db.setDatabaseName(mbtileFile->fileName());
        db.open();
        if (db.isOpenError()) {
            hasDBError = true;
            return;
        }
        databaseConnections += databaseConnectionName;

        // Read metadata from database
        QSqlQuery query(db);
        if (!query.exec("select name, value from metadata;")) {
            hasDBError = true;
            return;
        }
        while(query.next()) {
            QString key = query.value(0).toString();
            if (key == "name") {
                _name = query.value(1).toString();
            }
            if (key == "format") {
                _format= query.value(1).toString();
            }
            if (key == "description") {
                _description = query.value(1).toString();
            }
            if (key == "version") {
                _version = query.value(1).toString();
            }
            if (key == "attribution") {
                _attribution = query.value(1).toString();
            }
            if (key == "maxzoom") {
               _maxzoom = query.value(1).toInt();
            }
            if (key == "minzoom") {
                _minzoom = query.value(1).toInt();
            }
        }
        _tiles = baseURL+"/{z}/{x}/{y}."+_format;

        // Safety check
        if (_minzoom > _maxzoom) {
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
    foreach(auto databaseConnectionName, databaseConnections) {
        if (!databaseConnectionName.endsWith(localFileName)) {
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
    if (path.isEmpty() || path.endsWith("json", Qt::CaseInsensitive)) {
        socket->setHeader("Content-Type", "application/json");
        QByteArray json = tileJSON();
        socket->setHeader("Content-Length", QByteArray::number(json.length()));
        socket->write(json);
        socket->close();
        return;
    }

    // Serve tile, if requested
    QRegularExpression tileQueryPattern("[0-9]{1,2}/[0-9]{1,4}/[0-9]{1,4}");
    QRegularExpressionMatch match = tileQueryPattern.match(path);
    if (match.hasMatch()) {
        // Retrieve tile data from the database
        quint32 z       = path.section('/', 1, 1).toInt();
        QString x       = path.section('/', 2, 2);
        quint32 y       = path.section('/', 3, 3).section('.', 0, 0).toInt();
        quint32 yflipped = ((quint32(1) <<z)-1)-y;
        QString queryString = QString("select zoom_level, tile_column, tile_row, tile_data from tiles where zoom_level=%1 and tile_column=%2 and tile_row=%3;").arg(z).arg(x).arg(yflipped);

        foreach(auto databaseConnection, databaseConnections) {
            auto db = QSqlDatabase::database(databaseConnection);

            QSqlQuery query(db);
            query.exec(queryString);

            // Error handling
            if (!query.first()) {
                continue;
            }

            // Get data
            QByteArray tileData = query.value(3).toByteArray();

            // Set the headers and write the content
            socket->setHeader("Content-Type", "application/octet-stream");
            socket->setHeader("Content-Encoding", "gzip");
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
    result.insert("tilejson", "2.2.0");

    // Insert tiles
    QJsonArray tiles;
    tiles.append(_tiles);
    result.insert("tiles", tiles);

    if (!_name.isEmpty()) {
        result.insert("name", _name);
    }
    if (!_description.isEmpty()) {
        result.insert("description", _description);
    }
    if (!_version.isEmpty()) {
        result.insert("version", _version);
    }
    if (!_attribution.isEmpty()) {
        result.insert("attribution", _attribution);
    }
    if (_maxzoom >= 0) {
        result.insert("maxzoom", _maxzoom);
    }
    if (_minzoom >= 0) {
        result.insert("minzoom", _minzoom);
    }

    QJsonDocument tileJSONDocument;
    tileJSONDocument.setObject(result);
    return tileJSONDocument.toJson();
}
