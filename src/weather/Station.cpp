/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

#include "geomaps/GeoMapProvider.h"
#include "weather/Station.h"

#include <utility>


Weather::Station::Station(QObject *parent)
    : QObject(parent)
{
}


Weather::Station::Station(QString id, GeoMaps::GeoMapProvider *geoMapProvider, QObject *parent)
    : QObject(parent),
      _ICAOCode(std::move(id)),
      _geoMapProvider(geoMapProvider)
{
    _extendedName = _ICAOCode;
    _twoLineTitle = _ICAOCode;

    // Wire up with GeoMapProvider, in order to learn about future changes in waypoints
    connect(_geoMapProvider, &GeoMaps::GeoMapProvider::geoJSONChanged, this, &Weather::Station::readDataFromWaypoint);
    readDataFromWaypoint();
}


void Weather::Station::readDataFromWaypoint()
{
    // Paranoid safety checks
    if (_geoMapProvider.isNull()) {
        return;
}
    // Immediately quit if we already have the necessary data
    if (hasWaypointData) {
        return;
}

    auto *waypoint = _geoMapProvider->findByID(_ICAOCode);
    if (waypoint == nullptr) {
        return;
}
    hasWaypointData = true;

    // Update data
    auto cacheCoordiante = _coordinate;
    _coordinate = waypoint->coordinate();
    if (_coordinate != cacheCoordiante) {
        emit coordinateChanged();
}

    auto cacheExtendedName = _extendedName;
    _extendedName = waypoint->extendedName();
    if (_extendedName != cacheExtendedName) {
        emit extendedNameChanged();
}

    auto cacheIcon = _icon;
    _icon = waypoint->icon();
    if (_icon != cacheIcon) {
        emit iconChanged();
}

    auto cacheTwoLineTitle = _twoLineTitle;
    _twoLineTitle = waypoint->twoLineTitle();
    if (_twoLineTitle != cacheTwoLineTitle) {
        emit twoLineTitleChanged();
}

    disconnect(_geoMapProvider, nullptr, this, nullptr);
}


void Weather::Station::setMETAR(Weather::METAR *metar)
{
    // Ignore invalid and expired METARs. Also ignore METARs whose ICAO code does not match with this weather station
    if (metar != nullptr) {
        if (!metar->isValid() || metar->isExpired() || (metar->ICAOCode() != _ICAOCode)) {
            metar->deleteLater();
            return;
        }
}

    // If METAR did not change, then do nothing
    if (metar == _metar) {
        return;
}

    // Cache values
    auto cacheHasMETAR = hasMETAR();

    // Take ownership. This will guarantee that the METAR gets deleted along with this weather station.
    if (metar != nullptr) {
        metar->setParent(this);
}

    // Clear and delete old METAR, if one exists.
    if (!_metar.isNull()) {
        disconnect(_metar, nullptr, this, nullptr);
        _metar->deleteLater();
    }

    // Overwrite metar pointer
    _metar = metar;

    if (_metar != nullptr) {
        // Connect new METAR, update the coordinate if necessary.
        connect(_metar, &QObject::destroyed, this, &Weather::Station::metarChanged);

        // Update coordinate
        if (!_coordinate.isValid()) {
            _coordinate = _metar->coordinate();
}
    }

    // Let the world know that the metar changed
    if (cacheHasMETAR != hasMETAR()) {
        emit hasMETARChanged();
}
    emit metarChanged();
}


void Weather::Station::setTAF(Weather::TAF *taf)
{
    // Ignore invalid and expired TAFs. Also ignore TAFs whose ICAO code does not match with this weather station
    if (taf != nullptr) {
        if (!taf->isValid() || taf->isExpired() || (taf->ICAOCode() != _ICAOCode)) {
            taf->deleteLater();
            return;
        }
}

    // If TAF did not change, then do nothing
    if (taf == _taf) {
        return;
}

    // Cache values
    auto cacheHasTAF = hasTAF();

    // Take ownership. This will guarantee that the METAR gets deleted along with this weather station.
    if (taf != nullptr) {
        taf->setParent(this);
}

    // Clear and delete old TAF, if one exists.
    if (!_taf.isNull()) {
        disconnect(_taf, nullptr, this, nullptr);
        _taf->deleteLater();
    }

    // Overwrite TAF pointer
    _taf = taf;

    if (_taf != nullptr) {
        // Connect new TAF, update the coordinate if necessary.
        connect(_taf, &QObject::destroyed, this, &Weather::Station::tafChanged);

        // Update coordinate
        if (!_coordinate.isValid()) {
            _coordinate = _taf->coordinate();
}
    }

    // Let the world know that the taf changed
    if (cacheHasTAF != hasTAF()) {
        emit hasTAFChanged();
}
    emit tafChanged();
}


auto Weather::Station::wayTo(const QGeoCoordinate& fromCoordinate, bool useMetricUnits) const -> QString
{
    // Paranoid safety checks
    if (!fromCoordinate.isValid()) {
        return QString();
}
    auto _coordinate = coordinate();
    if (!_coordinate.isValid()) {
        return QString();
}

    auto dist = AviationUnits::Distance::fromM(fromCoordinate.distanceTo(_coordinate));
    auto QUJ = qRound(fromCoordinate.azimuthTo(_coordinate));

    if (useMetricUnits) {
        return QString("DIST %1 km • QUJ %2°").arg(dist.toKM(), 0, 'f', 1).arg(QUJ);
}
    return QString("DIST %1 NM • QUJ %2°").arg(dist.toNM(), 0, 'f', 1).arg(QUJ);
}
