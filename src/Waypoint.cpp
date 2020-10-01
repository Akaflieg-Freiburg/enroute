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

#include <QDataStream>
#include <QJsonArray>
#include <QVariant>

#include "AviationUnits.h"
#include "Clock.h"
#include "GlobalSettings.h"
#include "Meteorologist_WeatherStation.h"
#include "SatNav.h"
#include "Waypoint.h"

#warning Need to emit weatherStationChanged signals when appropriate
#warning Need to emit hasMETARChanged signals when appropriate
#warning Need to emit wayTo signals when appropriate
#warning Need to emit flightCategoryColorChanged signals when appropriate
#warning Need to emit METARSummaryChanged signals when appropriate
#warning Need to emit threeLineTitleChanged signals when appropriate


Waypoint::Waypoint(QObject *parent)
    : QObject(parent)
{
}


Waypoint::Waypoint(const Waypoint &other, QObject *parent)
    : QObject(parent), _coordinate(other._coordinate), _properties(other._properties)
{
}


Waypoint::Waypoint(const QGeoCoordinate& coordinate, QObject *parent)
    : QObject(parent), _coordinate(coordinate)
{
    _properties.insert("CAT", QString("WP"));
    _properties.insert("NAM", QString("Waypoint"));
    _properties.insert("TYP", QString("WP"));
}


Waypoint::Waypoint(const QGeoCoordinate& coordinate, QString code, QObject *parent)
    : QObject(parent), _coordinate(coordinate)
{
    _properties.insert("CAT", QString("WP"));
    if (code.isEmpty())
        _properties.insert("NAM", QString("Waypoint"));
    else
        _properties.insert("COD", code);
    _properties.insert("TYP", QString("WP"));
}

Waypoint::Waypoint(const QJsonObject &geoJSONObject, QObject *parent)
    : QObject(parent)
{
    // Paranoid safety checks
    if (geoJSONObject["type"] != "Feature")
        return;

    // Get properties
    if (!geoJSONObject.contains("properties"))
        return;
    auto properties = geoJSONObject["properties"].toObject();
    foreach(auto propertyName, properties.keys())
        _properties.insert(propertyName, properties[propertyName].toVariant());

    // Get geometry
    if (!geoJSONObject.contains("geometry"))
        return;
    auto geometry = geoJSONObject["geometry"].toObject();
    if (geometry["type"] != "Point")
        return;
    if (!geometry.contains("coordinates"))
        return;
    auto coordinateArray = geometry["coordinates"].toArray();
    if (coordinateArray.size() != 2)
        return;
    _coordinate = QGeoCoordinate(coordinateArray[1].toDouble(), coordinateArray[0].toDouble() );
    if (_properties.contains("ELE"))
        _coordinate.setAltitude(properties["ELE"].toDouble());
}

QString Waypoint::extendedName() const
{
    if (_properties.value("TYP").toString() == "NAV")
        return QString("%1 (%2)").arg(_properties.value("NAM").toString(), _properties.value("CAT").toString());

    return _properties.value("NAM").toString();
}

QString Waypoint::fourLineTitle() const
{
    QStringList lines;

    QString codeName;
    if (_properties.contains("COD"))
        codeName += _properties.value("COD").toString();
    if (_properties.contains("MOR"))
        codeName += " " + _properties.value("MOR").toString();

    if (codeName.isEmpty())
        lines << extendedName();
    else {
        lines << QString("<strong>%1</strong>").arg(codeName);
        if (!extendedName().isEmpty())
            lines << QString("<font size='2'>%1</font>").arg(extendedName());
    }

    // line two: code name, codename and "way to" if available
    if (!_satNav.isNull() && (_satNav->status() == SatNav::OK) && _coordinate.isValid()) {
        bool useMetric = false;
        if (!_globalSettings.isNull())
            useMetric = _globalSettings->useMetricUnits();
        lines << QString("<font size='2'>%1</font>").arg(_satNav->wayTo(_coordinate, useMetric));
    }

    // line three: METAR information, if available
    if (!_meteorologist.isNull() && _properties.contains("COD")) {
        auto station = _meteorologist->findWeatherStation(_properties.value("COD").toString());
        if (station) {
            auto metar = station->metar();
            if (metar) {
                auto descr = metar->summary();
                if (!descr.isEmpty())
                    lines << QString("<font size='2'>%1</font>").arg(descr);
            }
        }
    }

    return lines.join("<br>");
}

QString Waypoint::twoLineTitle() const
{
    QString codeName;
    if (_properties.contains("COD"))
        codeName += _properties.value("COD").toString();
    if (_properties.contains("MOR"))
        codeName += " " + _properties.value("MOR").toString();

    if (!codeName.isEmpty())
        return QString("<strong>%1</strong><br><font size='2'>%2</font>").arg(codeName, extendedName());

    return extendedName();
}

Meteorologist::WeatherStation *Waypoint::weatherStation() const
{
    if (_meteorologist.isNull())
        return nullptr;
    if (!_properties.contains("COD"))
        return nullptr;

    return _meteorologist->findWeatherStation(_properties.value("COD").toString());
}

QList<QString> Waypoint::tabularDescription() const
{
    QList<QString> result;

    if (_properties.value("TYP").toString() == "NAV") {
        result.append("ID  " + _properties.value("COD").toString() + " " + _properties.value("MOR").toString());
        result.append("NAV " + _properties.value("NAV").toString());
        if (_properties.contains("ELE"))
            result.append(QString("ELEV%1 ft AMSL").arg(qRound(AviationUnits::Distance::fromM(_properties.value("ELE").toDouble()).toFeet())));
    }

    if (_properties.value("TYP").toString() == "AD") {
        if (_properties.contains("COD"))
            result.append("ID  " + _properties.value("COD").toString());
        if (_properties.contains("INF"))
            result.append("INF " + _properties.value("INF").toString().replace("\n", "<br>"));
        if (_properties.contains("COM"))
            result.append("COM " + _properties.value("COM").toString().replace("\n", "<br>"));
        if (_properties.contains("NAV"))
            result.append("NAV " + _properties.value("NAV").toString().replace("\n", "<br>"));
        if (_properties.contains("OTH"))
            result.append("OTH " + _properties.value("OTH").toString().replace("\n", "<br>"));
        if (_properties.contains("RWY"))
            result.append("RWY " + _properties.value("RWY").toString().replace("\n", "<br>"));

        result.append( QString("ELEV%1 ft AMSL").arg(qRound(AviationUnits::Distance::fromM(_properties.value("ELE").toDouble()).toFeet())));
    }

    if (_properties.value("TYP").toString() == "WP") {
        if (_properties.contains("ICA"))
            result.append("ID  " + _properties.value("COD").toString());
        if (_properties.contains("COM"))
            result.append("COM " + _properties.value("COM").toString());
    }

    return result;
}

QJsonObject Waypoint::toJSON() const
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

QString Waypoint::wayTo() const
{
    if (_satNav.isNull())
        return QString();
    if (_satNav->status() != SatNav::OK)
        return QString();

    bool useMetricUnits = false;
    if (_globalSettings)
        useMetricUnits = _globalSettings->useMetricUnits();

    auto position = _satNav->lastValidCoordinate();
    auto dist = AviationUnits::Distance::fromM(position.distanceTo(_coordinate));
    auto QUJ = qRound(position.azimuthTo(_coordinate));

    if (useMetricUnits)
        return QString("DIST %1 km • QUJ %2°").arg(dist.toKM(), 0, 'f', 1).arg(QUJ);
    return QString("DIST %1 NM • QUJ %2°").arg(dist.toNM(), 0, 'f', 1).arg(QUJ);
}

void Waypoint::setSatNav(SatNav *satNav)
{
    if (!_satNav.isNull()) {
        disconnect(_satNav, &SatNav::statusChanged, this, &Waypoint::fourLineTitle);
        disconnect(_satNav, &SatNav::update, this, &Waypoint::fourLineTitle);
    }

    _satNav = satNav;

    if (!_satNav.isNull()) {
        connect(_satNav, &SatNav::statusChanged, this, &Waypoint::fourLineTitle);
        connect(_satNav, &SatNav::update, this, &Waypoint::fourLineTitle);
    }
}

void Waypoint::setMeteorologist(Meteorologist *meteorologist)
{
    if (!_meteorologist.isNull()) {
        disconnect(_meteorologist, &Meteorologist::weatherStationsChanged, this, &Waypoint::flightCategoryColorChanged);
        disconnect(_meteorologist, &Meteorologist::weatherStationsChanged, this, &Waypoint::fourLineTitle);
        disconnect(_meteorologist, &Meteorologist::weatherStationsChanged, this, &Waypoint::weatherStationChanged);
    }

    _meteorologist = meteorologist;

    if (!_meteorologist.isNull()) {
        connect(_meteorologist, &Meteorologist::weatherStationsChanged, this, &Waypoint::flightCategoryColorChanged);
        connect(_meteorologist, &Meteorologist::weatherStationsChanged, this, &Waypoint::fourLineTitle);
        connect(_meteorologist, &Meteorologist::weatherStationsChanged, this, &Waypoint::weatherStationChanged);
    }
}

void Waypoint::setGlobalSettings(GlobalSettings *globalSettings)
{
    if (!_globalSettings.isNull())
        disconnect(_globalSettings, &GlobalSettings::useMetricUnitsChanged, this, &Waypoint::fourLineTitle);

    _globalSettings = globalSettings;

    if (!_globalSettings.isNull())
        connect(_globalSettings, &GlobalSettings::useMetricUnitsChanged, this, &Waypoint::fourLineTitle);
}

QString Waypoint::flightCategoryColor() const
{
    auto station = weatherStation();
    if (station == nullptr)
        return "transparent";
    auto metar = station->metar();
    if (metar == nullptr)
        return "transparent";
    return metar->flightCategoryColor();
}

QString Waypoint::METARSummary() const
{
    auto station = weatherStation();
    if (station == nullptr)
        return QString();
    auto metar = station->metar();
    if (metar == nullptr)
        return QString();
    return metar->summary();
}


bool Waypoint::isValid() const
{
#warning need to check if properties satisfy GeoJSON standard.
    return _coordinate.isValid();
}

bool Waypoint::operator==(const Waypoint &other) const {
#warning need to ensure that all properties agree
    return _coordinate == other._coordinate;
}

bool Waypoint::hasMETAR() const
{
    auto station = weatherStation();
    if (station == nullptr)
        return false;
    return (station->metar() != nullptr);
}

bool Waypoint::hasTAF() const
{
    auto station = weatherStation();
    if (station == nullptr)
        return false;
    return (station->taf() != nullptr);
}
