/***************************************************************************
 *   Copyright (C) 2021-2025 by Stefan Kebekus                             *
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

#include <QQmlEngine>

#include "positioning/PositionInfo.h"
#include "traffic/TrafficDataSource_Abstract.h"


// Member functions

Traffic::TrafficDataSource_Abstract::TrafficDataSource_Abstract(bool isCanonical, QObject *parent)
    : QObject(parent), m_canonical(isCanonical)
{
    QQmlEngine::setObjectOwnership(&m_factor, QQmlEngine::CppOwnership);

    // Setup heartbeat timer
    m_heartbeatTimer.setSingleShot(true);
    m_heartbeatTimer.setInterval(5s);
    connect(&m_heartbeatTimer, &QTimer::timeout, this, [this]() {m_receivingHeartbeat = false;});

    // Setup timer for pressure altitude
    m_pressureAltitudeTimer.setInterval(Positioning::PositionInfo::lifetime);
    m_pressureAltitudeTimer.setSingleShot(true);
    connect(&m_pressureAltitudeTimer, &QTimer::timeout, this, [this]() {m_pressureAltitude = Units::Distance();});

    // Setup other times
    m_pressureAltitudeTimer.setSingleShot(true);
    m_trueAltitudeTimer.setInterval(5s);
    m_trueAltitudeTimer.setSingleShot(true);
}


void Traffic::TrafficDataSource_Abstract::setConnectivityStatus(const QString& newConnectivityStatus)
{
    if (m_connectivityStatus == newConnectivityStatus) {
        return;
    }

    m_connectivityStatus = newConnectivityStatus;
    emit connectivityStatusChanged(m_connectivityStatus);
}


void Traffic::TrafficDataSource_Abstract::setErrorString(const QString& newErrorString)
{
    if (m_errorString == newErrorString) {
        return;
    }

    m_errorString = newErrorString;
    emit errorStringChanged(m_errorString);
}

void Traffic::TrafficDataSource_Abstract::setPressureAltitude(Units::Distance newPressureAltitude)
{
    m_pressureAltitudeTimer.start();
    m_pressureAltitude = newPressureAltitude;
}

void Traffic::TrafficDataSource_Abstract::setReceivingHeartbeat(bool newReceivingHeartbeat)
{
    if (newReceivingHeartbeat)
    {
        m_heartbeatTimer.start();
    }
    else
    {
        m_heartbeatTimer.stop();
    }
    m_receivingHeartbeat = newReceivingHeartbeat;
}

void Traffic::TrafficDataSource_Abstract::resetReceivingHeartbeat()
{
    setReceivingHeartbeat(false);
}
