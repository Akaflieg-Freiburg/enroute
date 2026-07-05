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
import QtPositioning
import QtQuick
import QtQuick.Controls

import akaflieg_freiburg.enroute

MapQuickItem {
    id: trafficLabel

    property var trafficInfo: ({})
    property double bearing

    // The traffic's own heading in degrees, animated exactly like the icon in
    // Traffic.qml (same RotationAnimation, duration and "animate" gate). Deriving
    // the label's offset angle "t" from this — instead of from the raw trueTrack —
    // makes the label orbit the symbol in lockstep with the icon's rotation,
    // rather than jumping each time a new heading arrives.
    property double trueTrackDEG: trafficLabel.trafficInfo.positionInfo.trueTrack().isFinite() ?
                                      trafficLabel.trafficInfo.positionInfo.trueTrack().toDEG() : 0
    Behavior on trueTrackDEG {
        RotationAnimation {
            direction: RotationAnimation.Shortest
            duration: 1000
        }
        enabled: trafficLabel.trafficInfo.animate
    }

    property real t: trafficLabel.trafficInfo.positionInfo.trueTrack().isFinite() ?
                         2*Math.PI*(trafficLabel.trueTrackDEG - bearing)/360.0 : 0

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
    // Smoothly glide between the extrapolated positions that
    // TrafficFactor_WithPosition publishes once per second, so that motion looks
    // continuous even though the C++ side only updates at 1 Hz. Gated by
    // "animate" so that newly-appearing or teleporting traffic snaps into place
    // instead of sliding across the map.
    Behavior on coordinate {
        CoordinateAnimation {
            duration: 1000
        }
        enabled: trafficLabel.trafficInfo.animate
    }

    visible: trafficInfo.valid && trafficInfo.relevant && lbl.text !== ""

    Control { id: fontGlean }

    sourceItem: Label {
        id: lbl

        x: -contentWidth/2.0  - trafficLabel.distFromCenter*Math.sin(trafficLabel.t)
        y: -contentHeight/2.0 + trafficLabel.distFromCenter*Math.cos(trafficLabel.t)

        text: trafficLabel.trafficInfo.description
        textFormat: Text.RichText
        color: Global.trafficLabelTextColor

        font.pixelSize: 0.8*fontGlean.font.pixelSize

        leftInset: -4
        rightInset: -4
        bottomInset: -1
        topInset: -2

        background: Rectangle {
            border.color: Global.trafficLabelFrameColor
            border.width: 1
            color: Global.trafficLabelBackgroundColor(trafficLabel.trafficInfo.color)

            Behavior on color {
                ColorAnimation { duration: 400 }
                enabled: trafficLabel.trafficInfo.animate
            }
            radius: 4
        }
    }
}
