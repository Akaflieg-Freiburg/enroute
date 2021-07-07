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

    lifeTimeCounter.setSingleShot(true);
    lifeTimeCounter.setInterval(lifeTime);

    // Bindings for property color
    connect(this, &Traffic::TrafficFactor_Abstract::alarmLevelChanged, this, &Traffic::TrafficFactor_Abstract::colorChanged);

    // Bindings for property valid
    connect(&lifeTimeCounter, &QTimer::timeout, this, &Traffic::TrafficFactor_Abstract::updateValid);
    connect(this, &Traffic::TrafficFactor_Abstract::alarmLevelChanged, this, &Traffic::TrafficFactor_Abstract::updateValid);

}


auto Traffic::TrafficFactor_Abstract::hasHigherPriorityThan(const TrafficFactor_Abstract& rhs) const -> bool
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
    if (alarmLevel() > rhs.alarmLevel()) {
        return true;
    }
    if (alarmLevel() < rhs.alarmLevel()) {
        return false;
    }
    // At this point, both instances have equal alarm levels

    // Final criterion: distance to current position
    return (hDist() < rhs.hDist());

}


void Traffic::TrafficFactor_Abstract::setAlarmLevel(int newAlarmLevel)
{

    // Safety checks
    if ((newAlarmLevel < 0) || (newAlarmLevel > 3)) {
        return;
    }

    startLiveTime();
    if (m_alarmLevel == newAlarmLevel) {
        return;
    }
    if ((newAlarmLevel < 0) || (newAlarmLevel > 3)) {
        return;
    }
    m_alarmLevel = newAlarmLevel;
    emit alarmLevelChanged();

}


void Traffic::TrafficFactor_Abstract::setHDist(AviationUnits::Distance newHDist) {

    if (m_hDist == newHDist) {
        return;
    }
    m_hDist = newHDist;
    emit hDistChanged();

}


void Traffic::TrafficFactor_Abstract::startLiveTime()
{

    lifeTimeCounter.start();
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
    return lifeTimeCounter.isActive();
}
