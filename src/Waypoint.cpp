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
#include "SatNav.h"
#include "Waypoint.h"


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


QString Waypoint::richTextName() const
{
    if (_properties.value("TYP").toString() == "AD") {
        QStringList lines;

        // Line one: full text name, if available
        if (_properties.contains("NAM"))
            lines << _properties.value("NAM").toString();
#warning Need to find some other name if NAM does not exist!

        // line two: code name, codename and "way to" if available
        QStringList items4Line2;
        if (_properties.contains("COD"))
            items4Line2 << "<strong>"+_properties.value("COD").toString()+"</strong>";
        if (!_satNav.isNull() && (_satNav->status() == SatNav::OK) && _coordinate.isValid()) {
            bool useMetric = false;
            if (!_globalSettings.isNull())
                useMetric = _globalSettings->useMetricUnits();
            items4Line2 << _satNav->wayTo(_coordinate, useMetric);
        }
        if (!items4Line2.isEmpty())
            lines << items4Line2.join(" • ");

        // line three: METAR information, if available
        if (!_meteorologist.isNull() && _properties.contains("COD")) {
            auto descr = _meteorologist->briefDescription(_properties.value("COD").toString());
            if (!descr.isEmpty())
                lines << descr;
        }

        return lines.join("<br>");
    }


    QString codeName;
    if (_properties.contains("COD"))
        codeName += _properties.value("COD").toString();
    if (_properties.contains("MOR"))
        codeName += " " + _properties.value("MOR").toString();

    if (!codeName.isEmpty())
        return QString("<strong>%1</strong><br><font size='2'>%2</font>").arg(codeName, extendedName());

    return extendedName();
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

QString Waypoint::wayFrom(const QGeoCoordinate& position, bool useMetricUnits) const
{
    auto dist = AviationUnits::Distance::fromM(position.distanceTo(_coordinate));
    auto QUJ = qRound(position.azimuthTo(_coordinate));

    QString result;
    if (useMetricUnits) {
        result += QString("DIST %1 km • QUJ %2°").arg(dist.toKM(), 0, 'f', 1).arg(QUJ);
    } else {
        result += QString("DIST %1 NM • QUJ %2°").arg(dist.toNM(), 0, 'f', 1).arg(QUJ);
    }
    return result;
}

void Waypoint::setClock(Clock *clock)
{
    if (!_clock.isNull())
        disconnect(_clock, &Clock::timeChanged, this, &Waypoint::richTextNameChanged);

    _clock = clock;

    if (!_clock.isNull())
        connect(_clock, &Clock::timeChanged, this, &Waypoint::richTextNameChanged);
}

void Waypoint::setSatNav(SatNav *satNav)
{
    if (!_satNav.isNull()) {
        disconnect(_satNav, &SatNav::statusChanged, this, &Waypoint::richTextNameChanged);
        disconnect(_satNav, &SatNav::update, this, &Waypoint::richTextNameChanged);
    }

    _satNav = satNav;

    if (!_satNav.isNull()) {
        connect(_satNav, &SatNav::statusChanged, this, &Waypoint::richTextNameChanged);
        connect(_satNav, &SatNav::update, this, &Waypoint::richTextNameChanged);
    }
}

void Waypoint::setMeteorologist(Meteorologist *meteorologist)
{
    if (!_meteorologist.isNull())
        disconnect(_meteorologist, &Meteorologist::reportsChanged, this, &Waypoint::richTextNameChanged);

    _meteorologist = meteorologist;

    if (!_meteorologist.isNull())
        connect(_meteorologist, &Meteorologist::reportsChanged, this, &Waypoint::richTextNameChanged);
}

void Waypoint::setGlobalSettings(GlobalSettings *globalSettings)
{
    if (!_globalSettings.isNull())
        disconnect(_globalSettings, &GlobalSettings::useMetricUnitsChanged, this, &Waypoint::richTextNameChanged);

    _globalSettings = globalSettings;

    if (!_globalSettings.isNull())
        connect(_globalSettings, &GlobalSettings::useMetricUnitsChanged, this, &Waypoint::richTextNameChanged);
}
