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

    // Night-mode-aware colors. Sky and terrain have no equivalent on the moving
    // map; the night hues are picked to blend with the dark base map. The
    // airspace hues themselves live in the Global singleton (Global.airspaceBlue
    // etc.), see FlightMap.
    readonly property color skyColor:           GlobalSettings.nightMode ? "#101820" : "lightblue"
    readonly property color terrainFillColor:   GlobalSettings.nightMode ? "#33251a" : "brown"
    readonly property color terrainStrokeColor: GlobalSettings.nightMode ? "#54402c" : "saddlebrown"

    // Same role as the properties of the same name in FlightMap: the
    // semi-transparent airspace bands and fills wash out to near-invisible over
    // a dark base, so their opacity is raised at night.
    readonly property real airspaceBandOpacity: GlobalSettings.nightMode ? 0.35 : 0.2
    readonly property real airspaceFillOpacity: GlobalSettings.nightMode ? 0.30 : 0.2

    Rectangle {
        id: sky
        anchors.fill: parent
        color: rawSideView.skyColor
    }

    Shape {
        id: shp

        preferredRendererType: Shape.CurveRenderer
        asynchronous: true

        ShapePath {
            id: terrain
            strokeWidth: -1
            strokeColor: rawSideView.terrainStrokeColor
            fillColor: rawSideView.terrainFillColor

            PathPolyline { path: rawSideView.terrain }
        }

        ShapePath {
            id: airspacesABorder
            strokeWidth: 10
            strokeColor: Qt.alpha(Global.airspaceBlue, rawSideView.airspaceBandOpacity)
            fillColor: "transparent"

            PathMultiline { paths: rawSideView.airspaces["A"] }
        }

        ShapePath {
            id: airspacesA
            strokeWidth: 2
            strokeColor: Global.airspaceBlue
            fillColor: "transparent"

            PathMultiline { paths: rawSideView.airspaces["A"] }
        }

        ShapePath {
            id: airspacesRBorder
            strokeWidth: 10
            strokeColor: Qt.alpha(Global.airspaceRed, rawSideView.airspaceBandOpacity)
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill

            PathMultiline { paths: rawSideView.airspaces["R"] }
        }

        ShapePath {
            id: airspacesR
            strokeWidth: 2
            strokeColor: Global.airspaceRed
            fillColor: "transparent"

            PathMultiline { paths: rawSideView.airspaces["R"] }
        }

        ShapePath {
            id: airspacesPJE
            strokeWidth: 2
            strokeColor: Global.airspaceRed
            fillColor: "transparent"
            dashPattern: [4,3]
            strokeStyle: ShapePath.DashLine

            PathMultiline { paths: rawSideView.airspaces["PJE"] }
        }

        ShapePath {
            id: airspacesTMZ
            strokeWidth: 2
            strokeColor: Global.airspaceNeutral
            fillColor: "transparent"
            dashPattern: [4.0, 3.0, 0.5, 3.0]
            strokeStyle: ShapePath.DashLine

            PathMultiline { paths: rawSideView.airspaces["TMZ"] }
        }

        ShapePath {
            id: airspacesNRABorder
            strokeWidth: 10
            strokeColor: Qt.alpha(Global.airspaceGreen, rawSideView.airspaceBandOpacity)
            fillColor: "transparent"
            fillRule: ShapePath.WindingFill

            PathMultiline { paths: rawSideView.airspaces["NRA"] }
        }

        ShapePath {
            id: airspacesNRA
            strokeWidth: 2
            strokeColor: Global.airspaceGreen
            fillColor: "transparent"

            PathMultiline { paths: rawSideView.airspaces["NRA"] }
        }

        ShapePath {
            id: airspacesRMZ
            strokeWidth: 2
            strokeColor: Global.airspaceBlue
            fillColor: Qt.alpha(Global.airspaceBlue, rawSideView.airspaceFillOpacity)
            dashPattern: [4,3]
            strokeStyle: ShapePath.DashLine
            fillRule: ShapePath.WindingFill

            PathMultiline { paths: rawSideView.airspaces["RMZ"] }
        }

        ShapePath {
            id: airspacesCTR
            strokeWidth: 2
            strokeColor: Global.airspaceBlue
            fillColor: Qt.alpha(Global.airspaceRed, rawSideView.airspaceFillOpacity)
            dashPattern: [4,3]
            strokeStyle: ShapePath.DashLine
            fillRule: ShapePath.WindingFill

            PathMultiline { paths: rawSideView.airspaces["CTR"] }
        }

        property double animatedFiveMinuteBarX: rawSideView.fiveMinuteBar.x
        Behavior on animatedFiveMinuteBarX { NumberAnimation {duration: 1000} }
        property double animatedFiveMinuteBarY: rawSideView.fiveMinuteBar.y
        Behavior on animatedFiveMinuteBarY { NumberAnimation {duration: 1000} }

        // Like the label/halo colors on the moving map, the black bar and its
        // white tick segments flip at night so the bar does not glare over the
        // dark sky.
        property color fiveMinuteBarColor: GlobalSettings.nightMode ? "#e0e0e0" : "black"
        property color fiveMinuteBarTickColor: GlobalSettings.nightMode ? "black" : "white"

        ShapePath {
            id: fiveMinuteBar
            strokeWidth: 3
            strokeColor: shp.fiveMinuteBarColor

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
            strokeColor: shp.fiveMinuteBarTickColor

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
            strokeColor: shp.fiveMinuteBarTickColor

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

        source: PositionProvider.icon

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

        onLinkActivated: {
            dialogLoader.active = false
            dialogLoader.setSource("../dialogs/LongTextDialog.qml",
                                   {
                                       title: qsTr("Static Pressure Unavailable"),
                                       text: Librarian.getStringFromRessource("sideView"),
                                       standardButtons: Dialog.Ok
                                   }
                                   )
            dialogLoader.active = true
        }
    }
}
