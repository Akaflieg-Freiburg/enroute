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

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDateTime>
#include <QDir>

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
//  Unit string conversion helpers
// ————————————————————————————————————————————————

static QString fuelConsumptionUnitToString(int unit)
{
    switch (unit) {
    case 0:  return QStringLiteral("l/h");
    case 1:  return QStringLiteral("gal/h");
    default: return QStringLiteral("l/h");
    }
}

static int fuelConsumptionUnitFromString(const QJsonValue& value)
{
    if (value.toString() == u"gal/h") { return 1; }
    return 0; // default: l/h
}

static QString horizontalDistanceUnitToString(int unit)
{
    switch (unit) {
    case 0:  return QStringLiteral("NM");
    case 1:  return QStringLiteral("km");
    case 2:  return QStringLiteral("mi");
    default: return QStringLiteral("NM");
    }
}

static int horizontalDistanceUnitFromString(const QJsonValue& value)
{
    auto str = value.toString();
    if (str == u"km") { return 1; }
    if (str == u"mi") { return 2; }
    return 0; // default: NM
}

static QString verticalDistanceUnitToString(int unit)
{
    switch (unit) {
    case 0:  return QStringLiteral("ft");
    case 1:  return QStringLiteral("m");
    default: return QStringLiteral("ft");
    }
}

static int verticalDistanceUnitFromString(const QJsonValue& value)
{
    if (value.toString() == u"m") { return 1; }
    return 0; // default: ft
}


// ————————————————————————————————————————————————
//  Waypoint construction helper
// ————————————————————————————————————————————————

// Build a Waypoint from backup-format fields using only the Waypoint
// public API.  No knowledge of the internal GeoJSON property keys is
// needed here.
static GeoMaps::Waypoint waypointFromBackup(double lon, double lat,
                                            double elevation,
                                            const QString& code,
                                            const QString& name,
                                            const QString& notes)
{
    QGeoCoordinate coord(lat, lon);
    if (!std::isnan(elevation))
    {
        coord.setAltitude(elevation);
    }

    // Use the public (coordinate, name) constructor.
    // When an ICAO code is present, it doubles as the display name.
    GeoMaps::Waypoint wp(coord, code.isEmpty() ? (name.isEmpty() ? QStringLiteral("Waypoint") : name) : code);

    if (!code.isEmpty())
    {
        wp.setICAOCode(code);
    }
    if (!notes.isEmpty())
    {
        wp.setNotes(notes);
    }

    return wp;
}


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

        QJsonObject obj;
        obj.insert(QStringLiteral("name"), aircraft.name());
        obj.insert(QStringLiteral("cabinPressureEqualsStaticPressure"), aircraft.cabinPressureEqualsStaticPressure());
        obj.insert(QStringLiteral("cruiseSpeed_mps"), aircraft.cruiseSpeed().toMPS());
        obj.insert(QStringLiteral("descentSpeed_mps"), aircraft.descentSpeed().toMPS());
        obj.insert(QStringLiteral("fuelConsumption_lph"), aircraft.fuelConsumption().toLPH());
        obj.insert(QStringLiteral("fuelConsumptionUnit"), fuelConsumptionUnitToString(aircraft.fuelConsumptionUnit()));
        obj.insert(QStringLiteral("horizontalDistanceUnit"), horizontalDistanceUnitToString(aircraft.horizontalDistanceUnit()));
        obj.insert(QStringLiteral("minimumSpeed_mps"), aircraft.minimumSpeed().toMPS());
        obj.insert(QStringLiteral("transponderCode"), aircraft.transponderCode());
        obj.insert(QStringLiteral("verticalDistanceUnit"), verticalDistanceUnitToString(aircraft.verticalDistanceUnit()));

        aircraftArray.append(obj);
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
        // Load the route via FlightRoute to get the parsed waypoints
        Navigation::FlightRoute route;
        auto errorMsg = route.load(fileInfo.absoluteFilePath());
        if (!errorMsg.isEmpty())
        {
            failed++;
            continue;
        }

        QJsonObject routeObj;
        routeObj.insert(QStringLiteral("name"), fileInfo.baseName());

        QJsonArray waypointsArray;
        for (const auto& waypoint : route.waypoints())
        {
            QJsonObject wpObj;

            auto code = waypoint.ICAOCode();
            if (!code.isEmpty())
            {
                wpObj.insert(QStringLiteral("code"), code);
            }
            else
            {
                auto name = waypoint.name();
                if (!name.isEmpty())
                {
                    wpObj.insert(QStringLiteral("name"), name);
                }
            }

            auto coord = waypoint.coordinate();
            if (coord.isValid())
            {
                wpObj.insert(QStringLiteral("longitude"), coord.longitude());
                wpObj.insert(QStringLiteral("latitude"), coord.latitude());
            }

            if (!std::isnan(coord.altitude()))
            {
                wpObj.insert(QStringLiteral("elevation"), coord.altitude());
            }

            waypointsArray.append(wpObj);
        }

        routeObj.insert(QStringLiteral("waypoints"), waypointsArray);
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
    QJsonArray waypointArray;

    auto waypointLibrary = GlobalObject::waypointLibrary();
    auto waypoints = waypointLibrary->waypoints();
    int failed = 0;

    for (const auto& waypoint : waypoints)
    {
        if (!waypoint.isValid())
        {
            failed++;
            continue;
        }

        QJsonObject wpObj;

        auto name = waypoint.name();
        if (!name.isEmpty())
        {
            wpObj.insert(QStringLiteral("name"), name);
        }

        auto notes = waypoint.notes();
        if (!notes.isEmpty())
        {
            wpObj.insert(QStringLiteral("notes"), notes);
        }

        auto coord = waypoint.coordinate();
        if (coord.isValid())
        {
            wpObj.insert(QStringLiteral("longitude"), coord.longitude());
            wpObj.insert(QStringLiteral("latitude"), coord.latitude());
        }

        if (!std::isnan(coord.altitude()))
        {
            wpObj.insert(QStringLiteral("elevation"), coord.altitude());
        }

        waypointArray.append(wpObj);
    }

    if (failed > 0)
    {
        errors += tr("%1 of %2 waypoints could not be exported.").arg(failed).arg(waypoints.count());
    }

    return waypointArray;
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

    // Import aircraft (skip section gracefully if missing)
    int aircraftFailed = 0;
    auto aircraftArray = rootObj.value(QStringLiteral("aircraft")).toArray();
    for (const auto& aircraftValue : aircraftArray)
    {
        if (!aircraftValue.isObject())
        {
            aircraftFailed++;
            continue;
        }

        auto aircraftObj = aircraftValue.toObject();

        // Build aircraft from backup fields using only the public API
        Navigation::Aircraft aircraft;
        aircraft.setName(aircraftObj.value(QStringLiteral("name")).toString());
        aircraft.setCabinPressureEqualsStaticPressure(aircraftObj.value(QStringLiteral("cabinPressureEqualsStaticPressure")).toBool(false));
        aircraft.setCruiseSpeed(Units::Speed::fromMPS(aircraftObj.value(QStringLiteral("cruiseSpeed_mps")).toDouble(NAN)));
        aircraft.setDescentSpeed(Units::Speed::fromMPS(aircraftObj.value(QStringLiteral("descentSpeed_mps")).toDouble(NAN)));
        aircraft.setFuelConsumption(Units::VolumeFlow::fromLPH(aircraftObj.value(QStringLiteral("fuelConsumption_lph")).toDouble(NAN)));
        aircraft.setFuelConsumptionUnit(static_cast<Navigation::Aircraft::FuelConsumptionUnit>(fuelConsumptionUnitFromString(aircraftObj.value(QStringLiteral("fuelConsumptionUnit")))));
        aircraft.setHorizontalDistanceUnit(static_cast<Navigation::Aircraft::HorizontalDistanceUnit>(horizontalDistanceUnitFromString(aircraftObj.value(QStringLiteral("horizontalDistanceUnit")))));
        aircraft.setMinimumSpeed(Units::Speed::fromMPS(aircraftObj.value(QStringLiteral("minimumSpeed_mps")).toDouble(NAN)));
        aircraft.setTransponderCode(aircraftObj.value(QStringLiteral("transponderCode")).toString());
        aircraft.setVerticalDistanceUnit(static_cast<Navigation::Aircraft::VerticalDistanceUnit>(verticalDistanceUnitFromString(aircraftObj.value(QStringLiteral("verticalDistanceUnit")))));

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
            for(int i=1; ; i++)
            {
                auto newName = u"%1 (%2)"_s.arg(aircraftName).arg(i);
                savePath = Librarian::fullPath(Librarian::Aircraft, newName);
                if (!QFile::exists(savePath))
                {
                    break;
                }
            }
        }

        auto errorMsg = aircraft.save(savePath);
        if (!errorMsg.isEmpty())
        {
            aircraftFailed++;
            continue;
        }
    }
    if (aircraftFailed > 0)
    {
        errors += tr("%1 of %2 aircraft could not be imported.").arg(aircraftFailed).arg(aircraftArray.count());
    }

    // Import routes (skip section gracefully if missing)
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

        Navigation::FlightRoute route;

        // Parse waypoints array
        auto waypointsArray = routeObj.value(QStringLiteral("waypoints")).toArray();
        if (waypointsArray.isEmpty())
        {
            routesFailed++;
            continue;
        }

        for (const auto& wpValue : waypointsArray)
        {
            if (!wpValue.isObject())
            {
                continue;
            }
            auto wpObj = wpValue.toObject();

            const double lon = wpObj.value(QStringLiteral("longitude")).toDouble(qQNaN());
            const double lat = wpObj.value(QStringLiteral("latitude")).toDouble(qQNaN());
            if (std::isnan(lon) || std::isnan(lat))
            {
                continue;
            }

            auto waypoint = waypointFromBackup(
                lon, lat,
                wpObj.value(QStringLiteral("elevation")).toDouble(qQNaN()),
                wpObj.value(QStringLiteral("code")).toString(),
                wpObj.value(QStringLiteral("name")).toString(),
                {});

            if (waypoint.isValid())
            {
                route.append(waypoint);
            }
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
            for(int i=1; ; i++)
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

    // Import waypoints (skip section gracefully if missing)
    int waypointsFailed = 0;
    auto waypointsArray = rootObj.value(QStringLiteral("waypoints")).toArray();
    if (!waypointsArray.isEmpty())
    {
        auto waypointLibrary = GlobalObject::waypointLibrary();
        for (const auto& waypointValue : waypointsArray)
        {
            if (!waypointValue.isObject())
            {
                waypointsFailed++;
                continue;
            }

            auto wpObj = waypointValue.toObject();

            const double lon = wpObj.value(QStringLiteral("longitude")).toDouble(qQNaN());
            const double lat = wpObj.value(QStringLiteral("latitude")).toDouble(qQNaN());
            if (std::isnan(lon) || std::isnan(lat))
            {
                waypointsFailed++;
                continue;
            }

            auto waypoint = waypointFromBackup(
                lon, lat,
                wpObj.value(QStringLiteral("elevation")).toDouble(qQNaN()),
                {},
                wpObj.value(QStringLiteral("name")).toString(),
                wpObj.value(QStringLiteral("notes")).toString());

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
