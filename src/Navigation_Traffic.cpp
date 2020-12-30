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
#include "Navigation_Traffic.h"
#include "SatNav.h"



Navigation::Traffic::Traffic(QObject *parent) : QObject(parent)
{  
    timeOutCounter.setInterval(3*1000);
    timeOutCounter.setSingleShot(true);
    connect(&timeOutCounter, &QTimer::timeout, this, &Navigation::Traffic::timeOut);

    connect(this, &Navigation::Traffic::alarmLevelChanged, this, &Navigation::Traffic::setIcon);
    connect(this, &Navigation::Traffic::positionInfoChanged, this, &Navigation::Traffic::setIcon);
    setIcon();
}


Navigation::Traffic::Traffic(int __alarmLevel, QString __ID,  AviationUnits::Distance __vDist, Type __type, QGeoPositionInfo __pInfo, QObject *parent)
    : QObject(parent),
      _alarmLevel(__alarmLevel),
      _positionInfo(__pInfo),
      _ID(__ID),
      _type(__type),
      _vDist(__vDist)
{
    timeOutCounter.setInterval(3*1000);
    timeOutCounter.setSingleShot(true);
    timeOutCounter.start();
    connect(&timeOutCounter, &QTimer::timeout, this, &Navigation::Traffic::timeOut);

    connect(this, &Navigation::Traffic::alarmLevelChanged, this, &Navigation::Traffic::setIcon);
    connect(this, &Navigation::Traffic::positionInfoChanged, this, &Navigation::Traffic::setIcon);
    setIcon();
}


void Navigation::Traffic::copyFrom(const Traffic & other)
{
    qWarning() << "copyFrom";
    timeOutCounter.start();

    if (_ID != other._ID) {
        setAnimate(false);
        _ID = other._ID;
    }

    if (_alarmLevel != other._alarmLevel) {
        _alarmLevel = other._alarmLevel;
        emit alarmLevelChanged();
    }

    if (_positionInfo != other._positionInfo) {
        _positionInfo = other._positionInfo;
        qWarning() << _positionInfo;
        emit positionInfoChanged();
    }


    if (_type != other._type) {
        _type = other._type;
        emit typeChanged();
    }

    if (_vDist != other._vDist) {
        _vDist = other._vDist;
        emit vDistChanged();
    }

    setAnimate(true);
}


void Navigation::Traffic::setIcon()
{
    // BaseType
    QString baseType = "noDirection";
    if (_positionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
        auto GS = AviationUnits::Speed::fromMPS( _positionInfo.attribute(QGeoPositionInfo::GroundSpeed) );
        if (GS.isFinite() && (GS.toKT() > 4))
            baseType = "withDirection";
    }

    // Color
    QString color = "red";
    if (_alarmLevel == 0)
        color = "green";
    if (_alarmLevel == 1)
        color = "yellow";

    QString newIcon = "/icons/traffic-"+baseType+"-"+color+".svg";

    if (newIcon == _icon)
        return;

    _icon = newIcon;
    emit iconChanged();
}


bool Navigation::Traffic::operator>(const Traffic &rhs)
{
    if (_alarmLevel > rhs._alarmLevel)
        return true;
    if (_alarmLevel < rhs._alarmLevel)
        return false;

    auto coordinate = SatNav::globalInstance()->lastValidCoordinate();

    return coordinate.distanceTo(_positionInfo.coordinate()) < coordinate.distanceTo(rhs._positionInfo.coordinate());
}


void Navigation::Traffic::timeOut()
{
    if (_alarmLevel == -1)
        return;

    _alarmLevel = -1;
    emit alarmLevelChanged();
}


void Navigation::Traffic::setAnimate(bool a)
{
    if (a == _animate)
        return;

    _animate = a;
    emit animateChanged();
}


bool Navigation::Traffic::valid() const
{
    if ((_alarmLevel < 0) || (_alarmLevel > 3))
        return false;
    if (!_positionInfo.isValid())
        return false;
    return true;
    return timeOutCounter.isActive();
}
