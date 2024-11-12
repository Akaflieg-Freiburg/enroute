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

#include <QGuiApplication>
#include <QHttpServerRequest>
#include <QHttpServerResponder>
#include <QTcpServer>

#include "TileServer.h"
#include "geomaps/GeoMapProvider.h"

using namespace Qt::Literals::StringLiterals;


GeoMaps::TileServer::TileServer(QObject* parent)
    : QAbstractHttpServer(parent)
{
#warning
    auto* localServer = new QTcpServer();
    localServer->listen();
    bind(localServer);
//    listen(QHostAddress(QStringLiteral("127.0.0.1")));

#if defined(Q_OS_IOS)
    connect(qGuiApp,
            &QGuiApplication::applicationStateChanged,
            this,
            [this](Qt::ApplicationState state)
            {
                if (state == Qt::ApplicationSuspended)
                {
                    suspended = true;
                    return;
                }
                if (suspended && (state == Qt::ApplicationActive))
                {
                    suspended = false;
                    restart();
                }
            });
#endif
}


void GeoMaps::TileServer::addMbtilesFileSet(const QString& baseName, const QVector<QSharedPointer<FileFormats::MBTILES>>& MBTilesFiles)
{
    QString const URL = serverUrl()+"/"+baseName;
    auto* handler = new TileHandler(MBTilesFiles, URL);
    m_tileHandlers[baseName] = QSharedPointer<GeoMaps::TileHandler>(handler);
}


QString GeoMaps::TileServer::serverUrl()
{
    auto ports = serverPorts();
    if (ports.isEmpty())
    {
        return {};
    }
    return QStringLiteral("http://127.0.0.1:%1").arg(QString::number(ports[0]));
}


// Private Methods

bool GeoMaps::TileServer::handleRequest(const QHttpServerRequest& request, QHttpServerResponder& responder)
{
    auto path = request.url().path();
    auto pathElements = path.split('/', Qt::SkipEmptyParts);

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
    if (path.endsWith(u"aviationData.geojson"_s))
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
        auto tileHandler = m_tileHandlers.value(pathElements[0]);
        if (tileHandler == nullptr)
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


void GeoMaps::TileServer::missingHandler(const QHttpServerRequest& request, QHttpServerResponder& responder)
{
    Q_UNUSED(request)
    responder.write(QHttpServerResponder::StatusCode::NotFound);
}


void GeoMaps::TileServer::restart()
{
    bool serverPortChanged = false;
    foreach (auto _server, servers())
    {
        if (_server == nullptr)
        {
            continue;
        }
        auto port = _server->serverPort();
        _server->close();
        auto success = _server->listen(QHostAddress(QStringLiteral("127.0.0.1")), port);
        if (!success)
        {
            _server->listen(QHostAddress(QStringLiteral("127.0.0.1")));
            serverPortChanged = true;
        }
    }

    if (serverPortChanged)
    {
        emit serverUrlChanged();
    }
}
