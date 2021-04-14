/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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


#include "GlobalSettings.h"
#include "traffic/TrafficFactor.h"


Traffic::TrafficFactor::TrafficFactor(QObject *parent) : QObject(parent)
{  
    timeoutCounter.setSingleShot(true);

    // Compute derived properties and set bindings
    connect(this, &Traffic::TrafficFactor::alarmLevelChanged, this, &Traffic::TrafficFactor::setColor);
    setColor();

    connect(this, &Traffic::TrafficFactor::coordinateChanged, this, &Traffic::TrafficFactor::setDescription);
    connect(this, &Traffic::TrafficFactor::typeChanged, this, &Traffic::TrafficFactor::setDescription);
    connect(this, &Traffic::TrafficFactor::vDistChanged, this, &Traffic::TrafficFactor::setDescription);
    setDescription();

    connect(this, &Traffic::TrafficFactor::colorChanged, this, &Traffic::TrafficFactor::setIcon);
    connect(this, &Traffic::TrafficFactor::positionInfoChanged, this, &Traffic::TrafficFactor::setIcon);
    setIcon();

    connect(this, &Traffic::TrafficFactor::positionInfoChanged, this, &Traffic::TrafficFactor::setValid);
    connect(&timeoutCounter, &QTimer::timeout, this, &Traffic::TrafficFactor::setValid);
    setValid();
}


auto Traffic::TrafficFactor::hasHigherPriorityThan(const TrafficFactor &rhs) const -> bool
{
    // Criterion 1: Valid instances have higher priority than invalid ones
    if (!rhs.valid()) {
        return true;
    }
    if (!valid()) {
        return false;
    }
    // At this point, both instances are valid.

    // Criterion 2: Alarm level
    if (_alarmLevel > rhs._alarmLevel) {
        return true;
    }
    if (_alarmLevel < rhs._alarmLevel) {
        return false;
    }
    // At this point, both instances have equal alarm levels

    // Final criterion: distance to current position
    return (_hDist < rhs._hDist);
}


void Traffic::TrafficFactor::setAnimate(bool a)
{
    if (a == _animate) {
        return;
    }

    _animate = a;
    emit animateChanged();
}


void Traffic::TrafficFactor::setColor()
{
    QString newColor = QStringLiteral("red");
    if (_alarmLevel == 0) {
        newColor = QStringLiteral("green");
    }
    if (_alarmLevel == 1) {
        newColor = QStringLiteral("yellow");
    }

    // Set value and emit signal, if appropriate
    if (_color == newColor) {
        return;
    }
    _color = newColor;
    emit colorChanged();
}


void Traffic::TrafficFactor::setData(int newAlarmLevel, const QString& newID, AviationUnits::Distance newHDist, AviationUnits::Distance newVDist, AircraftType newType, const QGeoPositionInfo& newPositionInfo)
{
    // Set properties
    bool hasAlarmLevelChanged = (_alarmLevel != newAlarmLevel);
    _alarmLevel = newAlarmLevel;

    bool hasIDChanged = (_ID != newID);
    _ID = newID;

    bool hasCoordinateChanged = (coordinate() != newPositionInfo.coordinate());
    bool hasTTChanged = false;
    if (_positionInfo.hasAttribute(QGeoPositionInfo::Direction) != newPositionInfo.hasAttribute(QGeoPositionInfo::Direction)) {
        hasTTChanged = true;
    } else {
        if (_positionInfo.attribute(QGeoPositionInfo::Direction) != newPositionInfo.attribute(QGeoPositionInfo::Direction)) {
            hasTTChanged = true;
        }
    }
    bool _positionInfoChanged = (_positionInfo != newPositionInfo);
    _positionInfo = newPositionInfo;

    bool hasTypeChanged = (_type != newType);
    _type = newType;

    bool hasVDistChanged = (_vDist != newVDist);
    _vDist = newVDist;

    bool hasHDistChanged = (_hDist != newHDist);
    _hDist = newHDist;

    bool hasClimbRateChanged = (_positionInfo.attribute(QGeoPositionInfo::VerticalSpeed) != newPositionInfo.attribute(QGeoPositionInfo::VerticalSpeed));

    bool hasGroundSpeedChanged = (_positionInfo.attribute(QGeoPositionInfo::GroundSpeed) != newPositionInfo.attribute(QGeoPositionInfo::GroundSpeed));

    // If the ID changed, do not animate property changes in the GUI.
    if (hasIDChanged) {
       setAnimate(false);
    }

    // Emit notifier signals as appropriate
    if (hasAlarmLevelChanged) {
        emit alarmLevelChanged();
    }
    if (hasCoordinateChanged) {
        emit coordinateChanged();
    }
    if (hasHDistChanged) {
        emit hDistChanged();
    }
    if (hasIDChanged) {
        emit IDChanged();
       setAnimate(false);
    }
    if (_positionInfoChanged)  {
        emit positionInfoChanged();
    }
    if (hasTTChanged) {
        emit ttChanged();
    }
    if (hasGroundSpeedChanged) {
       emit groundSpeedChanged();
    }
    if (hasClimbRateChanged) {
        emit climbRateChanged();
    }
    if (hasTypeChanged) {
        emit typeChanged();
    }
    if (hasVDistChanged) {
        emit vDistChanged();
    }

    setAnimate(true);
}


void Traffic::TrafficFactor::setDescription()
{
    QStringList results;

    switch(_type) {
    case Aircraft:
        results << tr("Aircraft");
        break;
    case Airship:
        results << tr("Airship");
        break;
    case Balloon:
        results << tr("Balloon");
        break;
    case Copter:
        results << tr("Copter");
        break;
    case Drone:
        results << tr("Drone");
        break;
    case Glider:
        results << tr("Glider");
        break;
    case HangGlider:
        results << tr("Hang glider");
        break;
    case Jet:
        results << tr("Jet");
        break;
    case Paraglider:
        results << tr("Paraglider");
        break;
    case Skydiver:
        results << tr("Skydiver");
        break;
    case StaticObstacle:
        results << tr("Static Obstacle");
        break;
    case TowPlane:
        results << tr("Tow Plane");
        break;
    default:
        results << tr("Traffic");
        break;
    }

    if (!_positionInfo.coordinate().isValid()) {
        results << tr("Position unknown");
    }

    if (_vDist.isFinite()) {
        auto result = _vDist.toString(GlobalSettings::useMetricUnitsStatic(), true, true);
        auto climbRateMPS = climbRate().toMPS();
        if ( qIsFinite(climbRateMPS) ) {
            if (climbRateMPS < -1.0) {
                result += " ↘";
            }
            if ((climbRateMPS >= -1.0) && (climbRateMPS <= +1.0)) {
                result += " →";
            }
            if (climbRateMPS > 1.0) {
                result += " ↗";
            }
        }
        results << result;
    }

    auto newDescription = results.join(u"<br>");

    // Set value and emit signal, if appropriate
    if (_description == newDescription) {
        return;
    }
    _description = newDescription;
    emit descriptionChanged();
}


void Traffic::TrafficFactor::setIcon()
{
    // BaseType
    QString baseType = QStringLiteral("noDirection");
    if (_positionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed) && _positionInfo.hasAttribute(QGeoPositionInfo::Direction)) {
        auto GS = AviationUnits::Speed::fromMPS( _positionInfo.attribute(QGeoPositionInfo::GroundSpeed) );
        if (GS.isFinite() && (GS.toKN() > 4)) {
            baseType = QStringLiteral("withDirection");
        }
    }

    auto newIcon = "/icons/traffic-"+baseType+"-"+color()+".svg";

    // Set value and emit signal, if appropriate
    if (_icon == newIcon) {
        return;
    }
    _icon = newIcon;
    emit iconChanged();
}


void Traffic::TrafficFactor::setValid()
{
    // Compute validity
    bool newValid = true;
    if (!_positionInfo.timestamp().isValid()) {
        newValid = false;
    } else {
        // If this traffic object it not invalid for trivial reasons, check the age and set the timer.
        auto delta = QDateTime::currentDateTimeUtc().msecsTo( _positionInfo.timestamp().addMSecs(timeoutMS) );
        if (delta <= 0) {
            newValid = false;
        } else {
            newValid = true;
            timeoutCounter.start(delta);
        }

    }

    // Set value and emit signal, if appropriate
    if (_valid == newValid) {
        return;
    }
    _valid = newValid;
    emit validChanged();
}
