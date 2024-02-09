/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

#include <QHttpServerResponder>
#include <QJsonArray>
#include <QJsonObject>
#include <QPointer>

#include "TileHandler.h"


GeoMaps::TileHandler::TileHandler(const QVector<QSharedPointer<GeoMaps::MBTILES>>& mbtileFiles, const QString& baseURL) :
    m_mbtiles(mbtileFiles)
{
    QString _name;
    QString _encoding;
    QString _tiles;
    QString _description;

    QString _version;

    QString _attribution;

    int _maxzoom {-1};
    int _minzoom {-1};

    // Go through mbtile files and find real values
    _maxzoom = 10;
    _minzoom = 6;
    foreach (auto mbtPtr, mbtileFiles)
    {
        if (mbtPtr.isNull())
        {
            continue;
        }

        _name = mbtPtr->metaData().value(QStringLiteral("name"));
        _encoding = mbtPtr->metaData().value(QStringLiteral("encoding"));
        m_format = mbtPtr->metaData().value(QStringLiteral("format"));
        _description = mbtPtr->metaData().value(QStringLiteral("description"));
        _version = mbtPtr->metaData().value(QStringLiteral("version"));
        _attribution = mbtPtr->metaData().value(QStringLiteral("attribution"));
        bool ok = false;
        auto tmp_maxzoom = mbtPtr->metaData().value(QStringLiteral("maxzoom")).toInt(&ok);
        if (ok)
        {
            _maxzoom = qMax(_maxzoom, tmp_maxzoom);
        }
        auto tmp_minzoom = mbtPtr->metaData().value(QStringLiteral("minzoom")).toInt(&ok);
        if (ok)
        {
            _minzoom = qMin(_minzoom, tmp_minzoom);
        }
    }

    _tiles = baseURL+"/{z}/{x}/{y}."+m_format;

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
    if (!m_format.isEmpty())
    {
        result.insert(QStringLiteral("format"), m_format);
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

    m_tileJSON.setObject(result);
}


bool GeoMaps::TileHandler::process(QHttpServerResponder* responder, const QStringList &pathElements)
{
    // Serve tileJSON file, if requested
    if (pathElements.isEmpty() || pathElements[0].endsWith(u"json"_qs, Qt::CaseInsensitive))
    {
        responder->write(m_tileJSON);
        return true;
    }

    if (pathElements.size() != 3)
    {
        return false;
    }

    // Serve tile, if requested
    auto z = pathElements[0].toInt();
    auto x = pathElements[1].toInt();
    auto y = pathElements[2].section('.', 0, 0).toInt();

    // Retrieve tile data from the database
    foreach(auto mbtilesPtr, m_mbtiles)
    {
        if (mbtilesPtr.isNull())
        {
            continue;
        }

        // Get data
        QByteArray const tileData = mbtilesPtr->tile(z,x,y);
        if (tileData.isEmpty())
        {
            continue;
        }

        if (m_format == u"pbf"_qs)
        {
            responder->write(tileData, {{"Content-Type", "application/octet-stream"}, {"Content-Encoding", "gzip"}});
        }
        else
        {
            responder->write(tileData, "application/octet-stream");
        }
        return true;
    }

    return false;
}
