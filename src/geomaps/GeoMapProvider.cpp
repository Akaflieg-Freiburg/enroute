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

#include <QApplication>
#include <QGeoCoordinate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlEngine>
#include <QRandomGenerator>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QtConcurrent/QtConcurrent>
#include <chrono>

#include "Clock.h"
#include "GeoMapProvider.h"
#include "Global.h"

using namespace std::chrono_literals;

// Static instance of this class. Do not analyze, because of many unwanted warnings.
#ifndef __clang_analyzer__
QPointer<GeoMaps::GeoMapProvider> geoMapProviderStatic {};
#endif


GeoMaps::GeoMapProvider::GeoMapProvider(QObject *parent)
    : QObject(parent),
      _tileServer(QUrl()),
      _styleFile(nullptr)
{
    // Initialize _combinedGeoJSON_ with an empty document
    QJsonObject resultObject;
    resultObject.insert(QStringLiteral("type"), "FeatureCollection");
    resultObject.insert(QStringLiteral("features"), QJsonArray());
    QJsonDocument geoDoc(resultObject);
    _combinedGeoJSON_ = geoDoc.toJson(QJsonDocument::JsonFormat::Compact);

    _tileServer.listen(QHostAddress(QStringLiteral("127.0.0.1")));
}


GeoMaps::GeoMapProvider::~GeoMapProvider()
{
    QMutexLocker lock(&_aviationDataMutex);

    _airspaces_.clear();
    _waypoints_.clear();
}


auto GeoMaps::GeoMapProvider::airspaces(const QGeoCoordinate& position) -> QVariantList
{
    // Lock data
    QMutexLocker lock(&_aviationDataMutex);

    QVector<Airspace> result;
    result.reserve(10);
    foreach(auto airspace, _airspaces_) {
        if (airspace.polygon().contains(position)) {
            result.append(airspace);
        }
    }

    // Sort airspaces according to lower boundary
    std::sort(result.begin(), result.end(), [](const Airspace& a, const Airspace& b) {return (a.estimatedLowerBoundInFtMSL() > b.estimatedLowerBoundInFtMSL()); });

    QVariantList final;
    foreach(auto airspace, result)
        final.append( QVariant::fromValue(airspace) );

    return final;
}


auto GeoMaps::GeoMapProvider::closestWaypoint(QGeoCoordinate position, const QGeoCoordinate& distPosition, FlightRoute* flightRoute) -> SimpleWaypoint
{
    position.setAltitude(qQNaN());

    SimpleWaypoint result;
    for(auto wp : waypoints()) {
        if (!wp.isValid()) {
            continue;
        }
        if (!result.isValid()) {
            result = wp;
        }
        if (position.distanceTo(wp.coordinate()) < position.distanceTo(result.coordinate())) {
            result = wp;
        }
    }

    if (flightRoute != nullptr) {
        for(auto& variant : flightRoute->midFieldWaypoints() ) {
            GeoMaps::SimpleWaypoint wp = variant.value<GeoMaps::SimpleWaypoint>();
            if (!wp.isValid()) {
                continue;
            }
            if (position.distanceTo(wp.coordinate()) < position.distanceTo(result.coordinate())) {
                result = wp;
            }
        }
    }

    if (position.distanceTo(result.coordinate()) > position.distanceTo(distPosition)) {
        return SimpleWaypoint(position);
    }

    return result;
}


auto GeoMaps::GeoMapProvider::describeMapFile(const QString& fileName) -> QString
{
    QFileInfo fi(fileName);
    if (!fi.exists()) {
        return tr("No information available.");
    }
    QString result = QStringLiteral("<table><tr><td><strong>%1 :&nbsp;&nbsp;</strong></td><td>%2</td></tr><tr><td><strong>%3 :&nbsp;&nbsp;</strong></td><td>%4</td></tr></table>")
            .arg(tr("Installed"),
                 fi.lastModified().toUTC().toString(),
                 tr("File Size"),
                 QLocale::system().formattedDataSize(fi.size(), 1, QLocale::DataSizeSIFormat));

    // Extract infomation from GeoJSON
    if (fileName.endsWith(u".geojson")) {
        QLockFile lockFile(fileName+".lock");
        lockFile.lock();
        QFile file(fileName);
        file.open(QIODevice::ReadOnly);
        auto document = QJsonDocument::fromJson(file.readAll());
        file.close();
        lockFile.unlock();
        QString concatInfoString = document.object()[QStringLiteral("info")].toString();
        if (!concatInfoString.isEmpty()) {
            result += "<p>"+tr("The map data was compiled from the following sources.")+"</p><ul>";
            auto infoStrings = concatInfoString.split(QStringLiteral(";"));
            foreach(auto infoString, infoStrings)
                result += "<li>"+infoString+"</li>";
            result += u"</ul>";
        }
    }

    // Extract infomation from MBTILES
    if (fileName.endsWith(u".mbtiles")) {
        // Open database
        auto databaseConnectionName = "GeoMapProvider::describeMapFile "+fileName;
        auto db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"), databaseConnectionName);
        db.setDatabaseName(fileName);
        db.open();
        if (!db.isOpenError()) {
            // Read metadata from database
            QSqlQuery query(db);
            QString intResult;
            if (query.exec(QStringLiteral("select name, value from metadata;"))) {
                while(query.next()) {
                    QString key = query.value(0).toString();
                    if (key == u"json") {
                        continue;
                    }
                    intResult += QStringLiteral("<tr><td><strong>%1 :&nbsp;&nbsp;</strong></td><td>%2</td></tr>")
                            .arg(key, query.value(1).toString());
                }
            }
            if (!intResult.isEmpty()) {
                result += QStringLiteral("<h4>%1</h4><table>%2</table>").arg(tr("Internal Map Data"), intResult);
            }
            db.close();
        }
    }

    return result;
}


auto GeoMaps::GeoMapProvider::filteredWaypointObjects(const QString &filter) -> QVariantList
{
    auto wps = waypoints();

    QStringList filterWords;
    foreach(auto word, filter.simplified().split(' ', Qt::SkipEmptyParts)) {
        QString simplifiedWord = Librarian::globalInstance()->simplifySpecialChars(word);
        if (simplifiedWord.isEmpty()) {
            continue;
        }
        filterWords.append(simplifiedWord);
    }

    QVariantList result;
    foreach(auto wp, wps) {
        if (!wp.isValid()) {
            continue;
        }
        bool allWordsFound = true;
        foreach(auto word, filterWords) {
            QString fullName = Librarian::globalInstance()->simplifySpecialChars(wp.getPropery(QStringLiteral("NAM")).toString());
            QString codeName = Librarian::globalInstance()->simplifySpecialChars(wp.getPropery(QStringLiteral("COD")).toString());
            QString wordx = Librarian::globalInstance()->simplifySpecialChars(word);

            if (!fullName.contains(wordx, Qt::CaseInsensitive) && !codeName.contains(wordx, Qt::CaseInsensitive)) {
                allWordsFound = false;
                break;
            }
        }
        if (allWordsFound) {
            result.append( QVariant::fromValue(wp) );
        }
    }

    return result;
}


auto GeoMaps::GeoMapProvider::findByID(const QString &id) -> SimpleWaypoint
{
    auto wps = waypoints();

    foreach(auto wp, wps) {
        if (!wp.isValid()) {
            continue;
        }
        if (wp.getPropery(QStringLiteral("COD")).toString() == id) {
            return wp;
        }
    }
    return {};
}


auto GeoMaps::GeoMapProvider::globalInstance() -> GeoMaps::GeoMapProvider*
{
#ifndef __clang_analyzer__
    if (geoMapProviderStatic.isNull()) {
        geoMapProviderStatic = new GeoMaps::GeoMapProvider();
    }
    return geoMapProviderStatic;
#else
    return nullptr;
#endif
}


auto GeoMaps::GeoMapProvider::nearbyWaypoints(const QGeoCoordinate& position, const QString& type) -> QVariantList
{
    auto wps = waypoints();

    QVector<SimpleWaypoint> tWps;
    foreach(auto wp, wps) {
        if (!wp.isValid()) {
            continue;
        }
        if (wp.getPropery(QStringLiteral("TYP")).toString() != type) {
            continue;
        }
        tWps.append(wp);
    }

    std::sort(tWps.begin(), tWps.end(), [position](const SimpleWaypoint &a, const SimpleWaypoint &b) {return position.distanceTo(a.coordinate()) < position.distanceTo(b.coordinate()); });

    QVariantList result;
    int sz = 0;
    foreach(auto ad, tWps) {
        result.append( QVariant::fromValue(ad) );
        sz++;
        if (sz == 20) {
            break;
        }
    }

    return result;
}


auto GeoMaps::GeoMapProvider::styleFileURL() const -> QString
{
    if (_styleFile.isNull()) {
        return QStringLiteral(":/flightMap/empty.json");
    }
    return "file://"+_styleFile->fileName();
}


void GeoMaps::GeoMapProvider::aviationMapsChanged()
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
    foreach(auto geoMapPtr, Global::mapManager()->aviationMaps()->downloadables()) {
        // Ignore everything but geojson files
        if (!geoMapPtr->fileName().endsWith(u".geojson", Qt::CaseInsensitive)) {
            continue;
        }
        if (!geoMapPtr->hasFile()) {
            continue;
        }
        JSONFileNames += geoMapPtr->fileName();
    }

    _aviationDataCacheFuture = QtConcurrent::run(this, &GeoMaps::GeoMapProvider::fillAviationDataCache, JSONFileNames, Settings::hideUpperAirspacesStatic());
}


void GeoMaps::GeoMapProvider::baseMapsChanged()
{

    // Delete old style file, stop serving tiles
    delete _styleFile;
    _tileServer.removeMbtilesFileSet(_currentPath);

    // Serve new tile set under new name
    _currentPath = QString::number(QRandomGenerator::global()->bounded(static_cast<quint32>(1000000000)));
    _tileServer.addMbtilesFileSet(Global::mapManager()->baseMaps()->downloadablesWithFile(), _currentPath);

    // Generate new mapbox style file
    _styleFile = new QTemporaryFile(this);
    QFile file(QStringLiteral(":/flightMap/osm-liberty.json"));
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    data.replace("%URL%", (_tileServer.serverUrl()+"/"+_currentPath).toLatin1());
    data.replace("%URL2%", _tileServer.serverUrl().toLatin1());
    _styleFile->open();
    _styleFile->write(data);
    _styleFile->close();

    emit styleFileURLChanged();

}


void GeoMaps::GeoMapProvider::fillAviationDataCache(const QStringList& JSONFileNames, bool hideUpperAirspaces)
{
    //
    // Generate new GeoJSON array and new list of waypoints
    //

    // First, create a set of JSON objects, in order to avoid duplicated entries
    QSet<QJsonObject> objectSet;
    foreach(auto JSONFileName, JSONFileNames) {
        // Read the lock file
        QLockFile lockFile(JSONFileName+".lock");
        lockFile.lock();
        QFile file(JSONFileName);
        file.open(QIODevice::ReadOnly);
        auto document = QJsonDocument::fromJson(file.readAll());
        file.close();
        lockFile.unlock();

        foreach(auto value, document.object()["features"].toArray()) {
            auto object = value.toObject();

            // If 'hideUpperAirspaces' is set, ignore all objects that are airspaces
            // and that begin at FL100 or above.
            if (hideUpperAirspaces) {
                Airspace airspaceTest(object);
                if (airspaceTest.isUpper()) {
                    continue;
                }
            }
            objectSet += object;
        }
    }


    // Then, create a new JSONArray of features and a new list of waypoints
    QJsonArray newFeatures;
    QVector<Airspace> newAirspaces;
    QVector<SimpleWaypoint> newWaypoints;
    foreach(auto object, objectSet) {
        newFeatures += object;

        // Check if the current object is a waypoint. If so, add it to the list of waypoints.
        SimpleWaypoint wp(object);
        if (wp.isValid()) {
            newWaypoints.append(wp);
            continue;
        }

        // Check if the current object is an airspace. If so, add it to the list of airspaces.
        Airspace as(object);
        if (as.isValid()) {
            newAirspaces.append(as);
            continue;
        }
    }
    QJsonObject resultObject;
    resultObject.insert(QStringLiteral("type"), "FeatureCollection");
    resultObject.insert(QStringLiteral("features"), newFeatures);
    QJsonDocument geoDoc(resultObject);

    // Sort waypoints by name
    std::sort(newWaypoints.begin(), newWaypoints.end(), [](const SimpleWaypoint &a, const SimpleWaypoint &b) {return a.getPropery(QStringLiteral("NAM")).toString() < b.getPropery(QStringLiteral("NAM")).toString(); });

    _aviationDataMutex.lock();
    _airspaces_ = newAirspaces;
    _waypoints_ = newWaypoints;
    _combinedGeoJSON_ = geoDoc.toJson(QJsonDocument::JsonFormat::Compact);
    _aviationDataMutex.unlock();

    emit geoJSONChanged();
}

#warning do not want this. use application-global static!
void GeoMaps::GeoMapProvider::setDownloadManager(Weather::DownloadManager *downloadManager)
{
    if (downloadManager == nullptr) {
        return;
    }
    _downloadManager = downloadManager;


    // Connect the Downloadmanager, so aviation maps will be generated
    connect(Global::mapManager()->aviationMaps(), &DownloadableGroup::localFileContentChanged_delayed, this, &GeoMaps::GeoMapProvider::aviationMapsChanged);
    connect(Global::mapManager()->baseMaps(), &DownloadableGroup::localFileContentChanged_delayed, this, &GeoMaps::GeoMapProvider::baseMapsChanged);
    connect(Global::settings(), &Settings::hideUpperAirspacesChanged, this, &GeoMaps::GeoMapProvider::aviationMapsChanged);

    _aviationDataCacheTimer.setSingleShot(true);
    _aviationDataCacheTimer.setInterval(3s);
    connect(&_aviationDataCacheTimer, &QTimer::timeout, this, &GeoMaps::GeoMapProvider::aviationMapsChanged);

    aviationMapsChanged();
    baseMapsChanged();
}
