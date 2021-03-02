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

#include <QDataStream>
#include <QJsonArray>
#include <QVariant>

#include "AviationUnits.h"
#include "Waypoint.h"
#include "weather/Station.h"


Waypoint::Waypoint(QObject *parent)
    : QObject(parent)
{
    _properties.insert("CAT", QString("WP"));
    _properties.insert("NAM", QString("Waypoint"));
    _properties.insert("TYP", QString("WP"));
}


Waypoint::Waypoint(const Waypoint &other, QObject *parent)
    : QObject(parent),

      _coordinate(other._coordinate),
      _properties(other._properties)
{
    // Initialize connections
    setDownloadManager(other._downloadManager);
}


Waypoint::Waypoint(const QGeoCoordinate& coordinate, QObject *parent)
    : QObject(parent),
      _coordinate(coordinate)
{
    _properties.insert("CAT", QString("WP"));
    _properties.insert("NAM", QString("Waypoint"));
    _properties.insert("TYP", QString("WP"));
}


Waypoint::Waypoint(const QJsonObject &geoJSONObject, QObject *parent)
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
// METHODS
//

auto Waypoint::isNear(const Waypoint *other) const -> bool
{
    if (other == nullptr) {
        return false;
    }
    if (!_coordinate.isValid()) {
        return false;
    }
    if (!other->coordinate().isValid()) {
        return false;
    }

    return _coordinate.distanceTo(other->_coordinate) < 2000;
}


//
// PROPERTIES
//

void Waypoint::setCoordinate(const QGeoCoordinate& newCoordinate)
{
    if (newCoordinate == _coordinate) {
        return;
    }
    _coordinate = newCoordinate;
    emit coordinateChanged();
}


auto Waypoint::extendedName() const -> QString
{
    if (_properties.value("TYP").toString() == "NAV") {
        return QString("%1 (%2)").arg(_properties.value("NAM").toString(), _properties.value("CAT").toString());
    }

    return _properties.value("NAM").toString();
}


void Waypoint::setExtendedName(const QString &newExtendedName)
{
    if (newExtendedName == _properties.value("NAM").toString()) {
        return;
    }
    _properties.replace("NAM", newExtendedName);
    emit extendedNameChanged();
}


auto Waypoint::hasMETAR() const -> bool
{
    auto *station = weatherStation();
    if (station == nullptr) {
        return false;
    }
    return (station->metar() != nullptr);
}


auto Waypoint::hasTAF() const -> bool
{
    auto *station = weatherStation();
    if (station == nullptr) {
        return false;
    }
    return (station->taf() != nullptr);
}


auto Waypoint::icon() const -> QString
{
    auto CAT = getPropery(QStringLiteral("CAT")).toString();

    // We prefer SVG icons. There are, however, a few icons that cannot be
    // rendered by Qt's tinySVG renderer. We have generated PNGs for those
    // and treat them separately here.
    if ((CAT == "AD-GLD") || (CAT == "AD-GRASS") || (CAT == "AD-MIL-GRASS") || (CAT == "AD-UL")) {
        return QStringLiteral("/icons/waypoints/%1.png").arg(CAT);
    }

    return QStringLiteral("/icons/waypoints/%1.svg").arg(CAT);
}


void Waypoint::initializeWeatherStationConnections()
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


auto Waypoint::isValid() const -> bool
{
    if (!_coordinate.isValid()) {
        return false;
    }
    if (!_properties.contains("TYP")) {
        return false;
    }
    auto TYP = _properties.value("TYP").toString();

    // Handle airfields
    if (TYP == "AD") {
        // Property CAT
        if (!_properties.contains("CAT")) {
            return false;
        }
        auto CAT = _properties.value("CAT").toString();
        if ((CAT != "AD") && (CAT != "AD-GRASS") && (CAT != "AD-PAVED") &&
                (CAT != "AD-INOP") && (CAT != "AD-GLD") && (CAT != "AD-MIL") &&
                (CAT != "AD-MIL-GRASS") && (CAT != "AD-MIL-PAVED") && (CAT != "AD-UL") &&
                (CAT != "AD-WATER")) {
            return false;
        }

        // Property ELE
        if (!_properties.contains("ELE")) {
            return false;
        }
        bool ok = false;
        _properties.value("ELE").toInt(&ok);
        if (!ok) {
            return false;
        }

        // Property NAM
        if (!_properties.contains("NAM")) {
            return false;
        }
        return true;
    }

    // Handle NavAids
    if (TYP == "NAV") {
        // Property CAT
        if (!_properties.contains("CAT")) {
            return false;
        }
        auto CAT = _properties.value("CAT").toString();
        if ((CAT != "NDB") && (CAT != "VOR") && (CAT != "VOR-DME") &&
                (CAT != "VORTAC") && (CAT != "DVOR") && (CAT != "DVOR-DME") &&
                (CAT != "DVORTAC")) {
            return false;
        }

        // Property COD
        if (!_properties.contains("COD")) {
            return false;
        }

        // Property NAM
        if (!_properties.contains("NAM")) {
            return false;
        }

        // Property NAV
        if (!_properties.contains("NAV")) {
            return false;
        }

        // Property MOR
        if (!_properties.contains("MOR")) {
            return false;
        }

        return true;
    }

    // Handle waypoints
    if (TYP == "WP") {
        // Property CAT
        if (!_properties.contains("CAT")) {
            return false;
        }
        auto CAT = _properties.value("CAT").toString();
        if ((CAT != "MRP") && (CAT != "RP") && (CAT != "WP")) {
            return false;
        }

        // Property COD
        if ((CAT == "MRP") || (CAT == "RP")) {
            if (!_properties.contains("COD")) {
                return false;
            }
        }

        // Property NAM
        if (!_properties.contains("NAM")) {
            return false;
        }

        // Property SCO
        if ((CAT == "MRP") || (CAT == "RP")) {
            if (!_properties.contains("SCO")) {
                return false;
            }
        }

        return true;
    }

    // Unknown TYP
    return false;
}


auto Waypoint::operator==(const Waypoint &other) const -> bool {
    if (_coordinate != other._coordinate) {
        return false;
    }
    if (_properties != other._properties) {
        return false;
    }
    return true;
}


void Waypoint::setDownloadManager(Weather::DownloadManager *downloadManager)
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


auto Waypoint::tabularDescription() const -> QList<QString>
{
    QList<QString> result;

    if (_properties.value("TYP").toString() == "NAV") {
        result.append("ID  " + _properties.value("COD").toString() + " " + _properties.value("MOR").toString());
        result.append("NAV " + _properties.value("NAV").toString());
        if (_properties.contains("ELE")) {
            result.append(QString("ELEV%1 ft AMSL").arg(qRound(AviationUnits::Distance::fromM(_properties.value("ELE").toDouble()).toFeet())));
        }
    }

    if (_properties.value("TYP").toString() == "AD") {
        if (_properties.contains("COD")) {
            result.append("ID  " + _properties.value("COD").toString());
        }
        if (_properties.contains("INF")) {
            result.append("INF " + _properties.value("INF").toString().replace("\n", "<br>"));
        }
        if (_properties.contains("COM")) {
            result.append("COM " + _properties.value("COM").toString().replace("\n", "<br>"));
        }
        if (_properties.contains("NAV")) {
            result.append("NAV " + _properties.value("NAV").toString().replace("\n", "<br>"));
        }
        if (_properties.contains("OTH")) {
            result.append("OTH " + _properties.value("OTH").toString().replace("\n", "<br>"));
        }
        if (_properties.contains("RWY")) {
            result.append("RWY " + _properties.value("RWY").toString().replace("\n", "<br>"));
        }

        result.append( QString("ELEV%1 ft AMSL").arg(qRound(AviationUnits::Distance::fromM(_properties.value("ELE").toDouble()).toFeet())));
    }

    if (_properties.value("TYP").toString() == "WP") {
        if (_properties.contains("ICA")) {
            result.append("ID  " + _properties.value("COD").toString());
        }
        if (_properties.contains("COM")) {
            result.append("COM " + _properties.value("COM").toString());
        }
    }

    return result;
}


auto Waypoint::toJSON() const -> QJsonObject
{
    QJsonArray coords;
    coords.insert(0, _coordinate.longitude());
    coords.insert(1, _coordinate.latitude());
    QJsonObject geometry;
    geometry.insert("type", "Point");
    geometry.insert("coordinates", coords);
    QJsonObject feature;
    feature.insert("type", "Feature");
    feature.insert("properties", QJsonObject::fromVariantMap(_properties));
    feature.insert("geometry", geometry);

    return feature;
}


auto Waypoint::twoLineTitle() const -> QString
{
    QString codeName;
    if (_properties.contains("COD")) {
        codeName += _properties.value("COD").toString();
    }
    if (_properties.contains("MOR")) {
        codeName += " " + _properties.value("MOR").toString();
    }

    if (!codeName.isEmpty()) {
        return QString("<strong>%1</strong><br><font size='2'>%2</font>").arg(codeName, extendedName());
    }

    return extendedName();
}


auto Waypoint::wayTo(const QGeoCoordinate& fromCoordinate, bool useMetricUnits) const -> QString
{
    // Paranoid safety checks
    if (!fromCoordinate.isValid()) {
        return QString();
    }
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


auto Waypoint::weatherStation() const -> Weather::Station *
{
    if (_downloadManager.isNull()) {
        return nullptr;
    }
    if (!_properties.contains("COD")) {
        return nullptr;
    }

    return _downloadManager->findWeatherStation(_properties.value("COD").toString());
}

