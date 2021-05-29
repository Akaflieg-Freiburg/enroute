/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

#include "navigation/Navigator.h"
#include "positioning/PositionProvider.h"


Navigation::Navigator::Navigator(QObject *parent) : QObject(parent)
{
    QTimer::singleShot(0, this, &Navigation::Navigator::deferredInitialization);
}


void Navigation::Navigator::deferredInitialization() const
{
    connect(Positioning::PositionProvider::globalInstance(), &Positioning::PositionProvider::positionInfoChanged, this, &Navigation::Navigator::onPositionUpdated);
}


void Navigation::Navigator::onPositionUpdated(const Positioning::PositionInfo& info)
{
    AviationUnits::Speed GS;

    if (info.isValid()) {
        GS = info.groundSpeed();
    }

    if (!GS.isFinite()) {
        m_isInFlight = false;
        emit isInFlightChanged();
        return;
    }

    if (m_isInFlight) {
        // If we are in flight at present, go back to ground mode only if the ground speed is less than minFlightSpeedInKT-flightSpeedHysteresis
        if ( GS.toKN() < minFlightSpeedInKN-flightSpeedHysteresisInKn ) {
            m_isInFlight = false;
            emit isInFlightChanged();
        }

        return;
        }

    // If we are on the ground at present, go to flight mode only if the ground sped is more than minFlightSpeedInKT
    if ( GS.toKN() > minFlightSpeedInKN ) {
        m_isInFlight = true;
        emit isInFlightChanged();
    }
}

