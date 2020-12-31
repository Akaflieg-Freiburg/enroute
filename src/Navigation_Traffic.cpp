/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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

#include <QDebug>
#include <QFile>
#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QtGlobal>


#include "GlobalSettings.h"
#include "Navigation_Traffic.h"
#include "SatNav.h"
#include <chrono>

using namespace std::chrono_literals;



Navigation::Traffic::Traffic(QObject *parent) : QObject(parent)
{  
    timeOutCounter.setInterval(3s);
    timeOutCounter.setSingleShot(true);
    connect(&timeOutCounter, &QTimer::timeout, this, &Navigation::Traffic::timeOut);

    connect(this, &Navigation::Traffic::alarmLevelChanged, this, &Navigation::Traffic::setIcon);
    connect(this, &Navigation::Traffic::positionInfoChanged, this, &Navigation::Traffic::setIcon);
    setIcon();
}


void Navigation::Traffic::setData(int newAlarmLevel, const QString& newID, AviationUnits::Distance newHDist, AviationUnits::Distance newVDist, AircraftType newType, const QGeoPositionInfo& newPositionInfo)
{
    timeOutCounter.start();

    if (_ID != newID) {
        setAnimate(false);
        _ID = newID;
    }

    if (_alarmLevel != newAlarmLevel) {
        _alarmLevel = newAlarmLevel;
        emit alarmLevelChanged();
    }

    if (_positionInfo != newPositionInfo) {
        _positionInfo = newPositionInfo;
        emit positionInfoChanged();
    }


    if (_type != newType) {
        _type = newType;
        emit typeChanged();
    }

    if (_vDist != newVDist) {
        _vDist = newVDist;
        emit vDistChanged();
    }

    if (_hDist != newHDist) {
        _hDist = newHDist;
        emit hDistChanged();
    }

    setAnimate(true);
    setIcon();
    emit validChanged();
}


void Navigation::Traffic::copyFrom(const Traffic & other)
{
    qWarning() << "copyFrom";
    setData(other._alarmLevel, other._ID, other._hDist, other._vDist, other._type, other._positionInfo);
}

auto Navigation::Traffic::color() const -> QString
{
    if (_alarmLevel == 0) {
        return QStringLiteral("green");
    }
    if (_alarmLevel == 1) {
        return QStringLiteral("yellow");
    }
    return QStringLiteral("red");
}

void Navigation::Traffic::setIcon()
{
    // BaseType
    QString baseType = QStringLiteral("noDirection");
    if (_positionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
        auto GS = AviationUnits::Speed::fromMPS( _positionInfo.attribute(QGeoPositionInfo::GroundSpeed) );
        if (GS.isFinite() && (GS.toKT() > 4)) {
            baseType = QStringLiteral("withDirection");
        }
    }

    QString newIcon = "/icons/traffic-"+baseType+"-"+color()+".svg";

    if (newIcon == _icon) {
        return;
    }

    _icon = newIcon;
    emit iconChanged();
}


auto Navigation::Traffic::operator>(const Traffic &rhs) -> bool
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


void Navigation::Traffic::timeOut()
{
    emit validChanged();
}


void Navigation::Traffic::setAnimate(bool a)
{
    if (a == _animate) {
        return;
    }

    _animate = a;
    emit animateChanged();
}


auto Navigation::Traffic::valid() const -> bool
{
    if ((_alarmLevel < 0) || (_alarmLevel > 3)) {
        return false;
    }
    if (!_positionInfo.timestamp().isValid()) {
        return false;
    }
    return timeOutCounter.isActive();
}
