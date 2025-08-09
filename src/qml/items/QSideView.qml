/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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

import QtQuick
import QtQuick.Controls
import QtQuick.Shapes

import akaflieg_freiburg.enroute

RawSideView {
    id: rawSideView

    clip: true
    pixelPer10km: flightMap.pixelPer10km

    Rectangle {
        anchors.fill: parent
        color: "lightblue"
    }

    Shape {
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            id: terrain
            strokeWidth: -1
            strokeColor: "saddlebrown"
            fillColor: "brown"

            PathPolyline { path: rawSideView.terrain }
        }

        ShapePath {
            id: fiveMinuteBar
            strokeWidth: 3
            strokeColor: "black"

            startX: rawSideView.ownshipPosition.x
            startY: rawSideView.ownshipPosition.y
            PathLine {
                relativeX: rawSideView.fiveMinuteBar.x
                relativeY: rawSideView.fiveMinuteBar.y
            }
        }

        ShapePath {
            id: fiveMinuteBar_1_2
            strokeWidth: 2
            strokeColor: "white"

            startX: rawSideView.ownshipPosition.x + 0.2*rawSideView.fiveMinuteBar.x
            startY: rawSideView.ownshipPosition.y + 0.2*rawSideView.fiveMinuteBar.y
            PathLine {
                relativeX: 0.2*rawSideView.fiveMinuteBar.x
                relativeY: 0.2*rawSideView.fiveMinuteBar.y
            }
        }

        ShapePath {
            id: fiveMinuteBar_3_4
            strokeWidth: 2
            strokeColor: "white"

            startX: rawSideView.ownshipPosition.x + 0.6*rawSideView.fiveMinuteBar.x
            startY: rawSideView.ownshipPosition.y + 0.6*rawSideView.fiveMinuteBar.y
            PathLine {
                relativeX: 0.2*rawSideView.fiveMinuteBar.x
                relativeY: 0.2*rawSideView.fiveMinuteBar.y
            }
        }
    }

    Label {
        x: rawSideView.width*0.1
        text: rawSideView.track
        visible: rawSideView.track !== ""
    }

    Label {
        anchors.centerIn: parent

        text: rawSideView.error
        visible: rawSideView.error !== ""

        leftInset: -4
        rightInset: -4
        bottomInset: -1
        topInset: -2

        background: Rectangle {
            border.color: "black"
            border.width: 1
            color: "white"
        }
    }

}
