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


#include "TileServer.h"
#include "geomaps/GeoMapProvider.h"


GeoMaps::TileServer::TileServer(QObject* parent)
    : QAbstractHttpServer(parent)
{
    listen(QHostAddress(QStringLiteral("127.0.0.1")));
}


void GeoMaps::TileServer::addMbtilesFileSet(const QString& baseName, const QVector<QPointer<GeoMaps::MBTILES>>& baseMapsWithFiles)
{
    QString URL = serverUrl()+"/"+baseName;
    auto* handler = new TileHandler(baseMapsWithFiles, URL, this);
    m_tileHandlers[baseName] = handler;
}


void GeoMaps::TileServer::removeMbtilesFileSet(const QString& baseName)
{
    delete m_tileHandlers.take(baseName);
}


auto GeoMaps::TileServer::serverUrl() -> QString
{
    auto ports = serverPorts();
    if (ports.isEmpty())
    {
        return {};
    }
    return QStringLiteral("http://127.0.0.1:%1").arg(QString::number(ports[0]));
}


// Private Methods

bool GeoMaps::TileServer::handleRequest(const QHttpServerRequest& request, QTcpSocket* socket)
{
    auto path = request.url().path();
    auto pathElements = path.split('/', Qt::SkipEmptyParts);
    auto responder = makeResponder(request, socket);

    //
    // Paranoid safety check
    //
    if (pathElements.isEmpty())
    {
        return false;
    }

    //
    // GeoJSON with aviation data
    //
    if (path.endsWith(QLatin1String("aviationData.geojson")))
    {
        responder.write(GlobalObject::geoMapProvider()->geoJSON(), "application/json");
        return true;
    }

    //
    // File from resource system
    //
    if (QFile::exists(":"+path))
    {
        auto* file = new QFile(":"+path);
        responder.write(file, "application/octet-stream");
        return true;
    }

    //
    // Tile data
    //
    if (m_tileHandlers.contains(pathElements[0]))
    {
        auto tileHandler = m_tileHandlers[pathElements[0]];
        if (tileHandler.isNull())
        {
            return false;
        }
        pathElements.remove(0);
        return tileHandler->process(&responder, pathElements);
    }

    //
    // Request not parsed
    //
    return false;
}


void GeoMaps::TileServer::missingHandler(const QHttpServerRequest& request, QTcpSocket* socket)
{
    auto responder = makeResponder(request, socket);
    responder.write(QHttpServerResponder::StatusCode::NotFound);
}
