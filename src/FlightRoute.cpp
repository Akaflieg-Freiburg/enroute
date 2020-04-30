/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QQmlEngine>
#include <QStandardPaths>

#include "AviationUnits.h"
#include "FlightRoute.h"
#include "Waypoint.h"


FlightRoute::FlightRoute(Aircraft *aircraft, Wind *wind, QObject *parent)
    : QObject(parent), _aircraft(aircraft), _wind(wind)
{
    stdFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flight route.geojson";

    // Load last flightRoute, restore suggested name
    auto error = load(stdFileName);
    if (error.isEmpty())
        _suggestedFileName = settings.value("flightRouteSuggestedName", QString()).toString();

    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::saveToStdLocation);
    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::summaryChanged);
    if (!_aircraft.isNull())
        connect(_aircraft, &Aircraft::valChanged, this, &FlightRoute::summaryChanged);
    if (!_wind.isNull())
        connect(_wind, &Wind::valChanged, this, &FlightRoute::summaryChanged);
}


void FlightRoute::append(QObject *waypoint)
{
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;

    auto* wp = dynamic_cast<Waypoint*>(waypoint);
    _waypoints.append(new Waypoint(*wp, this));

    updateLegs();
    emit waypointsChanged();
}


bool FlightRoute::fileExists(const QString& fileName) const
{
    return QFile::exists( libraryPath(fileName) );
}


QObject* FlightRoute::firstWaypointObject() const
{
    if (_waypoints.isEmpty())
        return nullptr;
    return _waypoints.first();
}


QVariantList FlightRoute::geoPath() const
{
    // Paranoid safety checks
    if (_waypoints.size() < 2)
        return QVariantList();

    QVariantList result;
    for(auto _waypoint : _waypoints) {
        if (!_waypoint->isValid())
            return QVariantList();
        result.append(QVariant::fromValue(_waypoint->coordinate()));
    }

    return result;
}


QObject* FlightRoute::lastWaypointObject() const
{
    if (_waypoints.isEmpty())
        return nullptr;
    return _waypoints.last();
}


void FlightRoute::clear()
{
    qDeleteAll(_waypoints);
    _waypoints.clear();
    _suggestedFileName = QString();

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::moveDown(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;
    if (waypoint == lastWaypointObject())
        return;

    auto swp = dynamic_cast<Waypoint*>(waypoint);

    auto idx = _waypoints.indexOf(swp);
    _waypoints.move(idx, idx+1);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::moveUp(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;
    if (waypoint == firstWaypointObject())
        return;

    auto swp = dynamic_cast<Waypoint*>(waypoint);

    auto idx = _waypoints.indexOf(swp);
    if (idx > 0)
        _waypoints.move(idx, idx-1);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::removeWaypoint(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;

    auto swp = dynamic_cast<Waypoint*>(waypoint);

    _waypoints.removeOne(swp);
    delete swp;

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::reverse()
{
    std::reverse(_waypoints.begin(), _waypoints.end());
    updateLegs();
    emit waypointsChanged();
}


QString FlightRoute::saveToLibrary(const QString &fileName)
{
    QDir dir;
    auto success = dir.mkpath(FlightRoute::libraryDir());
    if (!success)
        return tr("Unable to create directory '%1' for library.").arg(libraryDir());

    auto fullName = libraryPath(fileName);
    auto error = save(fullName);
    if (error.isEmpty()) {
        _suggestedFileName = fileName;
        settings.setValue("flightRouteSuggestedName", _suggestedFileName);
    } else
        settings.setValue("flightRouteSuggestedName", QString());
    return error;
}


QString FlightRoute::save(QString fileName) const
{
    QFile file(fileName);
    auto success = file.open(QIODevice::WriteOnly);
    if (!success)
        return tr("Unable to open the file '%1' for writing.").arg(fileName);
    auto numBytesWritten = file.write(toGeoJSON().toJson());
    if (numBytesWritten == -1) {
        file.close();
        QFile::remove(fileName);
        return tr("Unable to write to file '%1' for writing.").arg(fileName);
    }
    file.close();
    return QString();
}


void FlightRoute::updateLegs()
{
    foreach(auto _leg, _legs)
        _leg->deleteLater();
    _legs.clear();

    for(int i=0; i<_waypoints.size()-1; i++)
        _legs.append(new Leg(_waypoints[i], _waypoints[i+1], _aircraft, _wind, this));
}


QList<QObject*> FlightRoute::routeObjects() const
{
    QList<QObject*> result;

    if (_waypoints.isEmpty())
        return result;

    for(int i=0; i<_waypoints.size()-1; i++) {
        result.append(_waypoints[i]);
        result.append(_legs[i]);
    }
    result.append(_waypoints.last());

    return result;
}


QString FlightRoute::suggestedFilename() const
{
    if (_waypoints.size() < 2)
        return QString();

    if (!_suggestedFileName.isEmpty())
        return _suggestedFileName;

    QString start = _waypoints.constFirst()->get("COD").toString();
    if (start.isEmpty())
        return QString();
    QString end = _waypoints.constLast()->get("COD").toString();
    if (end.isEmpty())
        return QString();

    return start+" → "+end;
}


QString FlightRoute::summary() const
{
    if (_legs.empty())
        return {};

    QString result;

    auto dist = AviationUnits::Distance::fromM(0.0);
    auto time = AviationUnits::Time::fromS(0.0);
    double fuelInL = 0.0;

    for(auto _leg : _legs) {
        dist += _leg->distance();
        if (dist.toM() > 100) {
            time += _leg->Time();
            fuelInL += _leg->Fuel();
        }
    }

    result += QString("Total: %1&nbsp;NM").arg(dist.toNM(), 0, 'f', 1);
    if (time.isFinite())
        result += QString(" • %1&nbsp;h").arg(time.toHoursAndMinutes());
    if (qIsFinite(fuelInL))
        result += QString(" • %1&nbsp;L").arg(qRound(fuelInL));


    QStringList complaints;
    if (!_aircraft.isNull()) {
        if (!qIsFinite(_aircraft->cruiseSpeedInKT()))
            complaints += tr("Cruise speed not specified.");
        if (!qIsFinite(_aircraft->fuelConsumptionInLPH()))
            complaints += tr("Fuel consumption not specified.");
    }
    if (!_wind.isNull()) {
        if (!qIsFinite(_wind->windSpeedInKT()))
            complaints += tr("Wind speed not specified.");
        if (!qIsFinite(_wind->windDirectionInDEG()))
            if (!qIsFinite(_wind->windDirectionInDEG()))
                complaints += tr("Wind direction not specified.");

    }

    if (!complaints.isEmpty())
        result += QString("<p><font color='red'>Computation incomplete. %1</font></p>").arg(complaints.join(" "));

    return result;
}


QString FlightRoute::libraryDir() const
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation)+"/enroute flight navigation/flight routes";
}


QString FlightRoute::libraryPath(const QString &fileName) const
{
    return libraryDir()+"/"+fileName+".geojson";
}


QJsonDocument FlightRoute::toGeoJSON() const
{
    QJsonArray waypointArray;
    foreach(auto waypoint, _waypoints)
        waypointArray.append(waypoint->toJSON());
    QJsonObject jsonObj;
    jsonObj.insert("type", "FeatureCollection");
    jsonObj.insert("features", waypointArray);
    QJsonDocument doc;
    doc.setObject(jsonObj);
    return doc;
}


QString FlightRoute::loadFromLibrary(const QString &fileName)
{
    auto error = load(libraryPath(fileName));
    if (error.isEmpty())
        _suggestedFileName = fileName;
    return error;
}


QString FlightRoute::load(QString fileName)
{
    if (fileName.isEmpty())
        fileName = stdFileName;

    QFile file(fileName);
    auto success = file.open(QIODevice::ReadOnly);
    if (!success)
        return tr("Cannot open file '%1' for reading.").arg(fileName);
    auto fileContent = file.readAll();
    if (fileContent.isEmpty())
        return tr("Cannot read data from file '%1'.").arg(fileName);
    file.close();

    QJsonParseError parseError;
    auto document = QJsonDocument::fromJson(fileContent, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return tr("Cannot parse file '%1'. Reason: %2.").arg(fileName, parseError.errorString());

    QList<QPointer<Waypoint>> newWaypoints;
    foreach(auto value, document.object()["features"].toArray()) {
        auto wp = new Waypoint(value.toObject());
        if (!wp->isValid()) {
            qDeleteAll(newWaypoints);
            return tr("Cannot parse content of file '%1'.").arg(libraryPath(fileName));
        }
        QQmlEngine::setObjectOwnership(wp, QQmlEngine::CppOwnership);
        newWaypoints.append(wp);
    }

    qDeleteAll(_waypoints);
    _waypoints = newWaypoints;
    updateLegs();
    emit waypointsChanged();

    return QString();
}
