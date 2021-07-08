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

import QtLocation 5.15
import QtPositioning 5.15
import QtQuick 2.15
import QtQuick.Controls 2.15


MapQuickItem {
    id: trafficLabel

    property var trafficInfo: ({})

    property real distFromCenter: 0.5*Math.sqrt(lbl.width*lbl.width + lbl.height*lbl.height) + 28
    property real t: trafficInfo.positionInfo.trueTrack().isFinite() ? 2*Math.PI*(trafficInfo.positionInfo.trueTrack()-flightMap.bearing)/360.0 : 0

    coordinate: trafficInfo.positionInfo.coordinate().isValid ? trafficInfo.positionInfo.coordinate() : positionProvider.lastValidCoordinate
    Behavior on coordinate {
        CoordinateAnimation { duration: 1000 }
        enabled: trafficInfo.animate
    }

    visible: trafficInfo.valid

    Connections {
        // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
        // is not updated when the height of the map changes. It does get updated when the
        // width of the map changes. We use the undocumented method polishAndUpdate() here.
        target: flightMap
        function onHeightChanged() { trafficLabel.polishAndUpdate() }
    }

    sourceItem: Label {
        id: lbl

        x: -distFromCenter*Math.sin(t) - width/2
        y: distFromCenter*Math.cos(t) - height/2

        text: trafficInfo.description
        textFormat: Text.RichText

        font.pixelSize: 0.8*Qt.application.font.pixelSize

        leftInset: -4
        rightInset: -4
        bottomInset: -1
        topInset: -2

        background: Rectangle {
            border.color: "black"
            border.width: 1
            color: Qt.lighter(trafficInfo.color, 1.9)

            Behavior on color {
                ColorAnimation { duration: 400 }
                enabled: trafficInfo.animate
            }
            radius: 4
        }
    }
}
