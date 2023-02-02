/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

#include <QHttpServerRequest>
#include <QHttpServerResponder>

#include <QRegExp>
#include <QUrl>
#include <utility>

#include "TileHandler.h"
#include "TileServer.h"
#include "GlobalObject.h"
#include "geomaps/GeoMapProvider.h"


GeoMaps::TileServer::TileServer(QUrl baseUrl, QObject *parent)
    : QAbstractHttpServer(parent), _baseUrl(std::move(baseUrl))
{
    listen(QHostAddress("127.0.0.1"));

    setUpTileHandlers();
}


auto GeoMaps::TileServer::serverUrl() -> QString
{
    qDebug() << serverPorts();
    auto ports = serverPorts();
    if (ports.isEmpty())
    {
        return {};
    }

    return QStringLiteral("http://127.0.0.1:%1").arg(QString::number(ports[0]));
/*
    if (isListening())
    {
        return QStringLiteral("http://%1:%2").arg(serverAddress().toString(),QString::number(serverPort()));
    }
*/
    return {};
}


void GeoMaps::TileServer::addMbtilesFileSet(const QVector<QPointer<GeoMaps::MBTILES>>& baseMapsWithFiles, const QString& baseName)
{
    mbtileFileNameSets[baseName] = baseMapsWithFiles;
    setUpTileHandlers();
}


void GeoMaps::TileServer::removeMbtilesFileSet(const QString& path)
{
    mbtileFileNameSets.remove(path);
    setUpTileHandlers();
}


void GeoMaps::TileServer::setUpTileHandlers()
{

    // Now add subhandlers for each tile
    QMapIterator<QString, QVector<QPointer<GeoMaps::MBTILES>>> iterator(mbtileFileNameSets);
    while (iterator.hasNext())
    {
        iterator.next();

        // Find base name of the file
        QString URL;
        if (_baseUrl.isEmpty())
        {
            URL = serverUrl()+"/"+iterator.key();
        }
        else
        {
            URL = _baseUrl.toString()+"/"+iterator.key();
        }

        auto* handler = new TileHandler(iterator.value(), URL, this);
        tileHandlers[iterator.key()] = handler;
    }

}


bool GeoMaps::TileServer::handleRequest(const QHttpServerRequest& request, QTcpSocket* socket)
{
    auto path = request.url().path();

    // GeoJSON
    if (path.endsWith("aviationData.geojson"))
    {
        auto responder = makeResponder(request, socket);
        responder.write(GlobalObject::geoMapProvider()->geoJSON(), "application/json");
        return true;
    }

    // File in resource
    if (QFile::exists(":"+path))
    {
        auto* file = new QFile(":"+path);

        auto responder = makeResponder(request, socket);
        responder.write(file, "application/octet-stream");
        return true;
    }

    if (tileHandlers.contains(path))
    {
        auto tileHandler = tileHandlers[path];
        if (tileHandler.isNull())
        {
            return false;
        }
#warning
    }

    qDebug() << "handleRequest" << request;
    return false;
}


void GeoMaps::TileServer::missingHandler(const QHttpServerRequest& request, QTcpSocket* socket)
{
    auto responder = makeResponder(request, socket);
    responder.write(QHttpServerResponder::StatusCode::NotFound);
}
