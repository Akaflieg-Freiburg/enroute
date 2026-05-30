/***************************************************************************
 *   Copyright (C) 2021-2026 by Stefan Kebekus                             *
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

import QtLocation
import QtQuick
import QtQuick.Controls

MapQuickItem {
    id: trafficLabel

    property var trafficInfo: ({})
    property double bearing

    property real distFromCenter: 0.5*Math.sqrt(lbl.width*lbl.width + lbl.height*lbl.height) + 18
    property real t: trafficLabel.trafficInfo.positionInfo.trueTrack().isFinite() ?
                         2*Math.PI*(trafficLabel.trafficInfo.positionInfo.trueTrack().toDEG() - bearing)/360.0 : 0

    coordinate: trafficInfo.extrapolatedCoordinate

    visible: trafficInfo.valid && trafficInfo.relevant && lbl.text !== ""

    Control { id: fontGlean }

    sourceItem: Label {
        id: lbl

        x: -contentWidth/2.0  - trafficLabel.distFromCenter*Math.sin(trafficLabel.t)
        y: -contentHeight/2.0 + trafficLabel.distFromCenter*Math.cos(trafficLabel.t)

        text: trafficLabel.trafficInfo.description
        textFormat: Text.RichText

        font.pixelSize: 0.8*fontGlean.font.pixelSize

        leftInset: -4
        rightInset: -4
        bottomInset: -1
        topInset: -2

        background: Rectangle {
            border.color: "black"
            border.width: 1
            color: Qt.lighter(trafficLabel.trafficInfo.color, 1.9)

            Behavior on color {
                ColorAnimation { duration: 400 }
                enabled: trafficLabel.trafficInfo.animate
            }
            radius: 4
        }
    }
}
