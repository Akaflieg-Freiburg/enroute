/***************************************************************************
 *   Copyright (C) 2019-2025 by Stefan Kebekus                             *
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

#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QQmlEngine>
#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrentRun>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include <tuple>

#include "GlobalSettings.h"
#include "Librarian.h"
#include "dataManagement/DataManager.h"
#include "fileFormats/DataFileAbstract.h"
#include "fileFormats/MBTILES.h"
#include "fileFormats/VACCollection.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/WaypointLibrary.h"
#include "navigation/Navigator.h"

using namespace Qt::Literals::StringLiterals;


GeoMaps::GeoMapProvider::~GeoMapProvider()
{
    // Break all bindings before destruction proceeds
    m_availableRasterMaps.takeBinding();
}


GeoMaps::GeoMapProvider::GeoMapProvider(QObject *parent)
    : GlobalObject(parent)
{
    m_combinedGeoJSON = emptyGeoJSON();

    // An oversized cache file cannot be held in memory on mobile devices.
    // Delete it; it will be regenerated (respecting maxAviationMapFileSize)
    // by onAviationMapsChanged().
    if (QFileInfo(geoJSONCache).size() > maxGeoJSONCacheSize)
    {
        QFile::remove(geoJSONCache);
    }

    QFile geoJSONCacheFile(geoJSONCache);
    if (geoJSONCacheFile.open(QFile::ReadOnly))
    {
        m_combinedGeoJSON = geoJSONCacheFile.readAll();
    }
    geoJSONCacheFile.close();

    // Pass signal through when the tile server changes its URL
    connect(&m_tileServer, &GeoMaps::TileServer::serverUrlChanged, this, [this]() {emit styleFileURLChanged();});
}

void GeoMaps::GeoMapProvider::deferredInitialization()
{
    connect(GlobalObject::dataManager()->aviationMaps(), &DataManagement::Downloadable_Abstract::fileContentChanged_delayed, this, &GeoMaps::GeoMapProvider::onAviationMapsChanged);
    connect(GlobalObject::dataManager()->baseMaps(), &DataManagement::Downloadable_Abstract::fileContentChanged_delayed, this, &GeoMaps::GeoMapProvider::onMBTILESChanged);
    connect(GlobalObject::dataManager()->baseMaps(), &DataManagement::Downloadable_Abstract::filesChanged, this, &GeoMaps::GeoMapProvider::onMBTILESChanged);
    connect(GlobalObject::dataManager()->terrainMaps(), &DataManagement::Downloadable_Abstract::fileContentChanged_delayed, this, &GeoMaps::GeoMapProvider::onMBTILESChanged);
    connect(GlobalObject::dataManager()->baseMaps(), &DataManagement::Downloadable_Abstract::aboutToChangeFile, this, &GeoMaps::GeoMapProvider::onMBTILESAboutToChange);
    connect(GlobalObject::dataManager()->terrainMaps(), &DataManagement::Downloadable_Abstract::aboutToChangeFile, this, &GeoMaps::GeoMapProvider::onMBTILESAboutToChange);
    connect(GlobalObject::globalSettings(), &GlobalSettings::hideGlidingSectorsChanged, this, &GeoMaps::GeoMapProvider::onAviationMapsChanged);

    connect(GlobalObject::globalSettings(), &GlobalSettings::nightModeChanged, this, [this]() {delete m_styleFile; emit styleFileURLChanged();});

    connect(&m_tileServer, &GeoMaps::TileServer::serverUrlChanged, this, &GeoMaps::GeoMapProvider::serverUrlChanged);

    _aviationDataCacheTimer.setSingleShot(true);
    _aviationDataCacheTimer.setInterval(3s);
    connect(&_aviationDataCacheTimer, &QTimer::timeout, this, &GeoMaps::GeoMapProvider::onAviationMapsChanged);

    // Set Binding
    m_availableRasterMaps.setBinding([this]() {return computeAvailableRasterMaps();});

    onAviationMapsChanged();
    onMBTILESChanged();

    setCurrentRasterMap(QSettings().value("currentRasterMap", QString()).toString());
    m_currentRasterMapNotifier = m_currentRasterMap.addNotifier([this]() {
        QSettings().setValue("currentRasterMap", m_currentRasterMap.value());
    });

    GlobalObject::dataManager()->aviationMaps()->killFileContentChanged_delayed();
    GlobalObject::dataManager()->baseMaps()->killFileContentChanged_delayed();
    GlobalObject::dataManager()->terrainMaps()->killFileContentChanged_delayed();
}


//
// Getter Methods
//

QString GeoMaps::GeoMapProvider::copyrightNotice()
{
    QString result;
    if (GlobalObject::dataManager()->aviationMaps()->hasFile())
    {
        result += u"<h4>"_s + tr("Aviation Maps") + u"</h4>\n"_s;
        result += u"<p>"_s + tr("The aeronautical maps are compiled from databases provided by the "
                                "<a href='http://openaip.net'>openAIP</a> and "
                                "<a href='https://www.openflightmaps.org/'>open flightmaps</a> "
                                "projects.") + u"</p>\n"_s;
        result += QStringLiteral("<a href='https://openAIP.net'>© openAIP</a><br>"
                                 "<a href='https://openflightmaps.org'>© open flightmaps</a>");
    }

    if (GlobalObject::dataManager()->baseMaps()->hasFile())
    {
        result += u"<h4>"_s + tr("Base Maps") + u"</h4>\n"_s;
        result += u"<p>"_s + tr("The base maps are generated from "
                                "<a href='https://www.openstreetmap.org'>OpenStreetMap</a> data.") + u"</p>\n"_s;
        result += QStringLiteral("<a href='https://www.openstreetmap.org/about'>© OpenStreetMap contributors</a>");
    }

    if (GlobalObject::dataManager()->terrainMaps()->hasFile())
    {
        result += u"<h4>"_s + tr("Terrain Maps") + u"</h4>\n"_s;
        result += u"<p>"_s + tr("The terrain maps are derived from the "
                                "<a href='https://registry.opendata.aws/terrain-tiles/'>Terrain "
                                "Tiles Open Dataset on Amazon AWS</a>.") + u"</p>"_s +
                  u"<ul style='margin-left:-25px;'>"
                                "<li><p>ArcticDEM terrain data DEM(s) were created from DigitalGlobe, Inc., imagery and funded under National Science Foundation awards 1043681, 1559691, and 1542736</p>"
                                "<li><p>Australia terrain data © Commonwealth of Australia (Geoscience Australia) 2017</p>"
                                "<li><p>Austria terrain data © offene Daten Österreichs – Digitales Geländemodell (DGM) Österreich</p>"
                                "<li><p>Canada terrain data contains information licensed under the Open Government Licence – Canada</p>"
                                "<li><p>Europe terrain data produced using Copernicus data and information funded by the European Union - EU-DEM layers</p>"
                                "<li><p>Global ETOPO1 terrain data U.S. National Oceanic and Atmospheric Administration</p>"
                                "<li><p>Mexico terrain data source: INEGI, Continental relief, 2016</p>"
                                "<li><p>New Zealand terrain data Copyright 2011 Crown copyright (c) Land Information New Zealand and the New Zealand Government (All rights reserved)</p>"
                                "<li><p>Norway terrain data © Kartverket</p>"
                                "<li><p>United Kingdom terrain data © Environment Agency copyright and/or database right 2015. All rights reserved</p>"
                                "<li><p>United States 3DEP (formerly NED) and global GMTED2010 and SRTM terrain data courtesy of the U.S. Geological Survey.</p>"
                                "</ul>"_s;
    }

    if (GlobalObject::dataManager()->vacCollections()->hasFile())
    {
        // The attributions are set by the data server and are therefore not
        // translated here. Collections without attribution are silently
        // ignored.
        QStringList attributions;
        const auto files = GlobalObject::dataManager()->vacCollections()->files();
        for (const auto& file : files)
        {
            FileFormats::VACCollection const collection(file);
            if (!collection.isValid())
            {
                continue;
            }
            auto const attribution = collection.attribution();
            if (!attribution.isEmpty() && !attributions.contains(attribution))
            {
                attributions.append(attribution);
            }
        }
        if (!attributions.isEmpty())
        {
            attributions.sort();
            result += u"<h4>"_s + tr("Approach Charts") + u"</h4>\n"_s;
            result += u"<p>"_s + tr("The approach charts are provided by the following agencies.") + u"</p>"_s;
            result += u"<ul style='margin-left:-25px;'>"_s;
            for (const auto& attribution : std::as_const(attributions))
            {
                result += u"<li><p>"_s + attribution + u"</p>"_s;
            }
            result += u"</ul>"_s;
        }
    }

    return result;
}

QString GeoMaps::GeoMapProvider::styleFileURL()
{
    if (m_styleFile.isNull())
    {
        QFile file;
        if (GlobalObject::dataManager()->baseMaps()->hasFile())
        {
            if (GlobalObject::globalSettings()->nightMode())
            {
                file.setFileName(QStringLiteral(":/flightMap/osm-liberty-dark.json"));
            }
            else
            {
                file.setFileName(QStringLiteral(":/flightMap/osm-liberty.json"));
            }
        }
        else
        {
            file.setFileName(QStringLiteral(":/flightMap/empty.json"));
        }

        QByteArray data;
        if (file.open(QIODevice::ReadOnly))
        {
            data = file.readAll();
            data.replace("%URL%", (m_tileServer.serverUrl() + u"/"_s + _currentBaseMapPath).toLatin1());
            data.replace("%URLT%", (m_tileServer.serverUrl() + u"/"_s + _currentTerrainMapPath).toLatin1());
            data.replace("%URL2%", m_tileServer.serverUrl().toLatin1());
            file.close();
        }

        m_styleFile = new QTemporaryFile(this);
        if (m_styleFile->open())
        {
            m_styleFile->write(data);
            m_styleFile->close();
        }
    }
    return u"file://"_s + m_styleFile->fileName();
}


//
// Methods
//

QVariantList GeoMaps::GeoMapProvider::airspacesAtPosition(const QGeoCoordinate& position)
{
    QVector<Airspace> result;
    result.reserve(10);
    foreach(auto airspace, m_airspaces.value()) {
        if (airspace.polygon().contains(position)) {
            result.append(airspace);
        }
    }

    // Sort airspaces according to lower boundary
    std::sort(result.begin(), result.end(), [](const Airspace& first, const Airspace& second) {
        auto firstLB = first.estimatedLowerBoundMSL(Units::Distance::fromFT(0), Units::Pressure::fromHPa(1013.2), Units::Distance::fromFT(0), Units::Distance::fromFT(0));
        auto secondLB = second.estimatedLowerBoundMSL(Units::Distance::fromFT(0), Units::Pressure::fromHPa(1013.2), Units::Distance::fromFT(0), Units::Distance::fromFT(0));
        if (firstLB != secondLB)
        {
            return (firstLB > secondLB);
        }
        return (first.estimatedUpperBoundMSL(Units::Distance::fromFT(0), Units::Pressure::fromHPa(1013.2), Units::Distance::fromFT(0), Units::Distance::fromFT(0)) > second.estimatedUpperBoundMSL(Units::Distance::fromFT(0), Units::Pressure::fromHPa(1013.2), Units::Distance::fromFT(0), Units::Distance::fromFT(0)));
    });

    QVariantList final;
    foreach(auto airspace, result)
        final.append( QVariant::fromValue(airspace) );

    return final;
}

GeoMaps::Waypoint GeoMaps::GeoMapProvider::closestWaypoint(QGeoCoordinate position, const QGeoCoordinate& distPosition)
{
    position.setAltitude(qQNaN());
    if (!position.isValid())
    {
        return {position};
    }
    const double maxDistance = position.distanceTo(distPosition);

    Waypoint result;
    bool haveResult = false;
    double resultDistance = 0.0;
    auto consider = [&](const Waypoint& candidate) {
        const double distance = position.distanceTo(candidate.coordinate());
        if (!haveResult || (distance < resultDistance))
        {
            result = candidate;
            resultDistance = distance;
            haveResult = true;
        }
    };

    // Map waypoints are valid by construction, so no isValid() check here.
    // Only the grid cells around the position need to be visited: any
    // waypoint farther away than maxDistance leads to the fallback below
    // anyway. Rows are visited in ascending order so that ties resolve to
    // name order, as they did with a full scan.
    const auto rows = rowsNear(position, maxDistance);
    if (rows.has_value())
    {
        auto sortedRows = *rows;
        std::sort(sortedRows.begin(), sortedRows.end());
        for (const int row : std::as_const(sortedRows))
        {
            consider(_waypoints_.at(row));
        }
    }
    else
    {
        for (const auto& waypoint : std::as_const(_waypoints_))
        {
            consider(waypoint);
        }
    }

    const auto wpLibrary = GlobalObject::waypointLibrary()->waypoints();
    for (const auto& waypoint : wpLibrary)
    {
        if (waypoint.isValid())
        {
            consider(waypoint);
        }
    }

    for (const auto& waypoint : GlobalObject::navigator()->flightRoute()->midFieldWaypoints())
    {
        if (waypoint.isValid())
        {
            consider(waypoint);
        }
    }

    if (!haveResult || (resultDistance > maxDistance))
    {
        position.setAltitude( terrainElevationAMSL(position).toM() );
        return {position};
    }
    return result;
}


qint32 GeoMaps::GeoMapProvider::cellOf(const QGeoCoordinate& coordinate)
{
    auto latCell = static_cast<qint32>(std::floor((coordinate.latitude() + 90.0) * 10.0));
    latCell = std::clamp(latCell, 0, 1799);
    auto lonCell = static_cast<qint32>(std::floor((coordinate.longitude() + 180.0) * 10.0));
    lonCell = ((lonCell % 3600) + 3600) % 3600;
    return latCell * 3600 + lonCell;
}


std::optional<QList<int>> GeoMaps::GeoMapProvider::rowsNear(const QGeoCoordinate& position, double radiusM) const
{
    if (!position.isValid() || !qIsFinite(radiusM) || (radiusM < 0.0))
    {
        return std::nullopt;
    }

    // Conservative bounding box: 110 km per degree is slightly less than the
    // true value, and the longitude extent uses the latitude circle at the
    // pole-ward edge of the box, which is the smallest one in the box.
    constexpr double metersPerDegree = 110000.0;
    const double dLat = radiusM / metersPerDegree;
    const double maxAbsLat = std::min(std::abs(position.latitude()) + dLat, 89.9);
    const double dLon = radiusM / (metersPerDegree * std::cos(qDegreesToRadians(maxAbsLat)));
    if (dLon >= 180.0)
    {
        return std::nullopt;
    }

    const auto latLo = std::clamp(static_cast<qint32>(std::floor((position.latitude() - dLat + 90.0) * 10.0)), 0, 1799);
    const auto latHi = std::clamp(static_cast<qint32>(std::floor((position.latitude() + dLat + 90.0) * 10.0)), 0, 1799);
    const auto lonLo = static_cast<qint32>(std::floor((position.longitude() - dLon + 180.0) * 10.0));
    const auto lonHi = static_cast<qint32>(std::floor((position.longitude() + dLon + 180.0) * 10.0));
    const qint64 numCells = static_cast<qint64>(latHi - latLo + 1) * static_cast<qint64>(lonHi - lonLo + 1);
    if (numCells > 2500)
    {
        return std::nullopt;
    }

    const auto& cells = m_waypointIndex.cells;
    auto const byCell = [](const std::pair<qint32, int>& a, const std::pair<qint32, int>& b) { return a.first < b.first; };
    QList<int> rows;
    for (qint32 latCell = latLo; latCell <= latHi; ++latCell)
    {
        for (qint32 lon = lonLo; lon <= lonHi; ++lon)
        {
            const qint32 lonCell = ((lon % 3600) + 3600) % 3600;
            const std::pair<qint32, int> key {latCell * 3600 + lonCell, 0};
            auto const range = std::equal_range(cells.cbegin(), cells.cend(), key, byCell);
            for (auto it = range.first; it != range.second; ++it)
            {
                rows.append(it->second);
            }
        }
    }
    return rows;
}


GeoMaps::GeoMapProvider::WaypointIndex GeoMaps::GeoMapProvider::buildWaypointIndex(const QList<Waypoint>& waypoints)
{
    WaypointIndex index;
    index.searchKeys.reserve(waypoints.size());
    index.cells.reserve(waypoints.size());
    for (qsizetype i = 0; i < waypoints.size(); ++i)
    {
        const auto row = static_cast<int>(i);
        const auto& waypoint = waypoints.at(i);
        const auto code = waypoint.ICAOCode();
        if (!code.isEmpty() && !index.rowByICAOCode.contains(code))
        {
            index.rowByICAOCode.insert(code, row);
        }
        index.searchKeys.append(Librarian::simplify(waypoint.name()) + u' ' + Librarian::simplify(code));
        index.cells.append({cellOf(waypoint.coordinate()), row});
    }
    std::sort(index.cells.begin(), index.cells.end());
    return index;
}


Units::Distance decodeImageData(const QImage* image, double intraTileX, double intraTileY)
{
    if ((image == nullptr) || !qIsFinite(intraTileX) || !qIsFinite(intraTileY) ||
        (intraTileX < 0.0) || (intraTileX > 1.0) || (intraTileY < 0.0) || (intraTileY > 1.0) ||
        image->isNull())
    {
        return {};
    }
    intraTileX *= image->width()-1;
    intraTileY *= image->height()-1;
    auto t = intraTileX - qFloor(intraTileX);
    auto u = intraTileY - qFloor(intraTileY);

    auto pix = image->pixel(qFloor(intraTileX), qFloor(intraTileY));
    double const y1 = (qRed(pix) * 256.0 + qGreen(pix) + qBlue(pix) / 256.0) - 32768.0;

    pix = image->pixel(qCeil(intraTileX), qFloor(intraTileY));
    double const y2 = (qRed(pix) * 256.0 + qGreen(pix) + qBlue(pix) / 256.0) - 32768.0;

    pix = image->pixel(qCeil(intraTileX), qCeil(intraTileY));
    double const y3 = (qRed(pix) * 256.0 + qGreen(pix) + qBlue(pix) / 256.0) - 32768.0;

    pix = image->pixel(qFloor(intraTileX), qCeil(intraTileY));
    double const y4 = (qRed(pix) * 256.0 + qGreen(pix) + qBlue(pix) / 256.0) - 32768.0;

    return Units::Distance::fromM((1-t)*(1-u)*y1 + t*(1-u)*y2+t*u*y3+(1-t)*u*y4);
}

Units::Distance GeoMaps::GeoMapProvider::terrainElevationAMSL(const QGeoCoordinate& coordinate)
{
    if (!coordinate.isValid())
    {
        return {};
    }

    const int zoomMin = 6;
    const int zoomMax = 10;

    for(int zoom = zoomMax; zoom >= zoomMin; zoom--)
    {
        auto tilex = (coordinate.longitude()+180.0)/360.0 * (1<<zoom);
        auto tiley = (1.0 - asinh(tan(qDegreesToRadians(coordinate.latitude())))/M_PI)/2.0 * (1<<zoom);
        auto intraTileX = tilex-floor(tilex);
        auto intraTileY = tiley-floor(tiley);

        const qint64 keyA = qFloor(tilex) & 0xFFFF;
        const qint64 keyB = qFloor(tiley) & 0xFFFF;
        const qint64 key = (keyA << 32) + (keyB << 16) + zoom;

        if (terrainTileCache.contains(key))
        {
            auto* tileImg = terrainTileCache.object(key);
            if (!tileImg->isNull())
            {
                return decodeImageData(tileImg, intraTileX, intraTileY);
            }
        }

        foreach(auto mbtPtr, m_terrainMapTiles)
        {
            if (mbtPtr.isNull())
            {
                continue;
            }

            auto tileData = mbtPtr->tile(zoom, qFloor(tilex), qFloor(tiley));
            if (tileData.isEmpty())
            {
                continue;
            }

            auto* tileImg = new QImage();
            tileImg->loadFromData(tileData);
            if (tileImg->isNull())
            {
                delete tileImg;
                continue;
            }

            terrainTileCache.insert(key, tileImg);
            return decodeImageData(tileImg, intraTileX, intraTileY);
        }
    }

    return {};
}

QByteArray GeoMaps::GeoMapProvider::emptyGeoJSON()
{
    QJsonObject resultObject;
    resultObject.insert(QStringLiteral("type"), "FeatureCollection");
    resultObject.insert(QStringLiteral("features"), QJsonArray());
    QJsonDocument const geoDoc(resultObject);
    return geoDoc.toJson(QJsonDocument::JsonFormat::Compact);
}

GeoMaps::Waypoint GeoMaps::GeoMapProvider::findByID(const QString& icaoID)
{
    const auto row = m_waypointIndex.rowByICAOCode.value(icaoID, -1);
    if ((row < 0) || (row >= _waypoints_.size()))
    {
        return {};
    }
    return _waypoints_.at(row);
}

QList<GeoMaps::Waypoint> GeoMaps::GeoMapProvider::nearbyWaypoints(const QGeoCoordinate& position, const QString& type)
{
    static constexpr qsizetype maxResults = 20;
    // Distance, tie-break order, waypoint
    using Candidate = std::tuple<double, qsizetype, Waypoint>;

    QList<Candidate> libraryCandidates;
    {
        const auto wpsLib = GlobalObject::waypointLibrary()->waypoints();
        for (qsizetype i = 0; i < wpsLib.size(); ++i)
        {
            const auto& waypoint = wpsLib.at(i);
            if (!waypoint.isValid() || (waypoint.type() != type))
            {
                continue;
            }
            libraryCandidates.append({position.distanceTo(waypoint.coordinate()), _waypoints_.size() + i, waypoint});
        }
    }

    auto const closest = [](QList<Candidate>& candidates) {
        const auto n = std::min(maxResults, candidates.size());
        std::partial_sort(candidates.begin(), candidates.begin() + n, candidates.end(), [](const Candidate& a, const Candidate& b) {
            return std::tie(std::get<0>(a), std::get<1>(a)) < std::tie(std::get<0>(b), std::get<1>(b));
        });
        QList<Waypoint> result;
        result.reserve(n);
        for (qsizetype i = 0; i < n; ++i)
        {
            result.append(std::get<2>(candidates.at(i)));
        }
        return result;
    };

    // Grow the search radius until at least maxResults candidates lie within
    // it. rowsNear() visits every waypoint within the radius, so the closest
    // maxResults candidates are then the closest overall.
    if (position.isValid())
    {
        for (const double radius : {20.0e3, 80.0e3, 320.0e3, 1280.0e3})
        {
            const auto rows = rowsNear(position, radius);
            if (!rows.has_value())
            {
                break;
            }
            QList<Candidate> candidates = libraryCandidates;
            for (const int row : *rows)
            {
                const auto& waypoint = _waypoints_.at(row);
                if (waypoint.type() != type)
                {
                    continue;
                }
                candidates.append({position.distanceTo(waypoint.coordinate()), row, waypoint});
            }
            const auto within = std::count_if(candidates.cbegin(), candidates.cend(), [radius](const Candidate& c) { return std::get<0>(c) <= radius; });
            if (within >= maxResults)
            {
                return closest(candidates);
            }
        }
    }

    // Sparse region or invalid position: consider all waypoints of the type
    QList<Candidate> candidates = libraryCandidates;
    for (qsizetype row = 0; row < _waypoints_.size(); ++row)
    {
        const auto& waypoint = _waypoints_.at(row);
        if (waypoint.type() != type)
        {
            continue;
        }
        candidates.append({position.distanceTo(waypoint.coordinate()), row, waypoint});
    }
    return closest(candidates);
}

QVector<GeoMaps::Waypoint> GeoMaps::GeoMapProvider::waypoints()
{
    return _waypoints_;
}

void GeoMaps::GeoMapProvider::setCurrentRasterMap(const QString& mapName)
{
    if (mapName == m_currentRasterMap)
    {
        return;
    }

    serveRasterMap(mapName);
    emit styleFileURLChanged();
}

void GeoMaps::GeoMapProvider::serveRasterMap(const QString& mapName)
{
    auto newRasterMap = QSharedPointer<FileFormats::MBTILES>(new FileFormats::MBTILES());

    QString newRasterMapName;
    if (!mapName.isEmpty())
    {
        for(const auto& mbt : m_baseMapRasterTiles.value())
        {
            if (QFileInfo(mbt->fileName()).baseName() == mapName)
            {
                newRasterMap = mbt;
                newRasterMapName = mapName;
                break;
            }
        }
    }

    const QScopedPropertyUpdateGroup guard;
    m_tileServer.removeMbtilesFileSet(u"rasterMap"_s);
    const QVector<QSharedPointer<FileFormats::MBTILES>> single {newRasterMap};
    m_tileServer.addMbtilesFileSet(u"rasterMap"_s, single);
    m_currentRasterMap = newRasterMapName;
    m_currentRasterMapTileSize = newRasterMap->tileSize();
}



//
// Private Methods and Slots
//

QStringList GeoMaps::GeoMapProvider::computeAvailableRasterMaps()
{
    QStringList result;
    result.reserve(m_baseMapRasterTiles.value().size());
    for(const auto& rasterMap : m_baseMapRasterTiles.value())
    {
        const QFileInfo info(rasterMap->fileName());
        if (info.exists())
        {
            result << info.baseName();
        }
    }
    return result;
}

void GeoMaps::GeoMapProvider::onAviationMapsChanged()
{
    // Paranoid safety checks
    if (_aviationDataCacheFuture.isRunning())
    {
        _aviationDataCacheTimer.start();
        return;
    }

    //
    // Generate new GeoJSON array and new list of waypoints
    //
    QStringList JSONFileNames;
    QStringList oversizedMapNames;
    for(auto* geoMapPtrX : GlobalObject::dataManager()->aviationMaps()->downloadables())
    {
        auto *geoMapPtr = qobject_cast<DataManagement::Downloadable_SingleFile*>(geoMapPtrX);
        if (geoMapPtr == nullptr)
        {
            continue;
        }
        // Ignore everything but geojson files
        if (!geoMapPtr->fileName().endsWith(u".geojson", Qt::CaseInsensitive))
        {
            continue;
        }
        if (!geoMapPtr->hasFile())
        {
            continue;
        }
        // Ignore oversized files, whose processing would exhaust the memory
        // available on mobile devices. The files remain on disk, so a map
        // update can replace them with reasonably-sized versions.
        if (QFileInfo(geoMapPtr->fileName()).size() > maxAviationMapFileSize)
        {
            oversizedMapNames += geoMapPtr->objectName();
            continue;
        }
        JSONFileNames += geoMapPtr->fileName();
    }
    m_oversizedMaps = oversizedMapNames;

    _aviationDataCacheFuture = QtConcurrent::run(&GeoMaps::GeoMapProvider::fillAviationDataCache, this, JSONFileNames, GlobalObject::globalSettings()->hideGlidingSectors());
    _aviationDataCacheFuture.then(this, [this](GeoMaps::GeoMapProvider::aviationDataCacheResult result) {
        m_airspaces = result.airspaces;
        if (_waypoints_ != result.waypoints)
        {
            _waypoints_ = result.waypoints;
            m_waypointIndex = result.index;
            emit waypointsChanged();
        }
        m_combinedGeoJSON = result.combinedGeoJSON;
        (void)FileFormats::DataFileAbstract::saveFileAtomically(geoJSONCache, m_combinedGeoJSON);
    });
}

void GeoMaps::GeoMapProvider::onMBTILESChanged()
{
    terrainTileCache.clear();

    QList<QSharedPointer<FileFormats::MBTILES>> newBaseMapRasterTiles;
    for (auto* downloadableX : GlobalObject::dataManager()->baseMapsRaster()->downloadables())
    {
        auto* downloadable = qobject_cast<DataManagement::Downloadable_SingleFile*>(downloadableX);
        if (downloadable == nullptr)
        {
            continue;
        }
        if (!downloadable->hasFile())
        {
            continue;
        }

        newBaseMapRasterTiles.append(QSharedPointer<FileFormats::MBTILES>(new FileFormats::MBTILES(downloadable->fileName())));
    }
    m_baseMapRasterTiles = newBaseMapRasterTiles;

    m_baseMapVectorTiles.clear();

    for (auto* downloadableX : GlobalObject::dataManager()->baseMapsVector()->downloadables())
    {
        auto* downloadable = qobject_cast<DataManagement::Downloadable_SingleFile*>(downloadableX);
        if (downloadable == nullptr)
        {
            continue;
        }
        if (!downloadable->hasFile())
        {
            continue;
        }

        m_baseMapVectorTiles.append(QSharedPointer<FileFormats::MBTILES>(new FileFormats::MBTILES(downloadable->fileName())));
    }

    m_terrainMapTiles.clear();
    for (auto* downloadableX : GlobalObject::dataManager()->terrainMaps()->downloadables())
    {
        auto* downloadable = qobject_cast<DataManagement::Downloadable_SingleFile*>(downloadableX);
        if (downloadable == nullptr)
        {
            continue;
        }
        if (!downloadable->hasFile())
        {
            continue;
        }

        m_terrainMapTiles.append(QSharedPointer<FileFormats::MBTILES>(new FileFormats::MBTILES(downloadable->fileName())));
    }
    emit terrainMapTilesChanged();

    // Stop serving tiles
    m_tileServer.removeMbtilesFileSet(_currentBaseMapPath);
    _currentBaseMapPath = QString::number(QRandomGenerator::global()->bounded(static_cast<quint32>(1000000000)));
    m_tileServer.removeMbtilesFileSet(_currentTerrainMapPath);
    _currentTerrainMapPath = QString::number(QRandomGenerator::global()->bounded(static_cast<quint32>(1000000000)));

    // Start serving tiles again
    m_tileServer.addMbtilesFileSet(_currentBaseMapPath, m_baseMapVectorTiles);
    m_tileServer.addMbtilesFileSet(_currentTerrainMapPath, m_terrainMapTiles);

    // Serve the current raster map from the freshly opened files, not from a
    // handle to a previous version of the file. If the map no longer exists,
    // this clears the property currentRasterMap.
    serveRasterMap(m_currentRasterMap.value());

    // Update style file
    delete m_styleFile;
    emit styleFileURLChanged();
}

void GeoMaps::GeoMapProvider::onMBTILESAboutToChange(const QString& fileName)
{
    auto refersToFile = [&fileName](const QSharedPointer<FileFormats::MBTILES>& mbtiles)
    {
        return mbtiles.isNull() || (mbtiles->fileName() == fileName);
    };

    auto rasterTiles = m_baseMapRasterTiles.value();
    if (rasterTiles.removeIf(refersToFile) > 0)
    {
        m_baseMapRasterTiles = rasterTiles;
    }
    if (m_baseMapVectorTiles.removeIf(refersToFile) > 0)
    {
        m_tileServer.removeMbtilesFileSet(_currentBaseMapPath);
        m_tileServer.addMbtilesFileSet(_currentBaseMapPath, m_baseMapVectorTiles);
    }
    if (m_terrainMapTiles.removeIf(refersToFile) > 0)
    {
        m_tileServer.removeMbtilesFileSet(_currentTerrainMapPath);
        m_tileServer.addMbtilesFileSet(_currentTerrainMapPath, m_terrainMapTiles);
        emit terrainMapTilesChanged();
    }

    // The tile handler "rasterMap" holds its own reference to the current
    // raster map. Replace it with an invalid MBTILES until the file is back.
    auto currentRasterMap = m_currentRasterMap.value();
    if (!currentRasterMap.isEmpty() && (QFileInfo(fileName).baseName() == currentRasterMap))
    {
        m_tileServer.removeMbtilesFileSet(u"rasterMap"_s);
        const QVector<QSharedPointer<FileFormats::MBTILES>> placeholder {QSharedPointer<FileFormats::MBTILES>(new FileFormats::MBTILES())};
        m_tileServer.addMbtilesFileSet(u"rasterMap"_s, placeholder);
    }
}


GeoMaps::GeoMapProvider::aviationDataCacheResult GeoMaps::GeoMapProvider::fillAviationDataCache(QStringList JSONFileNames, bool hideGlidingSectors)
{
    // Ensure that order is the same every time
    JSONFileNames.sort();

    //
    // Generate new GeoJSON array and new list of waypoints
    //

    // First, create a vector of JSON objects.
    // We use a QSet to keep track of objects that have already been added in order to avoid duplicated entries.
    // The vector is used to ensure that the order of the objects remains identical during runs.
    // A QSet seems to have some built-in randomness and does not do that.
    QVector<QJsonObject> objectVector;
    {
        QSet<QJsonObject> objectSet;
        for(const auto& JSONFileName : JSONFileNames)
        {
            // Read the lock file
            QLockFile lockFile(JSONFileName + u".lock"_s);
            lockFile.lock();
            QFile file(JSONFileName);
            QJsonDocument document;
            if (file.open(QIODevice::ReadOnly))
            {
                document = QJsonDocument::fromJson(file.readAll());
                file.close();
            }
            lockFile.unlock();

            for(const auto& value : document.object()[QStringLiteral("features")].toArray())
            {
                auto object = value.toObject();
                if (objectSet.contains(object))
                {
                    continue;
                }
                objectVector += object;
                objectSet += object;
            }
        }
    }

    // Create vectors of airspaces and waypoints
    QVector<Airspace> newAirspaces;
    QVector<Waypoint> newWaypoints;
    for(const auto& object : std::as_const(objectVector))
    {
        // Check if the current object is a waypoint. If so, add it to the list of waypoints.
        Waypoint const waypoint(object);
        if (waypoint.isValid())
        {
            newWaypoints.append(waypoint);
            continue;
        }

        // Check if the current object is an airspace. If so, add it to the list of airspaces.
        Airspace const airspace(object);
        if (airspace.isValid())
        {
            newAirspaces.append(airspace);
            continue;
        }
    }

    // Then, create a new JSONArray of features and a new list of waypoints
    QJsonArray newFeatures;
    for(const auto& object : std::as_const(objectVector))
    {
        // If 'hideGlidingSector' is set, ignore all objects that are airspaces
        // and that are gliding sectors
        if (hideGlidingSectors)
        {
            Airspace const airspaceTest(object);
            if (airspaceTest.CAT() == u"GLD"_s)
            {
                continue;
            }
        }
        newFeatures += object;
    }

    QByteArray newGeoJSON;
    {
        QJsonObject resultObject;
        resultObject.insert(QStringLiteral("type"), "FeatureCollection");
        resultObject.insert(QStringLiteral("features"), newFeatures);
        QJsonDocument const geoDoc(resultObject);
        newGeoJSON = geoDoc.toJson(QJsonDocument::Compact);
    }

    // Sort waypoints by name
    std::sort(newWaypoints.begin(), newWaypoints.end(), [](const Waypoint& first, const Waypoint& second) {return first.name() < second.name(); });

    return {newWaypoints, buildWaypointIndex(newWaypoints), newAirspaces, newGeoJSON};
}
