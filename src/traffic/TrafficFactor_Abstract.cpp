/***************************************************************************
 *   Copyright (C) 2020-2026 by Stefan Kebekus                             *
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
#include "traffic/TrafficFactorData.h"


using namespace Qt::Literals::StringLiterals;


Traffic::TrafficFactor_Abstract::TrafficFactor_Abstract(QObject* parent) : QObject(parent)
{  
    lifetimeCounter.setSingleShot(true);
    lifetimeCounter.setInterval(lifetime);
    connect(&lifetimeCounter, &QTimer::timeout, this, [this]() {
        // Lifetime expired. The "valid" binding reacts on its own (it reads
        // lifetimeCounter.isActive(), which QTimer notifies on expiry). But
        // "animate" is independent of the timer, so reset it here — otherwise a
        // later reuse of this recycled object would animate from its stale
        // data.
        m_animate = false;
    });

    // Binding for property color
    m_color.setBinding([this]() {
        if (m_alarmLevel == 0)
        {
            return u"green"_s;
        }
        if (m_alarmLevel == 1)
        {
            return u"yellow"_s;
        }
        return u"red"_s;
    });

    // Binding for property validAbstractTrafficFactor
    m_validAbstractTrafficFactor.setBinding([this]() {
        return (m_alarmLevel >= 0) && (m_alarmLevel <= 3) && lifetimeCounter.isActive() && hDist().isFinite();
    });

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
        return m_valid.value() && isRelevant(m_hDist.value(), m_vDist.value());
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


Traffic::TrafficFactor_Abstract::~TrafficFactor_Abstract()
{
    // Break all bindings before destruction proceeds
    m_color.takeBinding();
    m_validAbstractTrafficFactor.takeBinding();
    m_typeString.takeBinding();
    m_relevant.takeBinding();
    m_relevantString.takeBinding();
}


void Traffic::TrafficFactor_Abstract::updateFrom(const TrafficFactorData& data)
{
    // If there is nothing to continue from, this is really a replacement: no
    // established identity to preserve and no meaningful transition to animate.
    if (!valid())
    {
        replaceBy(data);
        return;
    }

    setAlarmLevel(data.alarmLevel);
    if (m_callSign.value().isEmpty())
    {
        setCallSign(data.callSign);
    }
    setHDist(data.hDist);
    setID(data.ID);
    if (m_type.value() == TrafficFactor_Abstract::Type::unknown)
    {
        setType(data.type);
    }
    setVDist(data.vDist);
    m_animate = true;
    startLifetime();
}


void Traffic::TrafficFactor_Abstract::replaceBy(const TrafficFactorData& data)
{
    setAlarmLevel(data.alarmLevel);
    setCallSign(data.callSign);
    setHDist(data.hDist);
    setID(data.ID);
    setType(data.type);
    setVDist(data.vDist);
    m_animate = false;
    startLifetime();
}


bool Traffic::TrafficFactor_Abstract::isRelevant(Units::Distance hDist, Units::Distance vDist)
{
#warning For debug purposes, show all aircraft
    return true;
    if (vDist.isFinite() && (vDist > maxVerticalDistance))
    {
        return false;
    }
    if (hDist.isFinite() && (hDist > maxHorizontalDistance))
    {
        return false;
    }
    return true;
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


void Traffic::TrafficFactor_Abstract::startLifetime()
{
    lifetimeCounter.start();
}


bool Traffic::hasHigherPriorityThan(const TrafficFactorData& lhs, const TrafficFactor_Abstract& rhs)
{
    // A freshly received data record always carries current data and is
    // therefore treated as valid. This mirrors
    // TrafficFactor_Abstract::hasHigherPriorityThan().

    // Criterion: Valid instances have higher priority than invalid ones
    if (!rhs.valid())
    {
        return true;
    }

    // Criterion: Alarm level
    if (lhs.alarmLevel > rhs.alarmLevel())
    {
        return true;
    }
    if (lhs.alarmLevel < rhs.alarmLevel())
    {
        return false;
    }

    // Criterion: Relevant instances have higher priority than irrelevant ones
    const bool lhsRelevant = TrafficFactor_Abstract::isRelevant(lhs.hDist, lhs.vDist);
    if (lhsRelevant && !rhs.relevant())
    {
        return true;
    }
    if (!lhsRelevant && rhs.relevant())
    {
        return false;
    }

    if (lhs.hDist.isFinite() && lhs.vDist.isFinite() && rhs.hDist().isFinite() && rhs.vDist().isFinite())
    {
        return (lhs.hDist.toM()*lhs.hDist.toM() < rhs.hDist().toM()*rhs.hDist().toM());
    }

    if (lhs.hDist.isFinite() && rhs.hDist().isFinite())
    {
        return (lhs.hDist < rhs.hDist());
    }

    return false;
}
