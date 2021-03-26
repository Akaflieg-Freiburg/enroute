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
#include "Navigation_Traffic.h"


Navigation::Traffic::Traffic(QObject *parent) : QObject(parent)
{  
    timeoutCounter.setSingleShot(true);

    // Compute derived properties and set bindings
    connect(this, &Navigation::Traffic::alarmLevelChanged, this, &Navigation::Traffic::setColor);
    setColor();

    connect(this, &Navigation::Traffic::coordinateChanged, this, &Navigation::Traffic::setDescription);
    connect(this, &Navigation::Traffic::typeChanged, this, &Navigation::Traffic::setDescription);
    connect(this, &Navigation::Traffic::vDistChanged, this, &Navigation::Traffic::setDescription);
    setDescription();

    connect(this, &Navigation::Traffic::colorChanged, this, &Navigation::Traffic::setIcon);
    connect(this, &Navigation::Traffic::positionInfoChanged, this, &Navigation::Traffic::setIcon);
    setIcon();

    connect(this, &Navigation::Traffic::positionInfoChanged, this, &Navigation::Traffic::setValid);
    connect(&timeoutCounter, &QTimer::timeout, this, &Navigation::Traffic::setValid);
    setValid();
}


auto Navigation::Traffic::hasHigherPriorityThan(const Traffic &rhs) -> bool
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


void Navigation::Traffic::setAnimate(bool a)
{
    if (a == _animate) {
        return;
    }

    _animate = a;
    emit animateChanged();
}


void Navigation::Traffic::setColor()
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


void Navigation::Traffic::setData(int newAlarmLevel, const QString& newID, AviationUnits::Distance newHDist, AviationUnits::Distance newVDist, AviationUnits::Speed newGroundSpeed, AviationUnits::Speed newClimbRate, AircraftType newType, const QGeoPositionInfo& newPositionInfo)
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

    bool hasClimbRateChanged = (_climbRate != newClimbRate);
    _climbRate = newClimbRate;

    bool hasGroundSpeedChanged = (_groundSpeed != newGroundSpeed);
    _groundSpeed = newGroundSpeed;

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


void Navigation::Traffic::setDescription()
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
        if ( qIsFinite(_climbRate.toMPS())) {
            if (_climbRate.toMPS() < -1.0) {
                result += " ↘";
            }
            if ((_climbRate.toMPS() >= -1.0) && (_climbRate.toMPS() <= +1.0)) {
                result += " →";
            }
            if (_climbRate.toMPS() > 1.0) {
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


void Navigation::Traffic::setIcon()
{
    // BaseType
    QString baseType = QStringLiteral("noDirection");
    if (_positionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed) && _positionInfo.hasAttribute(QGeoPositionInfo::Direction)) {
        auto GS = AviationUnits::Speed::fromMPS( _positionInfo.attribute(QGeoPositionInfo::GroundSpeed) );
        if (GS.isFinite() && (GS.toKT() > 4)) {
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


void Navigation::Traffic::setValid()
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
