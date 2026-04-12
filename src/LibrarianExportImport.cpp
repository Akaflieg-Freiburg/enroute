/***************************************************************************
 *   Copyright (C) 2020-2026 by Stefan Kebekus                             *
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

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include "GlobalObject.h"
#include "Librarian.h"
#include "LibrarianExportImport.h"
#include "dataManagement/DataManager.h"
#include "fileFormats/DataFileAbstract.h"
#include "geomaps/Waypoint.h"
#include "geomaps/WaypointLibrary.h"
#include "navigation/Aircraft.h"
#include "navigation/FlightRoute.h"
#include "platform/FileExchange.h"

using namespace Qt::Literals::StringLiterals;


// ————————————————————————————————————————————————
//  Export helpers
// ————————————————————————————————————————————————

auto LibrarianExportImport::createAircraftJsonArray(QStringList& errors) -> QJsonArray
{
    QJsonArray aircraftArray;

    QDir const dir(Librarian::directory(Librarian::Aircraft));
    auto fileList = dir.entryInfoList(QDir::Files);
    int failed = 0;

    for (const QFileInfo& fileInfo : fileList)
    {
        Navigation::Aircraft aircraft;
        auto errorMsg = aircraft.loadFromJSON(fileInfo.absoluteFilePath());
        if (!errorMsg.isEmpty())
        {
            failed++;
            continue;
        }

        // Use the internal serialization format directly
        auto jsonData = aircraft.toJSON();
        auto doc = QJsonDocument::fromJson(jsonData);
        if (doc.isNull())
        {
            failed++;
            continue;
        }

        aircraftArray.append(doc.object());
    }

    if (failed > 0)
    {
        errors += tr("%1 of %2 aircraft could not be exported.").arg(failed).arg(fileList.count());
    }

    return aircraftArray;
}


auto LibrarianExportImport::createRoutesJsonArray(QStringList& errors) -> QJsonArray
{
    QJsonArray routeArray;

    QDir const dir(Librarian::directory(Librarian::Routes));
    auto fileList = dir.entryInfoList(QDir::Files);
    int failed = 0;

    for (const QFileInfo& fileInfo : fileList)
    {
        Navigation::FlightRoute route;
        auto errorMsg = route.load(fileInfo.absoluteFilePath());
        if (!errorMsg.isEmpty())
        {
            failed++;
            continue;
        }

        // Embed the internal GeoJSON serialization
        auto geoJsonData = route.toGeoJSON();
        auto geoJsonDoc = QJsonDocument::fromJson(geoJsonData);
        if (geoJsonDoc.isNull())
        {
            failed++;
            continue;
        }

        QJsonObject routeObj;
        routeObj.insert(QStringLiteral("name"), fileInfo.baseName());
        routeObj.insert(QStringLiteral("route"), geoJsonDoc.object());

        routeArray.append(routeObj);
    }

    if (failed > 0)
    {
        errors += tr("%1 of %2 routes could not be exported.").arg(failed).arg(fileList.count());
    }

    return routeArray;
}


auto LibrarianExportImport::createWaypointsJsonArray(QStringList& errors) -> QJsonArray
{
    Q_UNUSED(errors)

    auto* waypointLibrary = GlobalObject::waypointLibrary();
    if (waypointLibrary == nullptr)
    {
        return {};
    }

    // Use the internal GeoJSON serialization directly.
    // WaypointLibrary::GeoJSON() returns a FeatureCollection; we extract the
    // features array so the backup format stays an array of waypoint objects.
    auto geoJsonData = waypointLibrary->GeoJSON();
    auto doc = QJsonDocument::fromJson(geoJsonData);
    if (doc.isNull())
    {
        return {};
    }

    return doc.object().value(QStringLiteral("features")).toArray();
}


auto LibrarianExportImport::createMapsJsonArray() -> QJsonArray
{
    QJsonArray mapArray;

    auto* dataManager = GlobalObject::dataManager();
    if (dataManager == nullptr)
    {
        return mapArray;
    }

    auto* allItems = dataManager->items();
    if (allItems == nullptr)
    {
        return mapArray;
    }

    // Collect installed types per map name, preserving insertion order
    QStringList orderedNames;
    QHash<QString, QStringList> typesByName;

    for (auto* item : allItems->downloadables())
    {
        if (item == nullptr || !item->hasFile())
        {
            continue;
        }

        auto name = item->objectName();
        QString typeStr;

        switch (item->contentType())
        {
        case DataManagement::Downloadable_Abstract::AviationMap:
            typeStr = u"aviation"_s;
            break;
        case DataManagement::Downloadable_Abstract::BaseMapVector:
            typeStr = u"base-vector"_s;
            break;
        case DataManagement::Downloadable_Abstract::BaseMapRaster:
            typeStr = u"base-raster"_s;
            break;
        case DataManagement::Downloadable_Abstract::TerrainMap:
            typeStr = u"terrain"_s;
            break;
        case DataManagement::Downloadable_Abstract::Data:
            typeStr = u"data"_s;
            break;
        case DataManagement::Downloadable_Abstract::MapSet:
            typeStr = u"mapset"_s;
            break;
        default:
            continue;
        }

        if (!typesByName.contains(name))
        {
            orderedNames.append(name);
        }
        typesByName[name].append(typeStr);
    }

    for (const auto& name : orderedNames)
    {
        QJsonObject mapObj;
        mapObj.insert(QStringLiteral("name"), name);

        QJsonArray typesArray;
        for (const auto& t : typesByName[name])
        {
            typesArray.append(t);
        }
        mapObj.insert(QStringLiteral("types"), typesArray);

        mapArray.append(mapObj);
    }

    return mapArray;
}


// ————————————————————————————————————————————————
//  Export
// ————————————————————————————————————————————————

auto LibrarianExportImport::exportAndShareBackup() -> QString
{
    QStringList errors;

    // Create the combined backup object
    QJsonObject rootObj;
    rootObj.insert(QStringLiteral("content"), QStringLiteral("enroute-backup"));
    rootObj.insert(QStringLiteral("version"), backupFormatVersion);
    rootObj.insert(QStringLiteral("timestamp"), QDateTime::currentDateTimeUtc().toString(Qt::ISODate));
    rootObj.insert(QStringLiteral("aircraft"), createAircraftJsonArray(errors));
    rootObj.insert(QStringLiteral("routes"), createRoutesJsonArray(errors));
    rootObj.insert(QStringLiteral("waypoints"), createWaypointsJsonArray(errors));
    rootObj.insert(QStringLiteral("maps"), createMapsJsonArray());

    QJsonDocument doc;
    doc.setObject(rootObj);
    const QByteArray backupData = doc.toJson();

    // Build a date-stamped file name
    const QString fileName = u"EnrouteBackup-"_s
        + QDateTime::currentDateTimeUtc().date().toString(Qt::ISODate);

    // Share / save via FileExchange
    auto* fileExchange = GlobalObject::fileExchange();
    if (fileExchange == nullptr)
    {
        return tr("Cannot export backup. File exchange not available.");
    }
    const QString shareError = fileExchange->shareContent(
        backupData, u"application/json"_s, u"json"_s, fileName);

    // Return abort immediately so the QML can distinguish it
    if (shareError == u"abort"_s)
    {
        return shareError;
    }

    // Combine export warnings and share errors into one result
    if (!shareError.isEmpty())
    {
        errors += shareError;
    }
    return errors.join(u"<br>"_s);
}


// ————————————————————————————————————————————————
//  Import
// ————————————————————————————————————————————————

auto LibrarianExportImport::importFullBackup(const QByteArray& content) -> QString
{
    // Parse the backup file
    QJsonParseError parseError{};
    auto document = QJsonDocument::fromJson(content, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        return tr("Cannot read backup file. Error: %1").arg(parseError.errorString());
    }

    if (!document.isObject())
    {
        return tr("Cannot read backup file. Error: Document is not a JSON object.");
    }

    auto rootObj = document.object();

    // Verify it's a backup file
    if (rootObj.value(QStringLiteral("content")).toString() != u"enroute-backup")
    {
        return tr("Cannot read backup file. Error: File is not an Enroute backup.");
    }

    // Check backup format version
    auto fileVersion = rootObj.value(QStringLiteral("version")).toInt(0);
    if (fileVersion < 1)
    {
        return tr("Cannot read backup file. Error: No valid version number found.");
    }
    if (fileVersion > backupFormatVersion)
    {
        return tr("Cannot read backup file. The file was created by a newer version of Enroute Flight Navigation. Please update the app.");
    }

    QStringList errors;

    // Import aircraft — each element is an internal Aircraft JSON object
    int aircraftFailed = 0;
    auto aircraftArray = rootObj.value(QStringLiteral("aircraft")).toArray();
    for (const auto& aircraftValue : aircraftArray)
    {
        if (!aircraftValue.isObject())
        {
            aircraftFailed++;
            continue;
        }

        // Serialize the JSON object back to bytes and use the internal loader
        QJsonDocument aircraftDoc;
        aircraftDoc.setObject(aircraftValue.toObject());

        Navigation::Aircraft aircraft;
        auto errorMsg = aircraft.loadFromJSON(aircraftDoc.toJson());
        if (!errorMsg.isEmpty())
        {
            aircraftFailed++;
            continue;
        }

        // Skip exact duplicates
        if (Librarian::contains(aircraft))
        {
            continue;
        }

        // Get aircraft name for filename
        QString aircraftName = aircraft.name();
        if (aircraftName.isEmpty())
        {
            aircraftName = u"Imported Aircraft"_s;
        }

        // Save with unique name
        auto savePath = Librarian::fullPath(Librarian::Aircraft, aircraftName);
        if (QFile::exists(savePath))
        {
            for (int i = 1; ; i++)
            {
                auto newName = u"%1 (%2)"_s.arg(aircraftName).arg(i);
                savePath = Librarian::fullPath(Librarian::Aircraft, newName);
                if (!QFile::exists(savePath))
                {
                    break;
                }
            }
        }

        auto saveError = aircraft.save(savePath);
        if (!saveError.isEmpty())
        {
            aircraftFailed++;
            continue;
        }
    }
    if (aircraftFailed > 0)
    {
        errors += tr("%1 of %2 aircraft could not be imported.").arg(aircraftFailed).arg(aircraftArray.count());
    }

    // Import routes — each element has a "name" and a "route" GeoJSON object
    int routesFailed = 0;
    auto routesArray = rootObj.value(QStringLiteral("routes")).toArray();
    for (const auto& routeValue : routesArray)
    {
        if (!routeValue.isObject())
        {
            routesFailed++;
            continue;
        }

        auto routeObj = routeValue.toObject();

        // Extract route name
        QString routeName = routeObj.value(QStringLiteral("name")).toString();
        if (routeName.isEmpty())
        {
            routeName = u"Imported Route"_s;
        }

        // The "route" field contains the internal GeoJSON FeatureCollection
        auto geoJsonObj = routeObj.value(QStringLiteral("route")).toObject();
        if (geoJsonObj.isEmpty())
        {
            routesFailed++;
            continue;
        }

        QJsonDocument geoJsonDoc;
        geoJsonDoc.setObject(geoJsonObj);

        Navigation::FlightRoute route;
        auto errorMsg = route.loadFromGeoJSON(geoJsonDoc.toJson());
        if (!errorMsg.isEmpty())
        {
            routesFailed++;
            continue;
        }

        // Skip exact duplicates
        if (Librarian::contains(&route))
        {
            continue;
        }

        // Save with unique name
        auto savePath = Librarian::fullPath(Librarian::Routes, routeName);
        if (QFile::exists(savePath))
        {
            for (int i = 1; ; i++)
            {
                auto newName = u"%1 (%2)"_s.arg(routeName).arg(i);
                savePath = Librarian::fullPath(Librarian::Routes, newName);
                if (!QFile::exists(savePath))
                {
                    break;
                }
            }
        }

        auto saveError = route.save(savePath);
        if (!saveError.isEmpty())
        {
            routesFailed++;
            continue;
        }
    }
    if (routesFailed > 0)
    {
        errors += tr("%1 of %2 routes could not be imported.").arg(routesFailed).arg(routesArray.count());
    }

    // Import waypoints — each element is an internal GeoJSON Feature object
    int waypointsFailed = 0;
    auto waypointsArray = rootObj.value(QStringLiteral("waypoints")).toArray();
    if (!waypointsArray.isEmpty())
    {
        auto* waypointLibrary = GlobalObject::waypointLibrary();
        for (const auto& waypointValue : waypointsArray)
        {
            if (!waypointValue.isObject())
            {
                waypointsFailed++;
                continue;
            }

            // Use the internal Waypoint(QJsonObject) constructor
            const GeoMaps::Waypoint waypoint(waypointValue.toObject());
            if (!waypoint.isValid())
            {
                waypointsFailed++;
                continue;
            }

            if (!waypointLibrary->contains(waypoint))
            {
                waypointLibrary->add(waypoint);
            }
        }
    }
    if (waypointsFailed > 0)
    {
        errors += tr("%1 of %2 waypoints could not be imported.").arg(waypointsFailed).arg(waypointsArray.count());
    }

    if (!errors.isEmpty())
    {
        return errors.join(u"<br>"_s);
    }
    return {}; // Success
}


auto LibrarianExportImport::importFullBackupFromFile(const QString& fileName) -> QString
{
    auto file = FileFormats::DataFileAbstract::openFileURL(fileName);
    if (!file->open(QIODevice::ReadOnly))
    {
        return tr("Cannot open backup file.");
    }

    auto content = file->readAll();
    file->close();

    return importFullBackup(content);
}
