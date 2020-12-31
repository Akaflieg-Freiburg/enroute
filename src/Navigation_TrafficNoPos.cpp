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

#include <QDebug>
#include <QFile>
#include <QGeoCoordinate>
#include <QGeoPositionInfo>
#include <QtGlobal>


#include "GlobalSettings.h"
#include "Navigation_TrafficNoPos.h"
#include "SatNav.h"



Navigation::TrafficNoPos::TrafficNoPos(QObject *parent) : QObject(parent)
{  
    timeOutCounter.setInterval(3*1000);
    timeOutCounter.setSingleShot(true);
    connect(&timeOutCounter, &QTimer::timeout, this, &Navigation::TrafficNoPos::timeOut);
}


void Navigation::TrafficNoPos::setData(int __alarmLevel, QString __ID,  AviationUnits::Distance __vdist, AviationUnits::Distance __hdist, Type __type)
{
    timeOutCounter.start();

    if (_ID != __ID) {
        setAnimate(false);
        _ID = __ID;
    }

    if (_alarmLevel != __alarmLevel) {
        _alarmLevel = __alarmLevel;
        emit alarmLevelChanged();
    }

    if (_type != __type) {
        _type = __type;
        emit typeChanged();
    }

    if (_vDist != __vdist) {
        _vDist = __vdist;
        emit vDistChanged();
    }

    if (_hDist != __hdist) {
        _hDist = __hdist;
        emit hDistChanged();
    }

    setAnimate(true);
    emit validChanged();
}


void Navigation::TrafficNoPos::copyFrom(const TrafficNoPos & other)
{
    qWarning() << "copyFrom";
    setData(other._alarmLevel, other._ID, other._hDist, other._vDist, other._type);
}


bool Navigation::TrafficNoPos::operator>(const TrafficNoPos &rhs)
{
    // Criterion 1: Valid instances have higher priority than invalid ones
    if (!rhs.valid())
        return true;
    if (!valid())
        return false;
    // At this point, both instances are valid.

    // Criterion 2: Alarm level
    if (_alarmLevel > rhs._alarmLevel)
        return true;
    if (_alarmLevel < rhs._alarmLevel)
        return false;
    // At this point, both instances have equal alarm levels

    // Final criterion: distance to current position
    return _hDist < rhs._hDist;
}


void Navigation::TrafficNoPos::timeOut()
{
    emit validChanged();
}


void Navigation::TrafficNoPos::setAnimate(bool a)
{
    if (a == _animate)
        return;

    _animate = a;
    emit animateChanged();
}


bool Navigation::TrafficNoPos::valid() const
{
    if ((_alarmLevel < 0) || (_alarmLevel > 3))
        return false;
    if (!_hDist.isFinite())
        return false;
    return timeOutCounter.isActive();
}


QString Navigation::TrafficNoPos::color() const
{
    if (_alarmLevel == 0)
        return "green";
    if (_alarmLevel == 1)
        return "yellow";
    return "red";
}
