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

#include <QJsonArray>

#include "Waypoint.h"
#include "weather/Station.h"


GeoMaps::Waypoint::Waypoint(QObject *parent)
    : QObject(parent)
{
    _properties.insert("CAT", QString("WP"));
    _properties.insert("NAM", QString("Waypoint"));
    _properties.insert("TYP", QString("WP"));
}


GeoMaps::Waypoint::Waypoint(const Waypoint &other, QObject *parent)
    : QObject(parent)
{
    _coordinate = other._coordinate;
    _properties = other._properties;

    // Initialize connections
    setDownloadManager(other._downloadManager);
}


GeoMaps::Waypoint::Waypoint(const SimpleWaypoint &other, QObject *parent)
    : QObject(parent)
{
    _coordinate = other._coordinate;
    _properties = other._properties;

}

void GeoMaps::Waypoint::copyFrom(const SimpleWaypoint &other)
{
    _coordinate = other._coordinate;
    _properties = other._properties;

#warning
    emit extendedNameChanged();
    emit hasMETARChanged();
    emit hasTAFChanged();
    emit weatherStationChanged();
}


GeoMaps::Waypoint::Waypoint(const QGeoCoordinate& coordinate, QObject *parent)
    : QObject(parent)
{
    _coordinate = coordinate;
    _properties.insert("CAT", QString("WP"));
    _properties.insert("NAM", QString("Waypoint"));
    _properties.insert("TYP", QString("WP"));
}


GeoMaps::Waypoint::Waypoint(const QJsonObject &geoJSONObject, QObject *parent)
    : QObject(parent)
{
    // Paranoid safety checks
    if (geoJSONObject["type"] != "Feature") {
        return;
    }

    // Get properties
    if (!geoJSONObject.contains("properties")) {
        return;
    }
    auto properties = geoJSONObject["properties"].toObject();
    foreach(auto propertyName, properties.keys())
        _properties.insert(propertyName, properties[propertyName].toVariant());

    // Get geometry
    if (!geoJSONObject.contains("geometry")) {
        return;
    }
    auto geometry = geoJSONObject["geometry"].toObject();
    if (geometry["type"] != "Point") {
        return;
    }
    if (!geometry.contains("coordinates")) {
        return;
    }
    auto coordinateArray = geometry["coordinates"].toArray();
    if (coordinateArray.size() != 2) {
        return;
    }
    _coordinate = QGeoCoordinate(coordinateArray[1].toDouble(), coordinateArray[0].toDouble() );
    if (_properties.contains("ELE")) {
        _coordinate.setAltitude(properties["ELE"].toDouble());
    }
}



//
// PROPERTIES
//


void GeoMaps::Waypoint::setExtendedName(const QString &newExtendedName)
{
    if (newExtendedName == _properties.value("NAM").toString()) {
        return;
    }
    _properties.replace("NAM", newExtendedName);
    emit extendedNameChanged();
}


auto GeoMaps::Waypoint::hasMETAR() const -> bool
{
    auto *station = weatherStation();
    if (station == nullptr) {
        return false;
    }
    return (station->metar() != nullptr);
}


auto GeoMaps::Waypoint::hasTAF() const -> bool
{
    auto *station = weatherStation();
    if (station == nullptr) {
        return false;
    }
    return (station->taf() != nullptr);
}


void GeoMaps::Waypoint::initializeWeatherStationConnections()
{
    // Get new weather station
    auto *newStationPtr = weatherStation();
    if (newStationPtr == nullptr) {
        return;
    }

    // Do nothing if newStation is not really new
    if (newStationPtr == _weatherStation_unguarded) {
        return;
    }

    //
    // Delete old connections, etc
    //
    if (_weatherStation_guarded != nullptr) {
        disconnect(_weatherStation_guarded, nullptr, this, nullptr);
        auto *oldMETAR = _weatherStation_guarded->metar();
        if (oldMETAR != nullptr) {
            disconnect(oldMETAR, nullptr, this, nullptr);
        }
    }

    // Set new stations
    _weatherStation_guarded = newStationPtr;
    _weatherStation_unguarded = newStationPtr;

    // Setup new connections
    if (newStationPtr != nullptr) {
        connect(newStationPtr, &Weather::Station::metarChanged, this, &Waypoint::hasMETARChanged);
        connect(newStationPtr, &Weather::Station::tafChanged, this, &Waypoint::hasTAFChanged);
        connect(newStationPtr, &Weather::Station::destroyed, this, &Waypoint::initializeWeatherStationConnections);
    }

    // Emit all relevant changes
    emit hasMETARChanged();
    emit hasTAFChanged();
    emit weatherStationChanged();
}


auto GeoMaps::Waypoint::operator==(const Waypoint &other) const -> bool {
    if (_coordinate != other._coordinate) {
        return false;
    }
    if (_properties != other._properties) {
        return false;
    }
    return true;
}


void GeoMaps::Waypoint::setDownloadManager(Weather::DownloadManager *downloadManager)
{
    // No DownloadManager? Then nothing to do.
    if (downloadManager == nullptr) {
        return;
    }
    // No new DownloadManager? Then nothing to do.
    if (downloadManager == _downloadManager) {
        return;
    }

    // Set downloadManager
    _downloadManager = downloadManager;

    // If the waypoint does not have a four-letter ICAO code, there is certainly no weather station.
    // In that case, return and do not do anything.
    if (!_properties.contains("COD")) {
        return;
    }
    if (_properties.value("COD").toString().length() != 4) {
        return;
    }

    // Wire up the _downloadManager
    connect(_downloadManager, &Weather::DownloadManager::weatherStationsChanged, this, &Waypoint::initializeWeatherStationConnections);
    initializeWeatherStationConnections();
}


auto GeoMaps::Waypoint::weatherStation() const -> Weather::Station *
{
    if (_downloadManager.isNull()) {
        return nullptr;
    }
    if (!_properties.contains("COD")) {
        return nullptr;
    }

    return _downloadManager->findWeatherStation(_properties.value("COD").toString());
}
