/***************************************************************************
 *   Copyright (C) 2019, 2021 by Stefan Kebekus                            *
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

#include <QUrl>
#include <utility>

#include "TileHandler.h"
#include "TileServer.h"
#include "geomaps/Downloadable.h"


GeoMaps::TileServer::TileServer(QUrl baseUrl, QObject *parent)
    : QHttpEngine::Server(parent), _baseUrl(std::move(baseUrl))
{
    setUpTileHandlers();
}


auto GeoMaps::TileServer::serverUrl() const -> QString
{
    if (isListening()) {
        return QString("http://%1:%2").arg(serverAddress().toString(),QString::number(serverPort()));
    }
    return QString();
}


void GeoMaps::TileServer::addMbtilesFileSet(const QVector<QPointer<Downloadable>>& baseMapsWithFiles, const QString& baseName)
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
    // Create new file system handler and delete old one
    auto *newFileSystemHandler = new QHttpEngine::FilesystemHandler(":", this);
    newFileSystemHandler->addRedirect(QRegExp("^$"), "/index.html");
    setHandler(newFileSystemHandler);
    delete currentFileSystemHandler;
    currentFileSystemHandler = newFileSystemHandler;

    // Now add subhandlers for each tile
    QMapIterator<QString, QVector<QPointer<Downloadable>>> iterator(mbtileFileNameSets);
    while (iterator.hasNext()) {
        iterator.next();

        // Find base name of the file
        QString URL;
        if (_baseUrl.isEmpty()) {
            URL = serverUrl()+"/"+iterator.key();
        } else {
            URL = _baseUrl.toString()+"/"+iterator.key();
        }

        auto *handler = new TileHandler(iterator.value(), URL, newFileSystemHandler);
        newFileSystemHandler->addSubHandler(QRegExp("^"+iterator.key()), handler);
    }

}
