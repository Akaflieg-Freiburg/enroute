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
            return;
        }
        connect(mbtileFile, &DataManagement::Downloadable::aboutToChangeFile, this, &TileHandler::removeFile);

        auto* mbtPtr = new GeoMaps::MBTILES(mbtileFile->fileName());
        m_mbtiles.insert(mbtPtr);

        _name = mbtPtr->metaData().value("name");
        _encoding = mbtPtr->metaData().value("encoding");
        _format = mbtPtr->metaData().value("format");
        _description = mbtPtr->metaData().value("description");
        _version = mbtPtr->metaData().value("version");
        _attribution = mbtPtr->metaData().value("attribution");
        _maxzoom = mbtPtr->metaData().value("maxzoom").toInt();
        _minzoom = mbtPtr->metaData().value("minzoom").toInt();
    }

    _tiles = baseURL+"/{z}/{x}/{y}."+_format;

    // Safety check
    if (_minzoom > _maxzoom)
    {
        _maxzoom = -1;
        _minzoom = -1;
    }
}


GeoMaps::TileHandler::~TileHandler()
{
    qDeleteAll(m_mbtiles);
}


void GeoMaps::TileHandler::removeFile(const QString& localFileName)
{
    GeoMaps::MBTILES* mbtToRemove;
    foreach(auto mbtPtr, m_mbtiles)
    {
        if (mbtPtr->fileName() != localFileName)
        {
            continue;
        }
        mbtToRemove = mbtPtr;
        break;
    }

    m_mbtiles.remove(mbtToRemove);
    delete mbtToRemove;

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
        auto z = path.section('/', 1, 1).toInt();
        auto x = path.section('/', 2, 2).toInt();
        auto y = path.section('/', 3, 3).section('.', 0, 0).toInt();

        foreach(auto* mbtilesPtr, m_mbtiles)
        {
            // Get data
            QByteArray tileData = mbtilesPtr->tile(z,x,y);
            if (tileData.isEmpty())
            {
                continue;
            }

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
