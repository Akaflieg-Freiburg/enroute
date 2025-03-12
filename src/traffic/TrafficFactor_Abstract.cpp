/***************************************************************************
 *   Copyright (C) 2020-2025 by Stefan Kebekus                             *
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
#include "navigation/Navigator.h"
#include "traffic/TrafficFactor_Abstract.h"


Traffic::TrafficFactor_Abstract::TrafficFactor_Abstract(QObject* parent) : QObject(parent)
{  
    lifeTimeCounter.setSingleShot(true);
    lifeTimeCounter.setInterval(lifeTime);

    // Binding for property color
    connect(this, &Traffic::TrafficFactor_Abstract::alarmLevelChanged, this, &Traffic::TrafficFactor_Abstract::colorChanged);

    // Binding for property description
    connect(this, &Traffic::TrafficFactor_Abstract::callSignChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateDescription);
    connect(this, &Traffic::TrafficFactor_Abstract::typeChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateDescription);
    connect(this, &Traffic::TrafficFactor_Abstract::vDistChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateDescription);

    // Bindings for property valid
    connect(&lifeTimeCounter, &QTimer::timeout, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateValid);
    connect(this, &Traffic::TrafficFactor_Abstract::alarmLevelChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateValid);
    connect(this, &Traffic::TrafficFactor_Abstract::hDistChanged, this, &Traffic::TrafficFactor_Abstract::dispatchUpdateValid);

    // Binding for property typeString
    m_typeString.setBinding([this]() {
        switch(type()) {
        case Aircraft:
            return tr("Aircraft");
        case Airship:
            return tr("Airship");
        case Balloon:
            return tr("Balloon");
        case Copter:
            return tr("Copter");
        case Drone:
            return tr("Drone");
        case Glider:
            return tr("Glider");
        case HangGlider:
            return tr("Hang glider");
        case Jet:
            return tr("Jet");
        case Paraglider:
            return tr("Paraglider");
        case Skydiver:
            return tr("Skydiver");
        case StaticObstacle:
            return tr("Static Obstacle");
        case TowPlane:
            return tr("Tow Plane");
        default:
            return tr("Traffic");
        }
        return QString();
    });

    // Binding for property relevant
    m_relevant.setBinding([this]() {
        if (!m_valid.value())
        {
            return false;
        }
        if (m_vDist.value().isFinite() && (m_vDist.value() > maxVerticalDistance))
        {
            return false;
        }
        if (m_hDist.value().isFinite() && (m_hDist.value() > maxHorizontalDistance))
        {
            return false;
        }
        return true;
    });

    // Binding for property relevantString
    m_relevantString.setBinding([this]() {
        if (m_relevant.value())
        {
            return tr("Relevant Traffic");
        }
        return tr("Irrelevant Traffic");
    });
}


void Traffic::TrafficFactor_Abstract::dispatchUpdateDescription()
{
    updateDescription();
}


void Traffic::TrafficFactor_Abstract::dispatchUpdateValid()
{
    updateValid();
}


bool Traffic::TrafficFactor_Abstract::hasHigherPriorityThan(const TrafficFactor_Abstract& rhs) const
{
    // Criterion: Valid instances have higher priority than invalid ones
    if (valid() && !rhs.valid())
    {
        return true;
    }
    if (!valid() && rhs.valid())
    {
        return false;
    }

    // Criterion: Alarm level
    if (alarmLevel() > rhs.alarmLevel())
    {
        return true;
    }
    if (alarmLevel() < rhs.alarmLevel())
    {
        return false;
    }

    // Criterion: Relevant instances have higher priority than irrelevant ones
    if (relevant() && !rhs.relevant())
    {
        return true;
    }
    if (!relevant() && rhs.relevant())
    {
        return false;
    }

    if (hDist().isFinite() && vDist().isFinite() && rhs.hDist().isFinite() && rhs.vDist().isFinite())
    {
        return (hDist().toM()*hDist().toM() < rhs.hDist().toM()*rhs.hDist().toM());
    }

    if (hDist().isFinite() && rhs.hDist().isFinite())
    {
        return (hDist() < rhs.hDist());
    }

    return false;
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
        results << GlobalObject::navigator()->aircraft().verticalDistanceToString(vDist(), true);
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
    m_valid = newValid;
}
