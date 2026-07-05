/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

import QtQml
import QtQuick

import akaflieg_freiburg.enroute

Rectangle {
    id: flightVector

    property real pixelPerTenKM
    required property real groundSpeedInMetersPerSecond

    // Gate for the length animation. Callers that represent a single, continuously
    // moving object (e.g. ownship) leave this at "true". Callers that recycle this
    // item for different objects (traffic slots) bind it to the factor's "animate"
    // flag, so the bar snaps instead of tweening when the slot is repurposed.
    property bool animate: true

    Behavior on groundSpeedInMetersPerSecond {
        NumberAnimation {duration: 400}
        enabled: flightVector.animate
    }

    x: -width/2.0
    y: -height
    width: 5
    height: pixelPerTenKM*(5*60*groundSpeedInMetersPerSecond)/10000.0

    // Like the five-minute bar in the side view, the black bar and its white
    // stripes flip at night so the vector does not glare over the dark map.
    color: GlobalSettings.nightMode ? "#e0e0e0" : "black"

    Rectangle {
        id: whiteStripe3to4min

        x: parent.width > 2 ? 1 : 0
        y: parent.height/5

        width: parent.width > 2 ? parent.width-2 : parent.width
        height: parent.height/5
        color: GlobalSettings.nightMode ? "black" : "white"
    }

    Rectangle {
        id: whiteStripe1to2min

        x: parent.width > 2 ? 1 : 0
        y: 3*parent.height/5

        width: parent.width > 2 ? parent.width-2 : parent.width
        height: parent.height/5
        color: GlobalSettings.nightMode ? "black" : "white"
    }
}
