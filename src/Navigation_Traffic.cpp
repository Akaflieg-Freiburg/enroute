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


void Navigation::Traffic::setData(int __alarmLevel, QString __ID,  AviationUnits::Distance __vdist, Type __type, QGeoPositionInfo __pInfo)
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

    if (_positionInfo != __pInfo) {
        _positionInfo = __pInfo;
        emit positionInfoChanged();
    }


    if (_type != __type) {
        _type = __type;
        emit typeChanged();
    }

    if (_vDist != __vdist) {
        _vDist = __vdist;
        emit vDistChanged();
    }

    setAnimate(true);
    setIcon();
    emit validChanged();
}


void Navigation::Traffic::copyFrom(const Traffic & other)
{
    qWarning() << "copyFrom";
    setData(other._alarmLevel, other._ID, other._vDist, other._type, other._positionInfo);
}


void Navigation::Traffic::setIcon()
{
    // BaseType
    QString baseType = QStringLiteral("noDirection");
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
    auto coordinate = SatNav::globalInstance()->lastValidCoordinate();
    return coordinate.distanceTo(_positionInfo.coordinate()) < coordinate.distanceTo(rhs._positionInfo.coordinate());
}


void Navigation::Traffic::timeOut()
{
    emit validChanged();
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
    return timeOutCounter.isActive();
}
