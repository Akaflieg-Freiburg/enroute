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

SideviewQuickItem {
    id: rawSideView

    clip: true
    pixelPer10km: flightMap.pixelPer10km

    Rectangle {
        id: sky
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
            id: airspacesABorder
            strokeWidth: 10
            strokeColor: "#330000FF"
            fillColor: "transparent"

            PathMultiline { paths: rawSideView.airspaces["A"] }
        }

        ShapePath {
            id: airspacesA
            strokeWidth: 2
            strokeColor: "blue"
            fillColor: "transparent"

            PathMultiline { paths: rawSideView.airspaces["A"] }
        }

        ShapePath {
            id: airspacesRBorder
            strokeWidth: 10
            strokeColor: "#33FF0000"
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill

            PathMultiline { paths: rawSideView.airspaces["R"] }
        }

        ShapePath {
            id: airspacesR
            strokeWidth: 2
            strokeColor: "red"
            fillColor: "transparent"

            PathMultiline { paths: rawSideView.airspaces["R"] }
        }

        ShapePath {
            id: airspacesPJE
            strokeWidth: 2
            strokeColor: "red"
            fillColor: "transparent"
            dashPattern: [4,3]
            strokeStyle: ShapePath.DashLine

            PathMultiline { paths: rawSideView.airspaces["PJE"] }
        }

        ShapePath {
            id: airspacesTMZ
            strokeWidth: 2
            strokeColor: "black"
            fillColor: "transparent"
            dashPattern: [4.0, 3.0, 0.5, 3.0]
            strokeStyle: ShapePath.DashLine

            PathMultiline { paths: rawSideView.airspaces["TMZ"] }
        }

        ShapePath {
            id: airspacesNRABorder
            strokeWidth: 10
            strokeColor: "#3300FF00"
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill

            PathMultiline { paths: rawSideView.airspaces["NRA"] }
        }

        ShapePath {
            id: airspacesNRA
            strokeWidth: 2
            strokeColor: "green"
            fillColor: "transparent"

            PathMultiline { paths: rawSideView.airspaces["NRA"] }
        }

        ShapePath {
            id: airspacesRMZ
            strokeWidth: 2
            strokeColor: "blue"
            fillColor: "#330000FF"
            dashPattern: [4,3]
            strokeStyle: ShapePath.DashLine
            fillRule: ShapePath.WindingFill

            PathMultiline { paths: rawSideView.airspaces["RMZ"] }
        }

        ShapePath {
            id: airspacesCTR
            strokeWidth: 2
            strokeColor: "blue"
            fillColor: "#33FF0000"
            dashPattern: [4,3]
            strokeStyle: ShapePath.DashLine
            fillRule: ShapePath.WindingFill

            PathMultiline { paths: rawSideView.airspaces["CTR"] }
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
        id: ownShip

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
        id: trackLabel

        x: rawSideView.width*0.1
        text: rawSideView.track
        visible: rawSideView.track !== ""
    }

    Label {
        id: errorLabel

        anchors.centerIn: parent

        text: rawSideView.error
        visible: rawSideView.error !== ""
        width: 0.66*rawSideView.width
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter

        leftPadding: font.pixelSize/2.0
        rightPadding: font.pixelSize/2.0
        bottomPadding: font.pixelSize/4.0
        topPadding: font.pixelSize/4.0

        background: Pane { Material.elevation: 1 }

        onLinkActivated: Qt.openUrlExternally("https://www.faa.gov/air_traffic/publications/atpubs/aip_html/part2_enr_section_1.7.html")
    }

}
