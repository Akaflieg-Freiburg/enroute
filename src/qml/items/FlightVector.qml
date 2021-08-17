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

import QtGraphicalEffects 1.15
import QtLocation 5.15
import QtPositioning 5.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import enroute 1.0

import QtQml 2.15

import "."
import ".."


Rectangle {
    id: flightVector

    property real groundSpeedInMetersPerSecond: 0.0
    Behavior on groundSpeedInMetersPerSecond {NumberAnimation {duration: 400}}

    x: -width/2.0
    y: -height
    width: 5
    height: flightMap.pixelPer10km*(5*60*flightVector.groundSpeedInMetersPerSecond)/10000.0

    color: "black"

    Rectangle {
        id: whiteStripe1to2min
        x: 1
        y: parent.height/5

        width: 3
        height: parent.height/5
        color: "white"
    }

    Rectangle {
        id: whiteStripe3to4min

        x: 1
        y: 3*parent.height/5

        width: 3
        height: parent.height/5
        color: "white"
    }
}
