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
        id: shp

        preferredRendererType: Shape.CurveRenderer
        asynchronous: true

        ShapePath {
            id: terrain
            strokeWidth: -1
            strokeColor: "saddlebrown"
            fillColor: "brown"

            PathPolyline { path: rawSideView.terrain }
        }

        ShapePath {
            id: airspaces
            strokeWidth: 1
            strokeColor: "black"
            fillColor: "transparent"

            PathMultiline { paths: rawSideView.airspaces }
        }

        property double animatedFiveMinuteBarX: rawSideView.fiveMinuteBar.x
        Behavior on animatedFiveMinuteBarX { NumberAnimation {duration: 1000} }
        property double animatedFiveMinuteBarY: rawSideView.fiveMinuteBar.y
        Behavior on animatedFiveMinuteBarY { NumberAnimation {duration: 1000} }

        ShapePath {
            id: fiveMinuteBar
            strokeWidth: 3
            strokeColor: "black"

            startX: rawSideView.ownshipPosition.x
            startY: rawSideView.ownshipPosition.y
            PathLine {
                relativeX: shp.animatedFiveMinuteBarX
                relativeY: shp.animatedFiveMinuteBarY
            }
        }

        ShapePath {
            id: fiveMinuteBar_1_2
            strokeWidth: 2
            strokeColor: "white"

            startX: rawSideView.ownshipPosition.x + 0.2*shp.animatedFiveMinuteBarX
            startY: rawSideView.ownshipPosition.y + 0.2*shp.animatedFiveMinuteBarY
            PathLine {
                relativeX: 0.2*shp.animatedFiveMinuteBarX
                relativeY: 0.2*shp.animatedFiveMinuteBarY
            }
        }

        ShapePath {
            id: fiveMinuteBar_3_4
            strokeWidth: 2
            strokeColor: "white"

            startX: rawSideView.ownshipPosition.x + 0.6*shp.animatedFiveMinuteBarX
            startY: rawSideView.ownshipPosition.y + 0.6*shp.animatedFiveMinuteBarY
            PathLine {
                relativeX: 0.2*shp.animatedFiveMinuteBarX
                relativeY: 0.2*shp.animatedFiveMinuteBarY
            }
        }
    }

    Image {
        id: imageOP

        x: rawSideView.ownshipPosition.x-width/2.0
        y: rawSideView.ownshipPosition.y-height/2.0
        rotation: {
            if (shp.animatedFiveMinuteBarX > 2)
            {
                return 90 + 360*Math.atan( shp.animatedFiveMinuteBarY/shp.animatedFiveMinuteBarX )/(2*Math.PI)
            }
            return 90
        }

        source: {
            var pInfo = PositionProvider.positionInfo
            if (!pInfo.isValid()) {
                return "/icons/self-noPosition.svg"
            }
            if (!pInfo.trueTrack().isFinite()) {
                return "/icons/self-noDirection.svg"
            }
            return "/icons/self-withDirection.svg"
        }

        sourceSize.width: 25
        sourceSize.height: 25
    }

    Label {
        x: rawSideView.width*0.1
        text: rawSideView.track
        visible: rawSideView.track !== ""
    }

    Label {
        anchors.centerIn: parent

        text: qsTr("Unable to show side view.") + " " + rawSideView.error
        visible: rawSideView.error !== ""
        width: 0.66*rawSideView.width
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter

        leftPadding: font.pixelSize/2.0
        rightPadding: font.pixelSize/2.0
        bottomPadding: font.pixelSize/4.0
        topPadding: font.pixelSize/4.0

        background: Pane { Material.elevation: 1 }
    }

}
