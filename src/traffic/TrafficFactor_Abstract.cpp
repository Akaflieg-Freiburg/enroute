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


#include "traffic/TrafficFactor_Abstract.h"


Traffic::TrafficFactor_Abstract::TrafficFactor_Abstract(QObject *parent) : QObject(parent)
{  
    timeoutCounter.setSingleShot(true);
    timeoutCounter.setInterval(timeout);

    // Bindings for property color
    connect(this, &Traffic::TrafficFactor_Abstract::alarmLevelChanged, this, &Traffic::TrafficFactor_Abstract::colorChanged);

    // Bindings for property valid
    connect(&timeoutCounter, &QTimer::timeout, this, &Traffic::TrafficFactor_Abstract::updateValid);
    connect(this, &Traffic::TrafficFactor_Abstract::alarmLevelChanged, this, &Traffic::TrafficFactor_Abstract::updateValid);

}

#warning clarify and document: need to update timestamp manually or not?

void Traffic::TrafficFactor_Abstract::setAlarmLevel(int newAlarmLevel)
{

    // Safety checks
    if ((newAlarmLevel < 0) || (newAlarmLevel > 3)) {
        return;
    }

    updateTimestamp();
    if (m_alarmLevel == newAlarmLevel) {
        return;
    }
    if ((newAlarmLevel < 0) || (newAlarmLevel > 3)) {
        return;
    }
    m_alarmLevel = newAlarmLevel;
    emit alarmLevelChanged();

}


void Traffic::TrafficFactor_Abstract::updateTimestamp()
{

    timeoutCounter.start();
    updateValid();

}


auto Traffic::TrafficFactor_Abstract::validAbstract() const -> bool
{
    if (m_alarmLevel < 0) {
        return false;
    }
    if (m_alarmLevel > 3) {
        return false;
    }
    return timeoutCounter.isActive();
}
