/***************************************************************************
 *   Copyright (C) 2019-2026 by Stefan Kebekus                             *
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
import QtCore
import QtLocation
import QtPositioning
import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Shapes

import akaflieg_freiburg.enroute

import "."
import "../dialogs"

Item {
    id: page

    enum MapBearingPolicies { NUp=0, TTUp=1, UserDefinedBearingUp=2 }


    Connections {
        target: DemoRunner

        function onRequestMapBearing(newBearing) {
            mapBearingPolicy = newBearing
        }
        function onRequestShowSideView(show) {
            if (show)
                cl.SplitView.preferredHeight = page.height/3
            else
                cl.SplitView.preferredHeight = 0
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RemainingRouteBar {
            id: remainingRoute

            Layout.fillWidth: true

            visible: !Global.currentVAC.isValid
        }

        SplitView {
            id: splitView

            Layout.fillWidth: true
            Layout.fillHeight: true

            orientation: Qt.Vertical

            Item {
                SplitView.fillHeight: true
                // WARNING: The following line is a workaround against a bug in Qt 6.10.1:
                // as soon as a page header is shown, the implicitHeight of gridView does
                // not update anymore :-(
                SplitView.minimumHeight: followGPSButton.height*5

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

                    onMapReadyChanged: {
                        if (!mapReady)
                            return
                        if (Global.mapBearingPolicy === MFM.UserDefinedBearingUp)
                            flightMap.bearing = Global.mapBearing
                        if (!Global.followGPS)
                            flightMap.center = Global.mapCenter
                        zoomLevel = Global.mapZoomLevel
                    }

                    // GESTURES

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
                                Global.followGPS = false
                                startCentroid = flightMap.toCoordinate(pinch.centroid.position, false)
                                startZoomLevel = flightMap.zoomLevel
                                rawBearing = flightMap.bearing
                                startBearing = flightMap.bearing
                                return
                            }

                            if (flightMap.bearing === 0.0)
                            {
                                Global.mapBearingPolicy = MFM.NUp
                            }
                        }

                        onScaleChanged: function(delta) {
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

                        onRotationChanged: function(delta) {
                            pinch.rawBearing -= delta

                            if (Global.mapBearingPolicy === MFM.UserDefinedBearingUp)
                            {
                                // snap to 0° if we're close enough
                                flightMap.bearing = (Math.abs(pinch.rawBearing) < 5) ? 0 : pinch.rawBearing
                                flightMap.alignCoordinateToPoint(pinch.startCentroid, pinch.centroid.position)
                                return
                            }

                            if (Math.abs(pinch.rawBearing-pinch.startBearing) > 20)
                            {
                                Global.mapBearingPolicy = MFM.UserDefinedBearingUp
                            }
                        }
                    }

                    WheelHandler {
                        id: wheel
                        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
                        onWheel: function(event) {
                            const loc = flightMap.toCoordinate(wheel.point.position)
                            Global.followGPS = false;
                            if (event.modifiers === Qt.NoModifier)
                            {
                                zoomLevelBehavior.enabled = false
                                var newZoom = flightMap.zoomLevel + event.angleDelta.y / 240
                                if (newZoom < flightMap.minimumZoomLevel) {
                                    newZoom = flightMap.minimumZoomLevel
                                }
                                if (newZoom > flightMap.maximumZoomLevel) {
                                    newZoom = flightMap.maximumZoomLevel
                                }
                                flightMap.zoomLevel = newZoom
                                zoomLevelBehavior.enabled = true
                            }
                            else
                            {
                                Global.mapBearingPolicy = MFM.UserDefinedBearingUp
                                flightMap.bearing += event.angleDelta.y / 10
                            }
                            flightMap.alignCoordinateToPoint(loc, wheel.point.position)
                        }
                    }

                    DragHandler {
                        id: drag
                        target: null

                        // Work around https://bugreports.qt.io/browse/QTBUG-87815
                        enabled: !waypointDescription.visible && !Global.drawer.opened && !((Global.dialogLoader.item) && Global.dialogLoader.item.opened)

                        onActiveTranslationChanged: function(delta) {
                            flightMap.pan(-delta.x, -delta.y)
                        }

                        onActiveChanged: {
                            if (active)
                            {
                                Global.followGPS = false
                                if (Global.mapBearingPolicy === MFM.TTUp)
                                {
                                    Global.mapBearingPolicy = MFM.UserDefinedBearingUp
                                }
                            }
                        }
                    }

                    TapHandler {
                        // We used to use a MouseArea instead of a tap handler, but that
                        // triggered a host of bugs in Qt 6.4.2…
                        onDoubleTapped: {
                            if (!gridView.contains(point.position))
                                return

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
                            if (!gridView.contains(point.position))
                                return

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

                    Shortcut {
                        enabled: flightMap.zoomLevel < flightMap.maximumZoomLevel
                        sequences: [StandardKey.ZoomIn]
                        onActivated: flightMap.zoomLevel = Math.round(flightMap.zoomLevel + 1)
                    }

                    Shortcut {
                        enabled: flightMap.zoomLevel > flightMap.minimumZoomLevel
                        sequences: [StandardKey.ZoomOut]
                        onActivated: flightMap.zoomLevel = Math.round(flightMap.zoomLevel - 1)
                    }


                    //
                    // PROPERTY "bearing"
                    //

                    bearing: 0
                    onBearingChanged: Global.mapBearing = bearing
                    Binding on bearing {
                        id: bearingBinding
                        restoreMode: Binding.RestoreNone
                        when: Global.mapBearingPolicy !== MFM.UserDefinedBearingUp
                        value: Global.mapBearingPolicy === MFM.TTUp ? flightMap.animatedTT : 0
                    }


                    //
                    // PROPERTY "center"
                    //

                    center: PositionProvider.lastValidCoordinate
                    onCenterChanged: Global.mapCenter = center
                    Binding on center {
                        restoreMode: Binding.RestoreNone
                        when: Global.followGPS

                        value: {
                            var coordinate = flightMap.animatedCoordinate

                            // In in flight, we position the aircraft someplace on a circle around the
                            // center, so that the map shows a larger portion of the airspace ahead
                            // of the aircraft. The following lines find a good radius for that
                            // circle, which ensures that the circle does not collide with any of the
                            // GUI elements.
                            if (Navigator.flightStatus === Navigator.Flight)
                            {
                                const radiusInPixel = Math.min(centerItem.width/2.0, centerItem.height/2.0 - 2*font.pixelSize)
                                const radiusInM = 10000.0*radiusInPixel/flightMap.pixelPer10km
                                if (isFinite(radiusInM))
                                    coordinate = coordinate.atDistanceAndAzimuth(radiusInM, flightMap.animatedTT)
                            }

                            // Handle the offset between the center of the map and the center of the region that is
                            // not covered by buttons
                            const deltaYInPixel = col2.y + centerItem.y + centerItem.height/2.0 - flightMap.height/2.0
                            const deltaYInM     = 10000.0*deltaYInPixel/flightMap.pixelPer10km
                            if (isFinite(deltaYInM))
                                coordinate = coordinate.atDistanceAndAzimuth(deltaYInM, flightMap.bearing)
                            return coordinate
                        }
                    }


                    //
                    // PROPERTY "zoomLevel"
                    //

                    // Initially, set the zoom level to the last saved value
                    zoomLevel: 12
                    onZoomLevelChanged: Global.mapZoomLevel = zoomLevel
                    Behavior on zoomLevel {
                        id: zoomLevelBehavior
                        NumberAnimation { duration: 400 }
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
                            Global.followGPS = false
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

                Pane {
                    id: noMapWarningRect

                    Material.elevation: 1
                    anchors.centerIn: parent
                    width: parent.width*0.6
                    height: noMapWarning.height+20
                    visible: !Navigator.hasAviationMapForCurrentLocation
                    Label {
                        id: noMapWarning
                        anchors.centerIn: parent
                        width: parent.width-20
                        wrapMode: Text.WordWrap

                        text: {
                            var t = "<p><strong>" + qsTr("No aviation map installed for your present location.") + "</strong></p>"
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
                    id: gridView

                    anchors.fill: parent

                    columns: 3

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

                        Control {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.fillHeight: true
                            Layout.preferredWidth: 26

                            Pane {
                                Material.elevation: 1
                                anchors.fill: parent
                                anchors.bottomMargin: menuButton.bottomInset
                                anchors.topMargin: menuButton.topInset

                                opacity: GlobalSettings.nightMode ? 0.3 : 1.0
                                visible: (!Global.currentVAC.isValid) && !scale.visible

                                contentItem: Scale {
                                    anchors.fill: parent

                                    color: Material.foreground
                                    pixelPer10km: flightMap.pixelPer10km
                                    vertical: true
                                }
                            }
                        }

                        MapButton {
                            id: followGPSButton

                            icon.source: "/icons/material/ic_my_location.svg"

                            enabled: !Global.followGPS
                            visible: enabled

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                Global.followGPS = true
                            }
                        }

                        MapButton {
                            id: trafficDataReceiverButton

                            icon.source: "/icons/material/ic_airplanemode_active.svg"
                            icon.color: "red"

                            enabled: !TrafficDataProvider.receivingHeartbeat || TrafficDataProvider.currentSourceIsInternetService
                            visible: enabled

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
                            Layout.topMargin: 14

                            visible: text !== ""
                            wrapMode: Text.WordWrap

                            text: {
                                var resultList = []

                                if ((!Global.currentVAC.isValid) && (GeoMapProvider.currentRasterMap === "") && (GlobalSettings.airspaceAltitudeLimit.isFinite())) {
                                    var airspaceAltitudeLimit = GlobalSettings.airspaceAltitudeLimit
                                    var airspaceAltitudeLimitString = Navigator.aircraft.verticalDistanceToString(airspaceAltitudeLimit)
                                    resultList.push(qsTr("Airspaces up to %1").arg(airspaceAltitudeLimitString))
                                }
                                if (DataManager.items.downloading)
                                    resultList.push(qsTr("Downloading Maps and Data"))
                                if (NOTAMProvider.status !== "")
                                    resultList.push(NOTAMProvider.status)
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

                        Pane {
                            id: scale

                            Material.elevation: 1
                            Layout.bottomMargin: 14
                            Layout.fillWidth: true
                            Layout.alignment: Qt.AlignVCenter
                            Layout.preferredHeight: 24

                            opacity: GlobalSettings.nightMode ? 0.3 : 1.0
                            visible: (!Global.currentVAC.isValid) && (gridView.height > gridView.width)

                            contentItem: Scale
                            {
                                anchors.fill: parent

                                color: Material.foreground
                                pixelPer10km: flightMap.pixelPer10km
                                vertical: false
                            }
                        }
                    }

                    // Column 3: North Button / Spacer / Zoom In / Zoom Out / Show Sideview
                    ColumnLayout {
                        Layout.fillHeight: true
                        Layout.rightMargin: SafeInsets.right

                        MapButton {
                            id: northButton

                            rotation: -flightMap.bearing

                            icon.source: "/icons/NorthArrow.svg"
                            ToolTip.text: {
                                if (Global.mapBearingPolicy === MFM.NUp)
                                    return qsTr("Current Mode: North Up")
                                if (Global.mapBearingPolicy === MFM.TTUp)
                                    return qsTr("Current Mode: Track Up")
                                if (Global.mapBearingPolicy === MFM.UserDefinedBearingUp)
                                    return qsTr("Current Mode: User Defined Direction Up")
                                return Global.mapBearingPolicy
                            }
                            ToolTip.delay: 1000
                            ToolTip.timeout: 5000
                            ToolTip.visible: hovered

                            onClicked: {
                                if (Global.mapBearingPolicy === MFM.NUp) {
                                    Global.mapBearingPolicy = MFM.TTUp
                                }  else if (Global.mapBearingPolicy === MFM.TTUp) {
                                    Global.mapBearingPolicy = MFM.NUp
                                } else
                                    Global.mapBearingPolicy = Global.mapBearingRevertPolicy
                            }
                        }

                        MapButton {
                            id: rasterMapButton

                            icon.source: "/icons/material/ic_layers.svg"
                            visible: GeoMapProvider.availableRasterMaps.length !== 0
                            ToolTip.text: {
                                if (GeoMapProvider.currentRasterMap === "")
                                    return qsTr("Currently no raster map in use.")
                                return qsTr("Current Raster Map: " + GeoMapProvider.currentRasterMap)
                            }
                            ToolTip.delay: 1000
                            ToolTip.timeout: 5000
                            ToolTip.visible: hovered

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                rasterMenu.popup()

                            }

                            AutoSizingMenu {
                                id: rasterMenu
                                cascade: true

                                Instantiator {
                                    id: recentFilesInstantiator
                                    model: GeoMapProvider.availableRasterMaps
                                    delegate: CheckDelegate {
                                        checked: modelData === GeoMapProvider.currentRasterMap
                                        text: modelData

                                        onClicked: {
                                            PlatformAdaptor.vibrateBrief()
                                            rasterMenu.close()
                                            GeoMapProvider.currentRasterMap = checked ? modelData : ""
                                            flightMap.clearData()
                                        }

                                    }

                                    onObjectAdded: (index, object) => rasterMenu.insertItem(index, object)
                                    onObjectRemoved: (index, object) => rasterMenu.removeItem(object)
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

                        MapButton {
                            id: showSideView

                            icon.source: "/icons/material/ic_keyboard_arrow_up.svg"
                            visible: cl.SplitView.preferredHeight < 100
                            autoRepeat: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                openSideViewAnimation.running = true
                            }

                            NumberAnimation {
                                id: openSideViewAnimation
                                target: cl
                                property: "SplitView.preferredHeight"
                                to: 200
                                duration: 200
                            }

                        }
                    }
                }
            }

            ColumnLayout {
                id: cl

                SplitView.minimumHeight: navBar.implicitHeight

                DragHandler {
                    target: null

                    // Work around https://bugreports.qt.io/browse/QTBUG-87815
                    enabled: !waypointDescription.visible && !Global.drawer.opened && !((Global.dialogLoader.item) && Global.dialogLoader.item.opened)

                    onActiveTranslationChanged: (delta) => cl.SplitView.preferredHeight -= delta.y
                    //onActiveChanged: if (active) cl.SplitView.preferredHeight = navBar.implicitHeight
                }

                spacing: 0  // Set the spacing between children to 0

                Sideview {
                    id: rawSideView
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    pixelPer10km: flightMap.pixelPer10km
                }

                NavBar {
                    id: navBar

                    Layout.fillWidth: true
                }
            }
        }
    }

    WaypointDescription {
        id: waypointDescription
        objectName: "waypointDescription"
    }
}
