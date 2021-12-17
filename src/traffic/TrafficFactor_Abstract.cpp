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

#include "GlobalObject.h"
#include "Settings.h"
#include "navigation/Navigator.h"
#include "traffic/TrafficFactor_Abstract.h"


Traffic::TrafficFactor_Abstract::TrafficFactor_Abstract(QObject* parent) : QObject(parent)
{  

    lifeTimeCounter.setSingleShot(true);
    lifeTimeCounter.setInterval(lifeTime);

    // Bindings for property color
    connect(this, &Traffic::TrafficFactor_Abstract::alarmLevelChanged, this, &Traffic::TrafficFactor_Abstract::colorChanged);

    // Bindings for property description
    connect(this, &Traffic::TrafficFactor_Abstract::callSignChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateDescription);
    connect(this, &Traffic::TrafficFactor_Abstract::typeChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateDescription);
    connect(this, &Traffic::TrafficFactor_Abstract::vDistChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateDescription);

    // Bindings for property valid
    connect(&lifeTimeCounter, &QTimer::timeout, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateValid);
    connect(this, &Traffic::TrafficFactor_Abstract::alarmLevelChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateValid);
    connect(this, &Traffic::TrafficFactor_Abstract::hDistChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateValid);

}


void Traffic::TrafficFactor_Abstract::dispatchUpdateDescription()
{
    updateDescription();
}


void Traffic::TrafficFactor_Abstract::dispatchUpdateValid()
{
    updateValid();
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


void Traffic::TrafficFactor_Abstract::startLiveTime()
{

    lifeTimeCounter.start();
    updateValid();

}


void Traffic::TrafficFactor_Abstract::updateDescription()
{
    QStringList results;

    // CallSign
    if (!callSign().isEmpty()) {
        results << callSign();
    }

    // Aircraft type
    switch(type()) {
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

    // Position
    results << tr("Position unknown");

    // Vertical distance
    if (vDist().isFinite()) {
        QString unitString;
        double number;

        switch(GlobalObject::navigator()->aircraft()->verticalDistanceUnit()) {
        case Navigation::Aircraft::Feet:
            number = vDist().toFeet();
            unitString = "ft";
            break;
        case Navigation::Aircraft::Meters:
            number = vDist().toM();
            unitString = "m";
            break;
        }

        // Round value to reasonable numbers
        if (qAbs(number) > 1000.0) {
            number = qRound(number/100.0)*100.0;
        } else if (qAbs(number) > 100.0) {
            number = qRound(number/10.0)*10.0;
        }
        QString signString;
        if (number > 0.0) {
            signString += "+";
        }
        results << signString << QString::number(number) << "&nbsp;" << unitString;
    }

    // Set property value
    auto newDescription = results.join(u"<br>");
    if (m_description == newDescription) {
        return;
    }
    m_description = newDescription;
    emit descriptionChanged();
}


void Traffic::TrafficFactor_Abstract::updateValid()
{

    bool newValid = true;
    if (m_alarmLevel < 0) {
        newValid = false;
    }
    if (m_alarmLevel > 3) {
        newValid = false;
    }
    if (!lifeTimeCounter.isActive()) {
        newValid = false;
    }
    if (!hDist().isFinite()) {
        newValid = false;
    }

    // Update property
    if (m_valid == newValid) {
        return;
    }
    m_valid = newValid;
    emit validChanged();

}
