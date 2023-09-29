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

#include <QImage>
#include <QMimeDatabase>
#include <QUrl>

#include "fileFormats/TripKit.h"
#include "geomaps/CUP.h"
#include "geomaps/GeoJSON.h"
#include "geomaps/MBTILES.h"
#include "geomaps/OpenAir.h"
#include "geomaps/VAC.h"
#include "platform/FileExchange_Abstract.h"
#include "traffic/TrafficDataProvider.h"
#include "traffic/TrafficDataSource_File.h"


Platform::FileExchange_Abstract::FileExchange_Abstract(QObject *parent)
    : GlobalObject(parent)
{
}



//
// Methods
//

void Platform::FileExchange_Abstract::processFileOpenRequest(const QByteArray& path)
{
    processFileOpenRequest(QString::fromUtf8(path).simplified());
}


void Platform::FileExchange_Abstract::processFileOpenRequest(const QString& path)
{
    QString myPath;
    if (path.startsWith(u"file:"))
    {
        QUrl const url(path.trimmed());
        myPath = url.toLocalFile();
    }
    else
    {
        myPath = path;
    }

    QMimeDatabase const dataBase;
    auto mimeType = dataBase.mimeTypeForFile(myPath);

    /*
     * Check for various possible file formats/contents
     */

    // Flight Route in GPX format
    if ((mimeType.inherits(QStringLiteral("application/xml"))) || (mimeType.name() == u"application/x-gpx+xml"))
    {
        emit openFileRequest(myPath, {}, FlightRouteOrWaypointLibrary);
        return;
    }

    // GeoJSON file
    auto fileContent = GeoMaps::GeoJSON::inspect(myPath);
    if (fileContent == GeoMaps::GeoJSON::flightRoute)
    {
        emit openFileRequest(myPath, {}, FlightRoute);
        return;
    }
    if (fileContent == GeoMaps::GeoJSON::waypointLibrary)
    {
        emit openFileRequest(myPath, {}, WaypointLibrary);
        return;
    }
    if (fileContent == GeoMaps::GeoJSON::valid)
    {
        emit openFileRequest(myPath, {}, FlightRouteOrWaypointLibrary);
        return;
    }

    // FLARM Simulator file
    if (Traffic::TrafficDataSource_File::containsFLARMSimulationData(myPath))
    {
        auto *source = new Traffic::TrafficDataSource_File(myPath);
        GlobalObject::trafficDataProvider()->addDataSource(source); // Will take ownership of source
        source->connectToTrafficReceiver();
        return;
    }

    // MBTiles containing a vector map
    GeoMaps::MBTILES mbtiles(myPath);
    if (mbtiles.format() == GeoMaps::MBTILES::Vector)
    {
        emit openFileRequest(myPath, {}, VectorMap);
        return;
    }

    // MBTiles containing a raster map
    if (mbtiles.format() == GeoMaps::MBTILES::Raster)
    {
        emit openFileRequest(myPath, {}, RasterMap);
        return;
    }

    // CUP file
    if (GeoMaps::CUP::isValid(myPath))
    {
        emit openFileRequest(myPath, {}, WaypointLibrary);
        return;
    }

    // VAC
    GeoMaps::VAC const vac(myPath);
    if (vac.isValid())
    {
        emit openFileRequest(myPath, vac.baseName(), VAC);
        return;
    }

    // Image
    QImage const img(myPath);
    if (!img.isNull())
    {
        emit openFileRequest(myPath, {}, Image);
        return;
    }

    // TripKits
#warning Check MimeType
    FileFormats::TripKit const tripKit(myPath);
    if (tripKit.isValid())
    {
        emit openFileRequest(myPath, tripKit.name(), TripKit);
        return;
    }

    // OpenAir
    QString info;
    if (GeoMaps::openAir::isValid(myPath, &info))
    {
        emit openFileRequest(myPath, info, OpenAir);
        return;
    }

    emit openFileRequest(myPath, {}, UnknownFunction);
}
