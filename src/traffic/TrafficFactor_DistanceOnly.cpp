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


#include <Settings.h>

#include "traffic/TrafficFactor_DistanceOnly.h"


Traffic::TrafficFactor_DistanceOnly::TrafficFactor_DistanceOnly(QObject *parent) : Traffic::TrafficFactor_Abstract(parent)
{  
    // Bindings for property description
    connect(this, &Traffic::TrafficFactor_Abstract::callSignChanged, this, &Traffic::TrafficFactor_Abstract::descriptionChanged);
    connect(this, &Traffic::TrafficFactor_Abstract::typeChanged, this, &Traffic::TrafficFactor_Abstract::descriptionChanged);
    connect(this, &Traffic::TrafficFactor_Abstract::vDistChanged, this, &Traffic::TrafficFactor_Abstract::descriptionChanged);

    // Bindings for property valid
    connect(this, &Traffic::TrafficFactor_DistanceOnly::hDistChanged, this, &Traffic::TrafficFactor_DistanceOnly::updateValid);

}


auto Traffic::TrafficFactor_DistanceOnly::description() const -> QString
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
        results << vDist().toString(Settings::useMetricUnitsStatic(), true, true);
    }

    return results.join(u"<br>");

}


void Traffic::TrafficFactor_DistanceOnly::setHDist(AviationUnits::Distance newHDist) {
    if (m_hDist == newHDist) {
        return;
    }
    m_hDist = newHDist;
    emit hDistChanged();
}


void Traffic::TrafficFactor_DistanceOnly::updateValid()
{

    bool newValid = validAbstract();
    if (hDist().isFinite()) {
        newValid = false;
    }

    // Update property
    if (m_valid != newValid) {
        m_valid = newValid;
        emit validChanged();
    }

}
