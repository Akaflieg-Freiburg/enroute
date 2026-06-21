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

    property real t: trafficLabel.trafficInfo.positionInfo.trueTrack().isFinite() ?
                         2*Math.PI*(trafficLabel.trafficInfo.positionInfo.trueTrack().toDEG() - bearing)/360.0 : 0

    // Distance the nearest edge of the label should keep from the traffic symbol
    // (which sits at the label's coordinate). The traffic icon is 40px wide, so
    // this clears it with a small margin.
    readonly property real symbolGap: 26

    // Distance from the symbol to the *center* of the label, along the offset
    // direction (-sin t, cos t). It is derived from the label's actual extent in
    // that direction so that the nearest edge of the label keeps a constant
    // "symbolGap" from the symbol, independent of the label's size and of the
    // direction t. (A fixed distance — e.g. half the label diagonal — made the
    // apparent gap vary a lot with rotation and label dimensions.)
    property real distFromCenter: {
        // |components| of the unit offset direction.
        var s = Math.abs(Math.sin(trafficLabel.t))
        var c = Math.abs(Math.cos(trafficLabel.t))
        var halfW = 0.5*lbl.width
        var halfH = 0.5*lbl.height
        // Distance from the label's center to its edge along the offset direction.
        var edge = (s < 0.0001) ? halfH
                 : (c < 0.0001) ? halfW
                 : Math.min(halfW/s, halfH/c)
        return edge + trafficLabel.symbolGap
    }

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
