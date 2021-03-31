/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

#include "positioning/AbstractPositionInfoSource.h"
#include <chrono>

using namespace std::chrono_literals;



// Member functions

Positioning::AbstractPositionInfoSource::AbstractPositionInfoSource(QObject *parent) : QObject(parent)
{
    // Setup timer
    m_positionInfoTimer.setInterval(10s);
    m_positionInfoTimer.setSingleShot(true);
    connect(&m_positionInfoTimer, &QTimer::timeout, this, &Positioning::AbstractPositionInfoSource::resetPositionInfo);

    // Setup timer
    m_pressureAltitudeTimer.setInterval(10s);
    m_pressureAltitudeTimer.setSingleShot(true);
    connect(&m_pressureAltitudeTimer, &QTimer::timeout, this, &Positioning::AbstractPositionInfoSource::resetPressureAltitude);
}


void Positioning::AbstractPositionInfoSource::setPositionInfo(const QGeoPositionInfo &info)
{
    if (info.isValid()) {
        m_positionInfoTimer.start();
    }
    if (info == m_positionInfo) {
        return;
    }

    m_positionInfo = info;
    emit positionInfoChanged(m_positionInfo);
}

void Positioning::AbstractPositionInfoSource::resetPositionInfo()
{
    setPositionInfo( {} );
}


void Positioning::AbstractPositionInfoSource::setPressureAltitude(AviationUnits::Distance newPressureAltitude)
{
    if (newPressureAltitude.isFinite()) {
        m_pressureAltitudeTimer.start();
    }
    if (newPressureAltitude == m_pressureAltitude) {
        return;
    }

    m_pressureAltitude = newPressureAltitude;
    emit barometricAltitudeChanged();
}


void Positioning::AbstractPositionInfoSource::resetPressureAltitude()
{
    setPressureAltitude( {} );
}
