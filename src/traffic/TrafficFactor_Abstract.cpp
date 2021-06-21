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


#include "Settings.h"
#include "traffic/TrafficFactor_Abstract.h"


Traffic::TrafficFactor_Abstract::TrafficFactor_Abstract(QObject *parent) : QObject(parent)
{  
    timeoutCounter.setSingleShot(true);

    // Compute derived properties and set bindings
    connect(this, &Traffic::TrafficFactor_Abstract::alarmLevelChanged, this, &Traffic::TrafficFactor_Abstract::setColor);
    setColor();

//    connect(this, &Traffic::TrafficFactor_Abstract::coordinateChanged, this, &Traffic::TrafficFactor_Abstract::setDescription);
    connect(this, &Traffic::TrafficFactor_Abstract::typeChanged, this, &Traffic::TrafficFactor_Abstract::setDescription);
    connect(this, &Traffic::TrafficFactor_Abstract::vDistChanged, this, &Traffic::TrafficFactor_Abstract::setDescription);
#warning
    // setDescription();

//    connect(this, &Traffic::TrafficFactor_Abstract::positionInfoChanged, this, &Traffic::TrafficFactor_Abstract::setValid);
    connect(&timeoutCounter, &QTimer::timeout, this, &Traffic::TrafficFactor_Abstract::setValid);
#warning
    // setValid();
}


void Traffic::TrafficFactor_Abstract::setAnimate(bool a)
{
    if (a == _animate) {
        return;
    }

    _animate = a;
    emit animateChanged();
}


void Traffic::TrafficFactor_Abstract::setColor()
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


void Traffic::TrafficFactor_Abstract::setData(int newAlarmLevel, const QString& newID, AviationUnits::Distance newVDist, AircraftType newType, const QString & newCallSign)
{
    // Set properties
    bool hasAlarmLevelChanged = (_alarmLevel != newAlarmLevel);
    _alarmLevel = newAlarmLevel;

    bool hasIDChanged = (_ID != newID);
    _ID = newID;

    bool hasTypeChanged = (_type != newType);
    _type = newType;

    bool hasVDistChanged = (_vDist != newVDist);
    _vDist = newVDist;

    // If the ID changed, do not animate property changes in the GUI.
    if (hasIDChanged) {
        setAnimate(false);
    }

    bool hasCallSignChanged = (m_callSign != newCallSign);
    m_callSign = newCallSign;

    // Emit notifier signals as appropriate
    if (hasAlarmLevelChanged) {
        emit alarmLevelChanged();
    }
    if (hasIDChanged) {
        emit IDChanged();
        setAnimate(false);
    }
    if (hasTypeChanged) {
        emit typeChanged();
    }
    if (hasVDistChanged) {
        emit vDistChanged();
    }
    if (hasCallSignChanged) {
        emit callSignChanged();
    }

    setAnimate(true);
    setValid();
}
