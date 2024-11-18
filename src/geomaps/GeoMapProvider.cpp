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

#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLockFile>
#include <QQmlEngine>
#include <QRandomGenerator>
#include <QtConcurrent/QtConcurrentRun>

#include "GlobalSettings.h"
#include "Librarian.h"
#include "dataManagement/DataManager.h"
#include "fileFormats/MBTILES.h"
#include "geomaps/GeoMapProvider.h"
#include "geomaps/WaypointLibrary.h"
#include "navigation/Navigator.h"

using namespace Qt::Literals::StringLiterals;


GeoMaps::GeoMapProvider::GeoMapProvider(QObject *parent)
    : GlobalObject(parent)
{
    _combinedGeoJSON_ = emptyGeoJSON();

    QFile geoJSONCacheFile(geoJSONCache);
    geoJSONCacheFile.open(QFile::ReadOnly);
    _combinedGeoJSON_ = geoJSONCacheFile.readAll();
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
    connect(GlobalObject::globalSettings(), &GlobalSettings::airspaceAltitudeLimitChanged, this, &GeoMaps::GeoMapProvider::onAviationMapsChanged);
    connect(GlobalObject::globalSettings(), &GlobalSettings::hideGlidingSectorsChanged, this, &GeoMaps::GeoMapProvider::onAviationMapsChanged);

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
        result += "<h4>"+tr("Aviation Maps")+"</h4>\n";
        result += "<p>"+tr("The aeronautical maps are compiled from databases provided by the "
                           "<a href='http://openaip.net'>openAIP</a> and "
                           "<a href='https://www.openflightmaps.org/'>open flightmaps</a> "
                           "projects.")+"</p>\n";
        result += QStringLiteral("<a href='https://openAIP.net'>© openAIP</a><br>"
                                 "<a href='https://openflightmaps.org'>© open flightmaps</a>");
    }

    if (GlobalObject::dataManager()->baseMaps()->hasFile())
    {
        result += "<h4>"+tr("Base Maps")+"</h4>\n";
        result += "<p>"+tr("The base maps are generated from "
                           "<a href='https://www.openstreetmap.org'>Open Streetmap</a> data.")+"</p>\n";
        result += QStringLiteral("<a href='https://www.openstreetmap.org/about'>© OpenStreetMap contributors</a>");
    }

    if (GlobalObject::dataManager()->terrainMaps()->hasFile())
    {
        result += "<h4>"+tr("Terrain Maps")+"</h4>\n";
        result += "<p>"+tr("The terrain maps are derived from the "
                           "<a href='https://registry.opendata.aws/terrain-tiles/'>Terrain "
                           "Tiles Open Dataset on Amazon AWS</a>.") + "</p>" +
                "<ul style='margin-left:-25px;'>"
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
                "</ul>";
    }

    return result;
}

QByteArray GeoMaps::GeoMapProvider::geoJSON()
{
    QMutexLocker const lock(&_aviationDataMutex);
    return _combinedGeoJSON_;
}

QString GeoMaps::GeoMapProvider::styleFileURL()
{
    if (m_styleFile.isNull())
    {
        QFile file;
        if (GlobalObject::dataManager()->baseMaps()->hasFile())
        {
            file.setFileName(QStringLiteral(":/flightMap/osm-liberty.json"));
        }
        else
        {
            file.setFileName(QStringLiteral(":/flightMap/empty.json"));
        }

        file.open(QIODevice::ReadOnly);
        QByteArray data = file.readAll();
        data.replace("%URL%", (m_tileServer.serverUrl()+"/"+_currentBaseMapPath).toLatin1());
        data.replace("%URLT%", (m_tileServer.serverUrl()+"/"+_currentTerrainMapPath).toLatin1());
        data.replace("%URL2%", m_tileServer.serverUrl().toLatin1());

        m_styleFile = new QTemporaryFile(this);
        m_styleFile->open();
        m_styleFile->write(data);
        m_styleFile->close();
    }
    return "file://"+m_styleFile->fileName();
}


//
// Methods
//

QVariantList GeoMaps::GeoMapProvider::airspaces(const QGeoCoordinate& position)
{
    // Lock data
    QMutexLocker const lock(&_aviationDataMutex);

    QVector<Airspace> result;
    result.reserve(10);
    foreach(auto airspace, _airspaces_) {
        if (airspace.polygon().contains(position)) {
            result.append(airspace);
        }
    }

    // Sort airspaces according to lower boundary
    std::sort(result.begin(), result.end(), [](const Airspace& first, const Airspace& second) {return (first.estimatedLowerBoundMSL() > second.estimatedLowerBoundMSL()); });

    QVariantList final;
    foreach(auto airspace, result)
        final.append( QVariant::fromValue(airspace) );

    return final;
}

GeoMaps::Waypoint GeoMaps::GeoMapProvider::closestWaypoint(QGeoCoordinate position, const QGeoCoordinate& distPosition)
{
    position.setAltitude(qQNaN());

    Waypoint result;
    const auto wayppoints = waypoints();
    for(const auto& waypoint : wayppoints) {
        if (!waypoint.isValid()) {
            continue;
        }
        if (!result.isValid()) {
            result = waypoint;
        }
        if (position.distanceTo(waypoint.coordinate()) < position.distanceTo(result.coordinate())) {
            result = waypoint;
        }
    }

    const auto wpLibrary = GlobalObject::waypointLibrary()->waypoints();
    for(const auto& wayppoint : wpLibrary) {
        if (!wayppoint.isValid()) {
            continue;
        }
        if (!result.isValid()) {
            result = wayppoint;
        }
        if (position.distanceTo(wayppoint.coordinate()) < position.distanceTo(result.coordinate())) {
            result = wayppoint;
        }
    }

    for(auto& waypoint : GlobalObject::navigator()->flightRoute()->midFieldWaypoints() ) {
        if (!waypoint.isValid()) {
            continue;
        }
        if (position.distanceTo(waypoint.coordinate()) < position.distanceTo(result.coordinate())) {
            result = waypoint;
        }
    }

    if (position.distanceTo(result.coordinate()) > position.distanceTo(distPosition)) {
        position.setAltitude( terrainElevationAMSL(position).toM() );
        return {position};
    }

    return result;
}

Units::Distance GeoMaps::GeoMapProvider::terrainElevationAMSL(const QGeoCoordinate& coordinate)
{
    int const zoomMin = 6;
    int const zoomMax = 10;

    for(int zoom = zoomMax; zoom >= zoomMin; zoom--)
    {
        auto tilex = (coordinate.longitude()+180.0)/360.0 * (1<<zoom);
        auto tiley = (1.0 - asinh(tan(qDegreesToRadians(coordinate.latitude())))/M_PI)/2.0 * (1<<zoom);
        auto intraTileX = qRound(255.0*(tilex-floor(tilex)));
        auto intraTileY = qRound(255.0*(tiley-floor(tiley)));

        qint64 const keyA = qFloor(tilex) & 0xFFFF;
        qint64 const keyB = qFloor(tiley) & 0xFFFF;
        qint64 const key = (keyA << 32) + (keyB << 16) + zoom;

        if (terrainTileCache.contains(key))
        {
            auto* tileImg = terrainTileCache.object(key);
            if (tileImg->isNull())
            {
                continue;
            }
            auto pix = tileImg->pixel(intraTileX, intraTileY);
            double const elevation = (qRed(pix) * 256.0 + qGreen(pix) + qBlue(pix) / 256.0)
                                     - 32768.0;
            return Units::Distance::fromM(elevation);
        }
    }


    foreach(auto mbtPtr, m_terrainMapTiles)
    {
        if (mbtPtr.isNull())
        {
            continue;
        }

        for(int zoom = zoomMax; zoom >= zoomMin; zoom--)
        {
            auto tilex = (coordinate.longitude()+180.0)/360.0 * (1<<zoom);
            auto tiley = (1.0 - asinh(tan(qDegreesToRadians(coordinate.latitude())))/M_PI)/2.0 * (1<<zoom);
            auto intraTileX = qRound(255.0*(tilex-floor(tilex)));
            auto intraTileY = qRound(255.0*(tiley-floor(tiley)));

            qint64 const keyA = qFloor(tilex) & 0xFFFF;
            qint64 const keyB = qFloor(tiley) & 0xFFFF;
            qint64 const key = (keyA << 32) + (keyB << 16) + zoom;

            auto tileData = mbtPtr->tile(zoom, qFloor(tilex), qFloor(tiley));
            if (!tileData.isEmpty())
            {
                auto* tileImg = new QImage();
                tileImg->loadFromData(tileData);
                if (tileImg->isNull())
                {
                    delete tileImg;
                    continue;
                }
                terrainTileCache.insert(key,tileImg);

                auto pix = tileImg->pixel(intraTileX, intraTileY);
                double const elevation = (qRed(pix) * 256.0 + qGreen(pix) + qBlue(pix) / 256.0)
                                         - 32768.0;
                return Units::Distance::fromM(elevation);
            }
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

QVector<GeoMaps::Waypoint> GeoMaps::GeoMapProvider::filteredWaypoints(const QString& filter)
{

    QStringList filterWords;
    foreach(auto word, filter.simplified().split(' ', Qt::SkipEmptyParts)) {
        QString const simplifiedWord = GlobalObject::librarian()->simplifySpecialChars(word);
        if (simplifiedWord.isEmpty()) {
            continue;
        }
        filterWords.append(simplifiedWord);
    }

    QVector<GeoMaps::Waypoint> result;

    const auto wps = waypoints();
    for(const auto& waypoint : wps) {
        if (!waypoint.isValid()) {
            continue;
        }
        bool allWordsFound = true;
        foreach(auto word, filterWords) {
            QString const fullName = GlobalObject::librarian()->simplifySpecialChars(waypoint.name());
            if (!fullName.contains(word, Qt::CaseInsensitive) && !waypoint.ICAOCode().contains(word, Qt::CaseInsensitive)) {
                allWordsFound = false;
                break;
            }
        }
        if (allWordsFound) {
            result.append( waypoint );
        }
    }

    const auto wpsLib = GlobalObject::waypointLibrary()->waypoints();
    for(const auto& waypoint : wpsLib)
    {
        if (!waypoint.isValid())
        {
            continue;
        }
        bool allWordsFound = true;
        for(auto& word : filterWords)
        {
            QString const fullName = GlobalObject::librarian()->simplifySpecialChars(waypoint.name());
            if (!fullName.contains(word, Qt::CaseInsensitive) && !waypoint.ICAOCode().contains(word, Qt::CaseInsensitive))
            {
                allWordsFound = false;
                break;
            }
        }
        if (allWordsFound) {
            result.append( waypoint );
        }
    }

    std::sort(result.begin(), result.end(), [](const Waypoint& first, const Waypoint& second) {return first.name() < second.name(); });

    return result;
}

GeoMaps::Waypoint GeoMaps::GeoMapProvider::findByID(const QString& icaoID)
{
    auto wps = waypoints();

    for(auto wayppoint : wps)
    {
        if (!wayppoint.isValid())
        {
            continue;
        }
        if (wayppoint.ICAOCode() == icaoID) {
            return wayppoint;
        }
    }
    return {};
}

QList<GeoMaps::Waypoint> GeoMaps::GeoMapProvider::nearbyWaypoints(const QGeoCoordinate& position, const QString& type)
{
    QVector<Waypoint> tWps;

    auto wps = waypoints();
    for(auto& waypoint : wps)
    {
        if (!waypoint.isValid())
        {
            continue;
        }
        if (waypoint.type() != type)
        {
            continue;
        }
        tWps.append(waypoint);
    }

    auto wpsLib = GlobalObject::waypointLibrary()->waypoints();
    for(const auto& waypoint : wpsLib)
    {
        if (!waypoint.isValid())
        {
            continue;
        }
        if (waypoint.type() != type)
        {
            continue;
        }
        tWps.append(waypoint);
    }

    std::sort(tWps.begin(), tWps.end(), [position](const Waypoint& first, const Waypoint& second) {return position.distanceTo(first.coordinate()) < position.distanceTo(second.coordinate()); });

    return tWps.mid(0,20);
}

QVector<GeoMaps::Waypoint> GeoMaps::GeoMapProvider::waypoints()
{
    QMutexLocker const locker(&_aviationDataMutex);
    return _waypoints_;
}

void GeoMaps::GeoMapProvider::setCurrentRasterMap(const QString& mapName)
{
    if (mapName == m_currentRasterMap)
    {
        return;
    }

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

    m_tileServer.removeMbtilesFileSet(u"rasterMap"_s);
    const QVector<QSharedPointer<FileFormats::MBTILES>> single {newRasterMap};
    m_tileServer.addMbtilesFileSet(u"rasterMap"_s, single);
    m_currentRasterMap = newRasterMapName;
    emit styleFileURLChanged();
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
    if (_aviationDataCacheFuture.isRunning()) {
        _aviationDataCacheTimer.start();
        return;
    }

    //
    // Generate new GeoJSON array and new list of waypoints
    //
    QStringList JSONFileNames;
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
        JSONFileNames += geoMapPtr->fileName();
    }

    _aviationDataCacheFuture = QtConcurrent::run(&GeoMaps::GeoMapProvider::fillAviationDataCache, this, JSONFileNames, GlobalObject::globalSettings()->airspaceAltitudeLimit(), GlobalObject::globalSettings()->hideGlidingSectors());
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

    // Update style file
    delete m_styleFile;
    emit styleFileURLChanged();
}

void GeoMaps::GeoMapProvider::fillAviationDataCache(QStringList JSONFileNames, Units::Distance airspaceAltitudeLimit, bool hideGlidingSectors)
{
    // Avoid rounding errors
    airspaceAltitudeLimit = airspaceAltitudeLimit-Units::Distance::fromFT(1);

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
            QLockFile lockFile(JSONFileName+".lock");
            lockFile.lock();
            QFile file(JSONFileName);
            file.open(QIODevice::ReadOnly);
            auto document = QJsonDocument::fromJson(file.readAll());
            file.close();
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
    for(const auto& object : objectVector)
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
    for(const auto& object : objectVector)
    {
        // Ignore all objects that are airspaces and that begin above the airspaceAltitudeLimit.
        Airspace const airspaceTest(object);
        if (airspaceAltitudeLimit.isFinite() && (airspaceTest.estimatedLowerBoundMSL() > airspaceAltitudeLimit))
        {
            continue;
        }

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
        newGeoJSON = geoDoc.toJson();
    }
    auto _geoJSONChanged = (newGeoJSON != _combinedGeoJSON_);
    auto _waypointsChanged = (newWaypoints != _waypoints_);

    // Sort waypoints by name
    std::sort(newWaypoints.begin(), newWaypoints.end(), [](const Waypoint& first, const Waypoint& second) {return first.name() < second.name(); });

    _aviationDataMutex.lock();
    _airspaces_ = newAirspaces;
    if (_waypointsChanged)
    {
        _waypoints_ = newWaypoints;
    }
    if (_geoJSONChanged)
    {
        _combinedGeoJSON_ = newGeoJSON;
        QFile geoJSONCacheFile(geoJSONCache);
        geoJSONCacheFile.open(QFile::WriteOnly);
        geoJSONCacheFile.write(_combinedGeoJSON_);
        geoJSONCacheFile.close();
    }
    _aviationDataMutex.unlock();

    if (_waypointsChanged)
    {
        emit waypointsChanged();
    }
    if (_geoJSONChanged)
    {
        emit geoJSONChanged();
    }

}
