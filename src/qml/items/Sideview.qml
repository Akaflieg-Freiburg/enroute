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
import QtQuick.Layouts
import QtQuick.Shapes

import akaflieg_freiburg.enroute
import "../dialogs"

SideviewQuickItem {
    id: rawSideView

    clip: true
    pixelPer10km: flightMap.pixelPer10km

    // Feed the C++ backend the actual content width so it computes
    // coordinates in the Flickable's content space.
    renderWidth: flickable.contentWidth

    // Full-area sky background (scale panels sit on top of it)
    Rectangle {
        anchors.fill: parent
        color: "lightblue"
        z: -1
    }

    // -------------------------------------------------------------------------
    // Unit helpers
    // -------------------------------------------------------------------------
    readonly property bool useFeet: Navigator.aircraft.verticalDistanceUnit === Aircraft.Feet
    readonly property bool useNM:   Navigator.aircraft.horizontalDistanceUnit === Aircraft.NauticalMile
    readonly property bool useSM:   Navigator.aircraft.horizontalDistanceUnit === Aircraft.StatuteMile
    // km → display unit conversions
    readonly property double kmToHDist: useNM ? 0.539957 : (useSM ? 0.621371 : 1.0)
    readonly property string hDistUnit: useNM ? qsTr("NM") : (useSM ? qsTr("mi") : qsTr("km"))
    // ft → display unit for altitude
    readonly property double ftToVDist: useFeet ? 1.0 : 0.3048
    readonly property string vDistUnit: useFeet ? qsTr("ft") : qsTr("m")

    // -------------------------------------------------------------------------
    // Y-axis scale (altitude) — fixed strip on the left, not scrolled
    // -------------------------------------------------------------------------
    readonly property int yScaleWidth: 54
    readonly property int xScaleHeight: 20

    Item {
        id: yScalePanel

        x: 0
        y: 0
        width: rawSideView.yScaleWidth
        height: rawSideView.height - rawSideView.xScaleHeight
        z: 20
        clip: true

        // Semi-transparent background
        Rectangle {
            anchors.fill: parent
            color: "#99F0F0F0"
        }

        // Tick marks and labels
        Repeater {
            id: yTicks

            // Work in display units (ft or m)
            readonly property double minDisp: rawSideView.scaleMinAltFt * rawSideView.ftToVDist
            readonly property double maxDisp: rawSideView.scaleMaxAltFt * rawSideView.ftToVDist
            readonly property double rangeH:  maxDisp - minDisp

            // Pick a tick interval (in display units) giving roughly 4-8 ticks
            readonly property double tickStep: {
                var range = rangeH > 0 ? rangeH : 1000
                var candidates = rawSideView.useFeet
                    ? [100, 200, 500, 1000, 2000, 5000]
                    : [50, 100, 200, 500, 1000, 2000]
                for (var i = 0; i < candidates.length; i++) {
                    if (range / candidates[i] <= 8) { return candidates[i] }
                }
                return candidates[candidates.length - 1]
            }
            readonly property double firstTick: Math.ceil(minDisp / tickStep) * tickStep
            readonly property int tickCount: rangeH > 0
                ? Math.floor((maxDisp - firstTick) / tickStep) + 1
                : 0

            model: (rangeH > 0 && tickCount > 0) ? tickCount : 0

            delegate: Item {
                required property int index
                readonly property double altDisp: yTicks.firstTick + index * yTicks.tickStep
                readonly property double frac: (altDisp - yTicks.minDisp) / yTicks.rangeH
                readonly property double yPos: (1.0 - frac) * yScalePanel.height

                y: yPos - 7
                x: 0
                width: yScalePanel.width
                height: 14
                visible: yPos >= 0 && yPos <= yScalePanel.height

                Rectangle {
                    x: yScalePanel.width - 4
                    y: 6
                    width: 4
                    height: 1
                    color: "#80000000"
                }

                Label {
                    x: 2; y: 0
                    width: yScalePanel.width - 6
                    height: 14
                    text: qsTr("%1 %2").arg(Math.round(altDisp)).arg(rawSideView.vDistUnit)
                    font.pixelSize: 9
                    color: "#CC000000"
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideNone
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // X-axis scale (distance) — fixed strip at the bottom
    // -------------------------------------------------------------------------
    Item {
        id: xScalePanel

        x: rawSideView.yScaleWidth
        y: rawSideView.height - rawSideView.xScaleHeight
        width: rawSideView.width - rawSideView.yScaleWidth
        height: rawSideView.xScaleHeight
        z: 20
        clip: true

        Rectangle {
            anchors.fill: parent
            color: "#99F0F0F0"
        }

        // Scrolling tick marks — shift opposite to Flickable.contentX
        Item {
            x: -flickable.contentX
            width: flickable.contentWidth
            height: xScalePanel.height

            Repeater {
                id: xTicks

                readonly property double totalKm: rawSideView.scaleTotalDistKm
                readonly property double contentW: flickable.contentWidth
                readonly property bool isRouteMode: totalKm > 0

                // distance (in display units) per pixel in content space
                readonly property double dispPerPx: isRouteMode
                    ? (totalKm * rawSideView.kmToHDist / contentW)
                    : (10.0 * rawSideView.kmToHDist / rawSideView.pixelPer10km)

                // pixels per display-unit
                readonly property double pxPerDisp: dispPerPx > 0 ? 1.0 / dispPerPx : 0

                // pick a nice tick interval (display units) giving ≥ 50 px between ticks
                readonly property double tickDist: {
                    if (pxPerDisp <= 0) { return 100 }
                    var candidates = rawSideView.useNM
                        ? [0.5, 1, 2, 5, 10, 20, 50, 100, 200]
                        : [1, 2, 5, 10, 20, 50, 100, 200, 500]
                    for (var i = 0; i < candidates.length; i++) {
                        if (pxPerDisp * candidates[i] >= 50) { return candidates[i] }
                    }
                    return candidates[candidates.length - 1]
                }

                readonly property double totalDispShown: dispPerPx > 0 ? contentW * dispPerPx : 0
                readonly property int tickCount: totalDispShown > 0
                    ? Math.ceil(totalDispShown / tickDist) + 2
                    : 0

                model: tickCount

                delegate: Item {
                    required property int index
                    // distance value this tick represents (in display units)
                    readonly property double dist: {
                        if (xTicks.isRouteMode) {
                            return index * xTicks.tickDist
                        }
                        // track mode: pixel x=0 is 0.2*contentW*dispPerPx behind ownship
                        var startDisp = -0.2 * xTicks.contentW * xTicks.dispPerPx
                        return Math.ceil(startDisp / xTicks.tickDist) * xTicks.tickDist
                               + index * xTicks.tickDist
                    }
                    readonly property double xPos: xTicks.isRouteMode
                        ? (xTicks.pxPerDisp > 0 ? dist * xTicks.pxPerDisp : 0)
                        : (xTicks.pxPerDisp > 0
                           ? (dist + 0.2 * xTicks.contentW * xTicks.dispPerPx) * xTicks.pxPerDisp
                           : 0)

                    x: xPos - 1
                    y: 0
                    width: 2
                    height: xScalePanel.height
                    visible: xPos >= 0 && xPos <= xTicks.contentW

                    Rectangle {
                        x: 0; y: 0; width: 1; height: 4
                        color: "#80000000"
                    }

                    Label {
                        x: 2; y: 2
                        font.pixelSize: 9
                        color: "#CC000000"
                        text: {
                            var v = rawSideView.useNM
                                    ? dist.toFixed(1)
                                    : Math.round(dist)
                            return xTicks.isRouteMode
                                ? qsTr("%1 %2").arg(v).arg(rawSideView.hDistUnit)
                                : (dist >= 0
                                   ? qsTr("+%1 %2").arg(v).arg(rawSideView.hDistUnit)
                                   : qsTr("%1 %2").arg(v).arg(rawSideView.hDistUnit))
                        }
                    }
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Flickable — horizontal scrolling for the content area
    // -------------------------------------------------------------------------
    Flickable {
        id: flickable

        x: rawSideView.yScaleWidth
        y: 0
        width: rawSideView.width - rawSideView.yScaleWidth
        height: rawSideView.height - rawSideView.xScaleHeight

        contentWidth: width * zoomFactor
        contentHeight: height

        flickableDirection: Flickable.HorizontalFlick
        boundsBehavior: Flickable.StopAtBounds
        clip: true

        property real zoomFactor: 1.0

        Shape {
            id: shp

            width: flickable.contentWidth
            height: flickable.contentHeight

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

            // Planned cruise-altitude profile (route mode only). Sloped segments
            // connecting the planned altitude at each waypoint.
            ShapePath {
                strokeWidth: 3
                strokeColor: Global.flightRouteColor
                fillColor: "transparent"
                PathPolyline { path: rawSideView.plannedProfile }
            }
        }

        // Clickable altitude points, one per waypoint (route mode only). Tapping
        // a point opens a popup to specify the planned altitude. Placed in
        // Flickable content space, aligned with the profile polyline.
        Repeater {
            model: rawSideView.mode === SideviewQuickItem.Route ? rawSideView.plannedProfilePoints : 0

            delegate: Item {
                id: handle
                required property int index
                required property var modelData

                width: 28
                height: 28
                x: modelData.x - width/2
                y: modelData.y - height/2

                // Same marker as the route turnpoints on the moving map.
                Image {
                    anchors.centerIn: parent
                    source: "/icons/waypoints/WP-map.svg"
                    sourceSize.width: 14
                    sourceSize.height: 14
                    scale: tapArea.pressed ? 1.25 : 1.0
                }

                MouseArea {
                    id: tapArea
                    anchors.fill: parent
                    onDoubleClicked: {
                        PlatformAdaptor.vibrateBrief()
                        sideviewAltEditor.index = handle.index
                        sideviewAltEditor.open()
                    }
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
    }

    // -------------------------------------------------------------------------
    // Labels (outside Flickable — anchored to the visible area)
    // -------------------------------------------------------------------------
    Label {
        id: trackLabel

        x: rawSideView.yScaleWidth + flickable.width*0.1
        text: rawSideView.track
        visible: rawSideView.track !== ""
    }

    Label {
        id: errorLabel

        anchors.centerIn: parent
        anchors.horizontalCenterOffset: rawSideView.yScaleWidth / 2.0

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

    // -------------------------------------------------------------------------
    // Zoom buttons — Route mode only
    // -------------------------------------------------------------------------
    Column {
        id: zoomButtons

        anchors.right: modeToggle.left
        anchors.top:   parent.top
        anchors.margins: 4
        spacing: 2
        z: 10

        visible: rawSideView.mode === SideviewQuickItem.Route

        RoundButton {
            width:  28; height: 28; padding: 0
            text: "+"
            font.pixelSize: 16
            opacity: 0.85
            enabled: flickable.zoomFactor < 8.0
            onClicked: flickable.zoomFactor = Math.min(8.0, flickable.zoomFactor * 2.0)
        }

        RoundButton {
            width:  28; height: 28; padding: 0
            text: "−"
            font.pixelSize: 16
            opacity: 0.85
            enabled: flickable.zoomFactor > 1.0
            onClicked: {
                flickable.zoomFactor = Math.max(1.0, flickable.zoomFactor / 2.0)
                if (flickable.contentWidth <= flickable.width) {
                    flickable.contentX = 0
                }
            }
        }
    }

    // Mode toggle button — rendered last so it sits above all other children
    RoundButton {
        id: modeToggle

        anchors.right:   parent.right
        anchors.top:     parent.top
        anchors.margins: 4
        z: 10

        width:  32
        height: 32
        padding: 0

        text: rawSideView.mode === SideviewQuickItem.Route ? "✈" : "⇢"
        font.pixelSize: 16
        opacity: 0.85

        ToolTip.text: rawSideView.mode === SideviewQuickItem.Route
                      ? qsTr("Showing planned route – tap for current track")
                      : qsTr("Showing current track – tap for planned route")
        ToolTip.visible: hovered

        onClicked: {
            if (rawSideView.mode === SideviewQuickItem.Route) {
                rawSideView.mode = SideviewQuickItem.Track
                flickable.zoomFactor = 1.0
                flickable.contentX = 0
            } else {
                rawSideView.mode = SideviewQuickItem.Route
            }
        }
    }

    // Popup to specify the planned altitude of a waypoint, opened by tapping a
    // blue altitude point.
    CenteringDialog {
        id: sideviewAltEditor

        property int index: -1 // Index of waypoint in flight route

        title: qsTr("Planned Altitude")
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel

        onAboutToShow: {
            var m = Navigator.flightRoute.plannedAltitude(sideviewAltEditor.index)
            sideviewAltField.valueMeter = isNaN(m) ? Navigator.aircraft.cruiseAltitudeM : m
        }

        ColumnLayout {
            width: parent.width

            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: qsTr("Planned cruise altitude at this waypoint. Leave empty to use the aircraft's default cruise altitude.")
            }

            ElevationInput {
                id: sideviewAltField
                Layout.fillWidth: true
                currentIndex: Navigator.aircraft.verticalDistanceUnit === Aircraft.Meters ? 1 : 0
                onAccepted: sideviewAltEditor.accept()
            }
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            sideviewAltField.commit()
            Navigator.flightRoute.setPlannedAltitude(sideviewAltEditor.index, sideviewAltField.valueMeter)
        }
    }
}
