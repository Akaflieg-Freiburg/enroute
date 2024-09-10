/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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

import Qt5Compat.GraphicalEffects
import QtLocation
import QtPositioning
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import enroute 1.0


import "."
import ".."
import "../dialogs"

Item {
    id: page

    Plugin {
        id: mapPlugin
        name: "maplibre"

        PluginParameter {
            name: "maplibre.map.styles"
            value: GeoMapProvider.styleFileURL
        }

    }

    FlightMap {
        id: flightMap
        objectName: "flightMap"

        anchors.fill: parent

        copyrightsVisible: false // We have our own copyrights notice

        property bool followGPS: true
        onFollowGPSChanged: {
            if (followGPS)
                alignMapToCenter()
        }

        property real animatedTrack: PositionProvider.lastValidTT.isFinite() ? PositionProvider.lastValidTT.toDEG() : 0
        Behavior on animatedTrack { RotationAnimation {duration: 1000; direction: RotationAnimation.Shortest } }


        // GESTURES

        // Enable gestures. Make sure that whenever a gesture starts, the property "followGPS" is set to "false"
        PinchHandler {
            id: pinch
            target: null

            property geoCoordinate startCentroid
            property real rawBearing: 0
            property real startBearing: 0
            property bool aboveRotationThreshold: false
            property real startZoomLevel

            onActiveChanged: {
                if (active) {
                    flightMap.followGPS = false
                    startCentroid = flightMap.toCoordinate(pinch.centroid.position, false)
                    startZoomLevel = flightMap.zoomLevel
                    rawBearing = flightMap.bearing
                    startBearing = flightMap.bearing
                    return
                }

                if (flightMap.bearing === 0.0)
                {
                    GlobalSettings.mapBearingPolicy = GlobalSettings.NUp
                }
            }

            onScaleChanged: (delta) => {
                                var newZoom = startZoomLevel+Math.log2(activeScale)
                                if (newZoom < flightMap.minimumZoomLevel) {
                                    newZoom = flightMap.minimumZoomLevel
                                }
                                if (newZoom > flightMap.maximumZoomLevel) {
                                    newZoom = flightMap.maximumZoomLevel
                                }
                                zoomLevelBehavior.enabled = false
                                flightMap.zoomLevel = newZoom
                                flightMap.alignCoordinateToPoint(pinch.startCentroid, pinch.centroid.position)
                                zoomLevelBehavior.enabled = true
                            }

            onRotationChanged: (delta) => {
                                   pinch.rawBearing -= delta

                                   if (GlobalSettings.mapBearingPolicy === GlobalSettings.UserDefinedBearingUp)
                                   {
                                       // snap to 0° if we're close enough
                                       bearingBehavior.enabled = false
                                       flightMap.bearing = (Math.abs(pinch.rawBearing) < 5) ? 0 : pinch.rawBearing
                                       flightMap.alignCoordinateToPoint(pinch.startCentroid, pinch.centroid.position)
                                       bearingBehavior.enabled = true
                                       return
                                   }

                                   if (Math.abs(pinch.rawBearing-pinch.startBearing) > 5)
                                   {
                                       GlobalSettings.mapBearingPolicy = GlobalSettings.UserDefinedBearingUp
                                   }
                               }
        }

        WheelHandler {
            id: wheel
            // workaround for QTBUG-87646 / QTBUG-112394 / QTBUG-112432:
            // Magic Mouse pretends to be a trackpad but doesn't work with PinchHandler
            // and we don't yet distinguish mice and trackpads on Wayland either
            acceptedDevices: Qt.platform.pluginName === "cocoa" || Qt.platform.pluginName === "wayland"
                             ? PointerDevice.Mouse | PointerDevice.TouchPad
                             : PointerDevice.Mouse
            onWheel: (event) => {
                         const loc = flightMap.toCoordinate(wheel.point.position)
                         if (event.modifiers === Qt.NoModifier)
                         {
                             zoomLevelBehavior.enabled = false
                             flightMap.zoomLevel += event.angleDelta.y / 240
                             zoomLevelBehavior.enabled = true
                         }
                         else
                         {
                             bearingBehavior.enabled = false
                             flightMap.bearing += event.angleDelta.y / 5
                             bearingBehavior.enabled = true
                         }
                         flightMap.followGPS = false;
                         flightMap.alignCoordinateToPoint(loc, wheel.point.position)
                     }
        }

        DragHandler {
            id: drag
            target: null

            // Work around https://bugreports.qt.io/browse/QTBUG-87815
            enabled: !waypointDescription.visible && !Global.drawer.opened && !((Global.dialogLoader.item) && Global.dialogLoader.item.opened)

            onActiveTranslationChanged: (delta) => {
                                            if (delta === activeTranslation)
                                            {
                                                return;
                                            }
                                            flightMap.pan(-delta.x, -delta.y)
                                        }

            onActiveChanged: {
                if (active)
                {
                    flightMap.followGPS = false
                    if (GlobalSettings.mapBearingPolicy === GlobalSettings.TTUp)
                    {
                        GlobalSettings.mapBearingPolicy = GlobalSettings.UserDefinedBearingUp
                    }
                }
            }
        }

        Shortcut {
            enabled: flightMap.zoomLevel < flightMap.maximumZoomLevel
            sequence: StandardKey.ZoomIn
            onActivated: flightMap.zoomLevel = Math.round(flightMap.zoomLevel + 1)
        }

        Shortcut {
            enabled: flightMap.zoomLevel > flightMap.minimumZoomLevel
            sequence: StandardKey.ZoomOut
            onActivated: flightMap.zoomLevel = Math.round(flightMap.zoomLevel - 1)
        }


        //
        // PROPERTY "bearing"
        //

        // Initially, set the bearing to the last saved value
        bearing: 0

        // If "followGPS" is true, then update the map bearing whenever a new GPS position comes in
        Binding on bearing {
            restoreMode: Binding.RestoreNone
            when: GlobalSettings.mapBearingPolicy !== GlobalSettings.UserDefinedBearingUp
            value: GlobalSettings.mapBearingPolicy === GlobalSettings.TTUp ? PositionProvider.lastValidTT.toDEG() : 0
        }

        // We expect GPS updates every second. So, we choose an animation of duration 1000ms here, to obtain a flowing movement
        Behavior on bearing {
            id: bearingBehavior
            RotationAnimation {duration: 1000; direction: RotationAnimation.Shortest }
        }


        //
        // PROPERTY "center"
        //

        // If "followGPS" is true, then update the map center whenever a new GPS position comes in
        // or the zoom level changes
        property var centerCoordinate: {
            // If not in flight, then aircraft stays in center of display
            if (Navigator.flightStatus !== Navigator.Flight)
                return ownPosition.coordinate
            if (!PositionProvider.lastValidTT.isFinite())
                return ownPosition.coordinate

            // Otherwise, we position the aircraft someplace on a circle around the
            // center, so that the map shows a larger portion of the airspace ahead
            // of the aircraft. The following lines find a good radius for that
            // circle, which ensures that the circle does not collide with any of the
            // GUI elements.
            const radiusInPixel = Math.min(centerItem.width/2.0, centerItem.height/2.0)
            const radiusInM = 10000.0*radiusInPixel/flightMap.pixelPer10km

            return ownPosition.coordinate.atDistanceAndAzimuth(radiusInM, animatedTrack)
        }

        onCenterCoordinateChanged: {
            if (!flightMap.followGPS)
                return
            alignMapToCenter()
        }

        property var centerPoint: {
            const xCenter = col2.x + centerItem.x + centerItem.width/2.0
            const yCenter = col2.y + centerItem.y + centerItem.height/2.0
            return Qt.point(xCenter, yCenter)
        }

        onCenterPointChanged:  {
            if (!flightMap.followGPS)
                return
            alignMapToCenter()
        }

        onMapReadyChanged: alignMapToCenter()

        function alignMapToCenter() {
            flightMap.alignCoordinateToPoint(centerCoordinate, centerPoint)
        }


        //
        // PROPERTY "zoomLevel"
        //

        // Initially, set the zoom level to the last saved value
        zoomLevel: 12

        // Animate changes in zoom level for visually smooth transition
        Behavior on zoomLevel {
            id: zoomLevelBehavior
            NumberAnimation { duration: 400 }
        }


        // ADDITINAL MAP ITEMS
        MapCircle { // Circle for nondirectional traffic warning
            center: PositionProvider.lastValidCoordinate

            radius: Math.max(500, TrafficDataProvider.trafficObjectWithoutPosition.hDist.toM())
            Behavior on radius {
                NumberAnimation { duration: 1000 }
                enabled: TrafficDataProvider.trafficObjectWithoutPosition.animate
            }

            color: TrafficDataProvider.trafficObjectWithoutPosition.color
            Behavior on color {
                ColorAnimation { duration: 400 }
                enabled: TrafficDataProvider.trafficObjectWithoutPosition.animate
            }
            opacity: 0.1
            visible: TrafficDataProvider.trafficObjectWithoutPosition.valid
        }

        MapQuickItem {
            id: mapCircleLabel

            property real distFromCenter: 0.5*Math.sqrt(lbl.width*lbl.width + lbl.height*lbl.height) + 28

            coordinate: PositionProvider.lastValidCoordinate
            Behavior on coordinate {
                CoordinateAnimation { duration: 1000 }
                enabled: TrafficDataProvider.trafficObjectWithoutPosition.animate
            }

            visible: TrafficDataProvider.trafficObjectWithoutPosition.valid

            Connections {
                // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
                // is not updated when the height of the map changes. It does get updated when the
                // width of the map changes. We use the undocumented method polishAndUpdate() here.
                target: flightMap
                function onHeightChanged() { mapCircleLabel.polishAndUpdate() }
            }

            Control { id: fontGlean }

            sourceItem: Label {
                id: lbl

                x: -width/2
                y: mapCircleLabel.distFromCenter - height/2

                text: TrafficDataProvider.trafficObjectWithoutPosition.description
                textFormat: Text.RichText

                font.pixelSize: 0.8*fontGlean.font.pixelSize

                leftInset: -4
                rightInset: -4
                bottomInset: -1
                topInset: -2

                background: Rectangle {
                    border.color: "black"
                    border.width: 1
                    color: Qt.lighter(TrafficDataProvider.trafficObjectWithoutPosition.color, 1.9)

                    Behavior on color {
                        ColorAnimation { duration: 400 }
                        enabled: TrafficDataProvider.trafficObjectWithoutPosition.animate
                    }
                    radius: 4
                }
            }
        }

        MapItemView { // Labels for traffic opponents
            model: TrafficDataProvider.trafficObjects
            delegate: Component {
                TrafficLabel {
                    trafficInfo: modelData
                }
            }
        }

        MapPolyline {
            id: flightPath
            line.width: 4
            line.color: "#ff00ff"
            path: {
                var array = []
                //Looks weird, but is necessary. geoPath is an 'object' not an array
                Navigator.flightRoute.geoPath.forEach(element => array.push(element))
                return array
            }
        }

        MapPolyline {
            id: toNextWP
            visible: PositionProvider.lastValidCoordinate.isValid &&
                     (Navigator.remainingRouteInfo.status === RemainingRouteInfo.OnRoute)
            line.width: 2
            line.color: 'darkred'
            path: visible ? [PositionProvider.lastValidCoordinate, Navigator.remainingRouteInfo.nextWP.coordinate] : []
        }

        MapQuickItem {
            id: ownPosition

            coordinate: PositionProvider.lastValidCoordinate
            Behavior on coordinate { CoordinateAnimation { duration: 1000 } }

            Connections {
                // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
                // is not updated when the height of the map changes. It does get updated when the
                // width of the map changes. We use the undocumented method polishAndUpdate() here.
                target: flightMap
                function onHeightChanged() { ownPosition.polishAndUpdate() }
            }

            sourceItem: Item {

                rotation: flightMap.animatedTrack-flightMap.bearing

                FlightVector {
                    pixelPerTenKM: flightMap.pixelPer10km
                    groundSpeedInMetersPerSecond: PositionProvider.positionInfo.groundSpeed().toMPS()
                    visible: {
                        if (!PositionProvider.positionInfo.trueTrack().isFinite())
                            return false
                        if (!PositionProvider.positionInfo.groundSpeed().isFinite())
                            return false
                        if (PositionProvider.positionInfo.groundSpeed().toMPS() < 2.0)
                            return false
                        return true
                    }
                }

                Image {
                    id: imageOP

                    x: -width/2.0
                    y: -height/2.0

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

                    sourceSize.width: 50
                    sourceSize.height: 50
                }
            }
        }

        MapItemView { // Traffic opponents
            model: TrafficDataProvider.trafficObjects
            delegate: Component {
                Traffic {
                    map: flightMap
                    trafficInfo: modelData
                }
            }
        }

        Component {
            id: waypointComponent

            MapQuickItem {
                id: midFieldWP

                anchorPoint.x: image.width/2
                anchorPoint.y: image.height/2
                coordinate: model.modelData.coordinate

                Connections {
                    // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
                    // is not updated when the height of the map changes. It does get updated when the
                    // width of the map changes. We use the undocumented method polishAndUpdate() here.
                    target: flightMap
                    function onHeightChanged() { midFieldWP.polishAndUpdate() }
                }

                sourceItem: Item{
                    Image {
                        id: image

                        source:  "/icons/waypoints/WP-map.svg"
                        sourceSize.width: 10
                        sourceSize.height: 10
                    }
                    Label {
                        anchors.verticalCenter: image.verticalCenter
                        anchors.left: image.right
                        anchors.leftMargin: 5
                        text: model.modelData.extendedName
                        color: "black" // Always black, independent of dark/light mode
                        visible: (flightMap.zoomLevel > 11.0) && (model.modelData.extendedName !== "Waypoint")
                        leftInset: -4
                        rightInset: -4
                        topInset: -2
                        bottomInset: -2
                        background: Rectangle {
                            opacity: 0.8
                            border.color: "black"
                            border.width: 0.5
                            color: "white"
                        }
                    }
                }
            }

        }

        MapItemView {
            id: midFieldWaypoints
            model: Navigator.flightRoute.midFieldWaypoints
            delegate: waypointComponent
        }

        TapHandler {
            // We used to use a MouseArea instead of a tap handler, but that
            // triggered a host of bugs in Qt 6.4.2…
            onDoubleTapped: {
                PlatformAdaptor.vibrateBrief()
                var pos = point.position
                var posTr = Qt.point(pos.x+25,pos.y)

                var wp = GeoMapProvider.closestWaypoint(flightMap.toCoordinate(pos),
                                                        flightMap.toCoordinate(posTr))
                if (!wp.isValid)
                    return
                waypointDescription.waypoint = wp
                waypointDescription.open()
            }

            onLongPressed: {
                PlatformAdaptor.vibrateBrief()
                var pos = point.position
                var posTr = Qt.point(pos.x+25,pos.y)

                var wp = GeoMapProvider.closestWaypoint(flightMap.toCoordinate(pos),
                                                        flightMap.toCoordinate(posTr))
                if (!wp.isValid)
                    return
                waypointDescription.waypoint = wp
                waypointDescription.open()
            }
        }

        // On completion, re-consider the binding of the property bearing
        Component.onCompleted: {
            // Oddly, this is necessary, or else the system will try to reset
            // the write-once property 'plugin' on language changes
            plugin = mapPlugin
        }

        Connections {
            target: FileExchange

            function onOpenWaypointRequest(waypoint) {
                flightMap.followGPS = false
                flightMap.center = waypoint.coordinate
            }
        }
    }

    BrightnessContrast { // Graphical effects: increase contrast, reduce brightness in dark mode
        anchors.fill: flightMap
        source: flightMap
        brightness: GlobalSettings.nightMode ? -0.9 : -0.2
        contrast: GlobalSettings.nightMode ? 0.6 : 0.2
        visible: !DataManager.baseMapsRaster.hasFile
    }

    Rectangle {
        id: noMapWarningRect

        color: "white"
        anchors.centerIn: parent
        width: parent.width*0.6
        height: noMapWarning.height+20
        border.color: "black"
        visible: !DataManager.aviationMaps.hasFile
        Label {
            id: noMapWarning
            anchors.centerIn: parent
            width: parent.width-20
            wrapMode: Text.WordWrap

            text: {
                var t = "<p><strong>" + qsTr("There is no aviation map installed.") + "</strong></p>"
                if (DataManager.aviationMaps.downloading)
                    return t + "<p>" + qsTr("Please wait for the download to complete.") + "</p>"
                return t + "<p>" + qsTr("In order to install a map, please open the menu using the menu button in the upper left corner of this screen.") + " " +
                        qsTr("Choose <a href='xx'>Library/Maps and Data</a> to open the map management page.") + "</p>"
            }
            textFormat: Text.RichText
            onLinkActivated: stackView.push("../pages/DataManagerPage.qml", {"dialogLoader": dialogLoader, "stackView": stackView})
        }
    }

    GridLayout {
        anchors.fill: parent
        columns: 3

        RemainingRouteBar {
            id: remainingRoute

            Layout.columnSpan: 3
            Layout.fillWidth: true

            visible: !Global.currentVAC.isValid
        }

        // Column 1: Main Menu / Vertical Scale / ...
        ColumnLayout {
            Layout.fillHeight: true
            Layout.leftMargin: SafeInsets.left

            MapButton {
                id: menuButton

                icon.source: "/icons/material/ic_menu.svg"
                visible: !Global.currentVAC.isValid

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    drawer.open()
                }
            }

            Item {
                Layout.alignment: Qt.AlignHCenter
                Layout.fillHeight: true
                Layout.preferredWidth: 24

                Scale {
                    id: leftScale

                    anchors.fill: parent
                    opacity: GlobalSettings.nightMode ? 0.3 : 1.0
                    visible: (!Global.currentVAC.isValid) && !scale.visible

                    pixelPer10km: flightMap.pixelPer10km
                    vertical: true
                }
            }

            MapButton {
                id: followGPSButton

                icon.source: "/icons/material/ic_my_location.svg"

                enabled: !flightMap.followGPS

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    flightMap.followGPS = true
                    toast.doToast(qsTr("Map Mode: Autopan"))
                }
            }

            MapButton {
                id: trafficDataReceiverButton

                icon.source: "/icons/material/ic_airplanemode_active.svg"
                icon.color: "red"
                enabled: !TrafficDataProvider.receivingHeartbeat

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    PlatformAdaptor.vibrateBrief()
                    stackView.pop()
                    stackView.push("../pages/TrafficReceiver.qml", {"appWindow": view})
                }
            }
        }

        // Colmnn 2: Info Label / Center Item / Copyright / Horizontal Scale
        ColumnLayout {
            id: col2

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.minimumWidth: 0

            Label {
                id: airspaceAltLabel

                Layout.alignment: Qt.AlignHCenter
                Layout.maximumWidth: col2.width

                visible: (!Global.currentVAC.isValid) && !DataManager.baseMapsRaster.hasFile && (text !== "")
                wrapMode: Text.WordWrap

                text: {
                    var resultList = []

                    if (GlobalSettings.airspaceAltitudeLimit.isFinite())
                    {
                        var airspaceAltitudeLimit = GlobalSettings.airspaceAltitudeLimit
                        var airspaceAltitudeLimitString = Navigator.aircraft.verticalDistanceToString(airspaceAltitudeLimit)
                        resultList.push(qsTr("Airspaces up to %1").arg(airspaceAltitudeLimitString))
                    }
                    if (DataManager.items.downloading)
                        resultList.push(qsTr("Downloading Maps and Data"))
                    return resultList.join(" • ")

                }

                leftPadding: font.pixelSize/2.0
                rightPadding: font.pixelSize/2.0
                bottomPadding: font.pixelSize/4.0
                topPadding: font.pixelSize/4.0
                background: Pane { Material.elevation: 1 }
            }

            Item {
                id: centerItem

                Layout.fillHeight: true
                Layout.fillWidth: true
            }

            Label {
                id: noCopyrightInfo

                Layout.alignment: Qt.AlignRight

                visible: (!Global.currentVAC.isValid)
                text: "<font size='2'><a href='xx'>&nbsp;"+qsTr("ⓒ Map Data")+"&nbsp;</a></font>"
                opacity: 0.8

                //style: Text.Outline
                //styleColor: GlobalSettings.nightMode ? "black" : "white"
                background: Pane { opacity: GlobalSettings.nightMode ? 0.3 : 0.8 }
                onLinkActivated: {
                    Global.dialogLoader.active = false
                    Global.dialogLoader.setSource("../dialogs/LongTextDialog.qml", {title: qsTr("Map Data Copyright Information"),
                                                      text: GeoMapProvider.copyrightNotice,
                                                      standardButtons: Dialog.Ok})
                    Global.dialogLoader.active = true
                }
            }

            Scale {
                id: scale

                Layout.bottomMargin: 14
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                Layout.preferredHeight: 24

                opacity: GlobalSettings.nightMode ? 0.3 : 1.0
                visible: (!Global.currentVAC.isValid) && (page.height > page.width)

                pixelPer10km: flightMap.pixelPer10km
                vertical: false
            }
        }

        // Column 3: North Button / Spacer / Zoom In / Zoom Out
        ColumnLayout {
            Layout.fillHeight: true
            Layout.rightMargin: SafeInsets.right

            MapButton {
                id: northButton

                rotation: -flightMap.bearing

                icon.source: "/icons/NorthArrow.svg"

                onClicked: {
                    if (GlobalSettings.mapBearingPolicy === GlobalSettings.NUp) {
                        GlobalSettings.mapBearingPolicy = GlobalSettings.TTUp
                        toast.doToast(qsTr("Map Mode: Track Up"))
                    } else {
                        GlobalSettings.mapBearingPolicy = GlobalSettings.NUp
                        toast.doToast(qsTr("Map Mode: North Up"))
                    }
                }
            }

            Item {
                Layout.fillHeight: true
            }

            MapButton {
                id: zoomIn

                icon.source: "/icons/material/ic_add.svg"
                enabled: flightMap.zoomLevel < flightMap.maximumZoomLevel
                autoRepeat: true

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    flightMap.zoomLevel += 1
                }
            }

            MapButton {
                id: zoomOut

                icon.source: "/icons/material/ic_remove.svg"
                enabled: flightMap.zoomLevel > flightMap.minimumZoomLevel
                autoRepeat: true

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    var newZoomLevel = Math.max(flightMap.zoomLevel - 1, flightMap.minimumZoomLevel)
                    flightMap.zoomLevel = newZoomLevel
                }
            }
        }

        NavBar {
            id: navBar

            Layout.fillWidth: true
            Layout.columnSpan: 3
        }
    }


    WaypointDescription {
        id: waypointDescription
        objectName: "waypointDescription"
    }
}
