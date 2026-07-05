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

#include <QBuffer>
#include <QGuiApplication>
#include <QHttpServerRequest>
#include <QHttpServerResponder>
#include <QImage>
#include <QTcpServer>

#include "TileServer.h"
#include "geomaps/GeoMapProvider.h"

using namespace Qt::Literals::StringLiterals;


// Night-mode version of a sprite sheet PNG from the resource system. Two-part
// transform, following the night-mode conventions established in FlightMap.qml
// and Global.qml: colors with little saturation (the white icon halos, dark
// glyphs) have their brightness flipped, like the label and halo colors on the
// moving map; saturated colors are muted, calibrated so that the icon hues
// #1000b0 and #ff0000 land near the night-mode airspace hues of Global.qml.
// The two regimes are blended by saturation, so that anti-aliasing pixels do
// not produce seams.
static QByteArray nightVersionOf(const QString& fileName)
{
    QImage image(fileName);
    image.convertTo(QImage::Format_ARGB32);

    for (int y = 0; y < image.height(); ++y)
    {
        auto* line = reinterpret_cast<QRgb*>(image.scanLine(y));
        for (int x = 0; x < image.width(); ++x)
        {
            const QColor color = QColor::fromRgb(line[x] | 0xFF000000U);

            const qreal vFlipped = 0.88 * (1.0 - color.valueF());
            const qreal vMuted = 0.45 + 0.4 * color.valueF();
            const qreal saturation = color.saturationF();
            const qreal value = (1.0 - saturation) * vFlipped + saturation * vMuted;

            const auto newColor = QColor::fromHsvF(qMax(color.hsvHueF(), 0.0), 0.55 * saturation, value);
            line[x] = (line[x] & 0xFF000000U) | (newColor.rgb() & 0x00FFFFFFU);
        }
    }

    QByteArray result;
    QBuffer buffer(&result);
    buffer.open(QIODevice::WriteOnly);
    image.save(&buffer, "PNG");
    return result;
}


GeoMaps::TileServer::TileServer(QObject* parent)
    : QAbstractHttpServer(parent)
{
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
    // Night-mode sprite sheet, recolored on the fly from the day-mode sprite
    // sheet in the resource system
    //
    if (path.startsWith(u"/flightMap/sprites-night/"_s))
    {
        QString dayPath = ":"+path;
        dayPath.replace(u"/sprites-night/"_s, u"/sprites/"_s);
        if (!QFile::exists(dayPath))
        {
            return false;
        }
        if (path.endsWith(u".json"_s))
        {
            // The sprite geometry does not change, only the PNGs do
            responder.write(new QFile(dayPath), "application/json");
            return true;
        }
        if (path.endsWith(u".png"_s))
        {
            auto& nightSprite = m_nightSprites[dayPath];
            if (nightSprite.isEmpty())
            {
                nightSprite = nightVersionOf(dayPath);
            }
            responder.write(nightSprite, "image/png");
            return true;
        }
        return false;
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
