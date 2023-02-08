/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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
import QtQuick.Controls.Material
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
        name: "maplibregl"

        PluginParameter {
            name: "maplibregl.mapping.additional_style_urls"
            value: global.geoMapProvider().styleFileURL
        }

    }

    FlightMap {
        id: flightMap
        objectName: "flightMap"

        anchors.fill: parent

        copyrightsVisible: false // We have our own copyrights notice

        property bool followGPS: true
        property real animatedTrack: PositionProvider.lastValidTT.isFinite() ? PositionProvider.lastValidTT.toDEG() : 0
        Behavior on animatedTrack { RotationAnimation {duration: 400; direction: RotationAnimation.Shortest } }


        // GESTURES

        // Enable gestures. Make sure that whenever a gesture starts, the property "followGPS" is set to "false"
        gesture.enabled: true
        gesture.acceptedGestures: MapGestureArea.PanGesture|MapGestureArea.PinchGesture
        gesture.onPanStarted: {flightMap.followGPS = false}
        gesture.onPinchStarted: {flightMap.followGPS = false}
        gesture.onRotationStarted: {
            flightMap.followGPS = false
            GlobalSettings.mapBearingPolicy = GlobalSettings.UserDefinedBearingUp
        }


        //
        // PROPERTY "bearing"
        //

        // Initially, set the bearing to the last saved value
        bearing: 0

        // If "followGPS" is true, then update the map bearing whenever a new GPS position comes in
        Binding on bearing {
            restoreMode: Binding.RestoreBinding
            when: GlobalSettings.mapBearingPolicy !== GlobalSettings.UserDefinedBearingUp
            value: GlobalSettings.mapBearingPolicy === GlobalSettings.TTUp ? PositionProvider.lastValidTT.toDEG() : 0
        }

        // We expect GPS updates every second. So, we choose an animation of duration 1000ms here, to obtain a flowing movement
        Behavior on bearing { RotationAnimation {duration: 1000; direction: RotationAnimation.Shortest } }


        //
        // PROPERTY "center"
        //

        // Initially, set the center to the last saved value
        center: PositionProvider.lastValidCoordinate

        // If "followGPS" is true, then update the map center whenever a new GPS position comes in
        // or the zoom level changes
        Binding on center {
            id: centerBinding

            restoreMode: Binding.RestoreNone
            when: flightMap.followGPS === true
            value: {
                // If not in flight, then aircraft stays in center of display
                if (Navigator.flightStatus !== Navigator.Flight)
                    return PositionProvider.lastValidCoordinate
                if (!PositionProvider.lastValidTT.isFinite())
                    return PositionProvider.lastValidCoordinate

                // Otherwise, we position the aircraft someplace on a circle around the
                // center, so that the map shows a larger portion of the airspace ahead
                // of the aircraft. The following lines find a good radius for that
                // circle, which ensures that the circle does not collide with any of the
                // GUI elements.
                const xCenter = flightMap.width/2.0
                const yCenter = flightMap.height/2.0
                const radiusInPixel = Math.min(
                                        Math.abs(xCenter-zoomIn.x),
                                        Math.abs(xCenter-followGPSButton.x-followGPSButton.width),
                                        Math.abs(yCenter-northButton.y-northButton.height),
                                        Math.abs(yCenter-zoomIn.y)
                                        )
                const radiusInM = 10000.0*radiusInPixel/flightMap.pixelPer10km

                return PositionProvider.lastValidCoordinate.atDistanceAndAzimuth(radiusInM, PositionProvider.lastValidTT.toDEG())
            }
        }

        // We expect GPS updates every second. So, we choose an animation of duration 1000ms here, to obtain a flowing movement
        Behavior on center {
            id: centerBindingAnimation
            CoordinateAnimation { duration: 1000 }
            enabled: true

            function omitAnimationforZoom() {
                centerBindingAnimation.enabled = false
                omitAnimationForZoomTimer.stop()  // stop in case it was already runnnig
                omitAnimationForZoomTimer.start()
            }
        }

        Timer {
            id: omitAnimationForZoomTimer
            interval: 410  // little more than time for animation
            onTriggered: centerBindingAnimation.enabled = true
        }


        //
        // PROPERTY "zoomLevel"
        //

        // Initially, set the zoom level to the last saved value
        zoomLevel: 12

        // Animate changes in zoom level for visually smooth transition
        Behavior on zoomLevel { NumberAnimation { duration: 400 } }


        // ADDITINAL MAP ITEMS
        MapCircle { // Circle for nondirectional traffic warning
            center: PositionProvider.lastValidCoordinate

            radius: Math.max(500, global.trafficDataProvider().trafficObjectWithoutPosition.hDist.toM())
            Behavior on radius {
                NumberAnimation { duration: 1000 }
                enabled: global.trafficDataProvider().trafficObjectWithoutPosition.animate
            }

            color: global.trafficDataProvider().trafficObjectWithoutPosition.color
            Behavior on color {
                ColorAnimation { duration: 400 }
                enabled: global.trafficDataProvider().trafficObjectWithoutPosition.animate
            }
            opacity: 0.3
            visible: global.trafficDataProvider().trafficObjectWithoutPosition.valid
        }

        MapQuickItem {
            id: mapCircleLabel

            property real distFromCenter: 0.5*Math.sqrt(lbl.width*lbl.width + lbl.height*lbl.height) + 28

            coordinate: PositionProvider.lastValidCoordinate
            Behavior on coordinate {
                CoordinateAnimation { duration: 1000 }
                enabled: global.trafficDataProvider().trafficObjectWithoutPosition.animate
            }

            visible: global.trafficDataProvider().trafficObjectWithoutPosition.valid

            Connections {
                // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
                // is not updated when the height of the map changes. It does get updated when the
                // width of the map changes. We use the undocumented method polishAndUpdate() here.
                target: flightMap
                function onHeightChanged() { mapCircleLabel.polishAndUpdate() }
            }

            sourceItem: Label {
                id: lbl

                x: -width/2
                y: mapCircleLabel.distFromCenter - height/2

                text: global.trafficDataProvider().trafficObjectWithoutPosition.description
                textFormat: Text.RichText

                font.pixelSize: 0.8*view.font.pixelSize

                leftInset: -4
                rightInset: -4
                bottomInset: -1
                topInset: -2

                background: Rectangle {
                    border.color: "black"
                    border.width: 1
                    color: Qt.lighter(global.trafficDataProvider().trafficObjectWithoutPosition.color, 1.9)

                    Behavior on color {
                        ColorAnimation { duration: 400 }
                        enabled: global.trafficDataProvider().trafficObjectWithoutPosition.animate
                    }
                    radius: 4
                }
            }
        }

        MapItemView { // Labels for traffic opponents
            model: global.trafficDataProvider().trafficObjects4QML
            delegate: Component {
                TrafficLabel {
                    trafficInfo: model.modelData
                }
            }
        }

        MapQuickItem {
            id: fiveMinuteBar

            anchorPoint.x: fiveMinuteBarBaseRect.width/2
            anchorPoint.y: fiveMinuteBarBaseRect.height
            coordinate: PositionProvider.lastValidCoordinate
            visible: {
                if (!PositionProvider.positionInfo.trueTrack().isFinite())
                    return false
                if (!PositionProvider.positionInfo.groundSpeed().isFinite())
                    return false
                if (PositionProvider.positionInfo.groundSpeed().toMPS() < 2.0)
                    return false
                return true
            }

            Connections {
                // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
                // is not updated when the height of the map changes. It does get updated when the
                // width of the map changes. We use the undocumented method polishAndUpdate() here.
                target: flightMap
                function onHeightChanged() { fiveMinuteBar.polishAndUpdate() }
            }

            sourceItem: Item{
                Rectangle {
                    id: fiveMinuteBarBaseRect

                    property real animatedGroundSpeedInMetersPerSecond: {
                        if (!PositionProvider.positionInfo.groundSpeed().isFinite())
                            return 0.0
                        return PositionProvider.positionInfo.groundSpeed().toMPS()
                    }
                    Behavior on animatedGroundSpeedInMetersPerSecond {NumberAnimation {duration: 400}}

                    rotation: flightMap.animatedTrack-flightMap.bearing
                    transformOrigin: Item.Bottom

                    width: 5

                    height: flightMap.pixelPer10km*(5*60*animatedGroundSpeedInMetersPerSecond)/10000.0

                    color: "black"

                    Rectangle {
                        id: whiteStripe1to2min
                        x: 1
                        y: parent.height/5

                        width: 3
                        height: parent.height/5
                        color: "white"
                    }

                    Rectangle {
                        id: whiteStripe3to4min

                        x: 1
                        y: 3*parent.height/5

                        width: 3
                        height: parent.height/5
                        color: "white"
                    }
                }
            }
        }

        MapQuickItem {
            id: ownPosition

            coordinate: PositionProvider.lastValidCoordinate

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
                    visible: (Navigator.flightStatus === Navigator.Flight) && (PositionProvider.positionInfo.trueTrack().isFinite())
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

        MapPolyline {
            id: flightPath

            line.width: 4
            line.color: "#ff00ff"
            path: Navigator.flightRoute.geoPath
        }

        MapPolyline {
            id: toNextWP
            visible: PositionProvider.lastValidCoordinate.isValid &&
                     (Navigator.remainingRouteInfo.status === RemainingRouteInfo.OnRoute)
            line.width: 2
            line.color: 'darkred'
            path: visible ? [PositionProvider.lastValidCoordinate, Navigator.remainingRouteInfo.nextWP.coordinate] : []
        }

        MapItemView { // Traffic opponents
            model: global.trafficDataProvider().trafficObjects4QML
            delegate: Component {
                Traffic {
                    trafficInfo: model.modelData
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

        MapItemView {
            id: waypointLibrary
            model: WaypointLibrary.waypoints
            delegate: waypointComponent
        }

        TapHandler {
            // We used to use a MouseArea instead of a tap handler, but that
            // triggered a host of bugs in Qt 6.4.2…
            onDoubleTapped: {
                PlatformAdaptor.vibrateBrief()
                var pos = point.position
                var posTr = Qt.point(pos.x+25,pos.y)

                var wp = global.geoMapProvider().closestWaypoint(flightMap.toCoordinate(pos),
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

                var wp = global.geoMapProvider().closestWaypoint(flightMap.toCoordinate(pos),
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
    }

    BrightnessContrast { // Graphical effects: increase contrast, reduce brightness in dark mode
        anchors.fill: flightMap
        source: flightMap
        brightness: Material.theme == Material.Dark ? -0.9 : -0.2
        contrast: Material.theme == Material.Dark ? 0.6 : 0.2
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

            text: qsTr("<p><strong>There is no aviation map installed.</strong></p>
<p>In order to install a map, please open the menu using the menu button in the upper left corner of this screen.
Choose <strong>Library/Maps and Data</strong> to open the map management page.</p>")
            textFormat: Text.StyledText
            color: "red"
        }
    }

    RemainingRouteBar {
        id: remainingRoute

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Pane {
        id: airspaceAltLabel

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: menuButton.verticalCenter

        topPadding: 0
        bottomPadding: 0
        Material.elevation: 2
        opacity: 0.8
        visible: GlobalSettings.airspaceAltitudeLimit.isFinite() && !DataManager.baseMapsRaster.hasFile

        Label {

            text: {
                // Mention
                Navigator.aircraft.verticalDistanceUnit

                var airspaceAltitudeLimit = GlobalSettings.airspaceAltitudeLimit
                var airspaceAltitudeLimitString = Navigator.aircraft.verticalDistanceToString(airspaceAltitudeLimit)
                return " "+qsTr("Airspaces up to %1").arg(airspaceAltitudeLimitString)+" "
            }
        }
    }

    RoundButton {
        id: menuButton
        icon.source: "/icons/material/ic_menu.svg"

        height: 4*font.pixelSize
        width: 4*font.pixelSize

        anchors.left: parent.left
        anchors.leftMargin: 0.5*view.font.pixelSize + SafeInsets.left
        anchors.top: remainingRoute.bottom
        anchors.topMargin: 0.5*view.font.pixelSize

        Material.background: GlobalSettings.nightMode ? undefined : "white"

        onClicked: {
            PlatformAdaptor.vibrateBrief()
            drawer.open()
        }
    }

    RoundButton {
        id: northButton

        anchors.horizontalCenter: zoomIn.horizontalCenter
        anchors.verticalCenter: menuButton.verticalCenter

        horizontalPadding: 0
        verticalPadding: 0
        height: 3.3*font.pixelSize
        width: 3.3*font.pixelSize

        contentItem: Image {
            Layout.alignment: Qt.AlignHCenter
            id: northArrow

            opacity: GlobalSettings.nightMode ? 0.3 : 1.0
            rotation: -flightMap.bearing

            source: "/icons/NorthArrow.svg"
        }

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

    RoundButton {
        id: followGPSButton

        opacity: 0.9
        icon.source: "/icons/material/ic_my_location.svg"
        enabled: !flightMap.followGPS

        anchors.left: parent.left
        anchors.leftMargin: 0.5*view.font.pixelSize + SafeInsets.left
        anchors.bottom: trafficDataReceiverButton.top
        anchors.bottomMargin: trafficDataReceiverButton.visible ? 0.5*view.font.pixelSize : 1.5*view.font.pixelSize

        height: 4*font.pixelSize
        width: 4*font.pixelSize
        Material.background: GlobalSettings.nightMode ? undefined : "white"

        onClicked: {
            PlatformAdaptor.vibrateBrief()
            flightMap.followGPS = true
            toast.doToast(qsTr("Map Mode: Autopan"))
        }
    }

    RoundButton {
        id: trafficDataReceiverButton

        Material.background: global.trafficDataProvider().receivingHeartbeat ? Material.Green : Material.Red

        opacity: Material.theme === Material.Dark ? 0.3 : 0.8
        icon.source: "/icons/material/ic_airplanemode_active.svg"
        visible: !global.trafficDataProvider().receivingHeartbeat

        anchors.left: parent.left
        anchors.leftMargin: 0.5*view.font.pixelSize + SafeInsets.left
        anchors.bottom: navBar.top
        anchors.bottomMargin: visible ? 1.5*view.font.pixelSize : 0

        height: visible ? 4*font.pixelSize : 0
        width: 4*font.pixelSize

        onClicked: {
            PlatformAdaptor.vibrateBrief()
            PlatformAdaptor.vibrateBrief()
            stackView.pop()
            stackView.push("../pages/TrafficReceiver.qml")
        }
    }

    RoundButton {
        id: zoomIn

        opacity: 0.9
        icon.source: "/icons/material/ic_add.svg"
        enabled: flightMap.zoomLevel < flightMap.maximumZoomLevel
        autoRepeat: true

        anchors.right: parent.right
        anchors.rightMargin: 0.5*view.font.pixelSize + SafeInsets.right
        anchors.bottom: zoomOut.top
        anchors.bottomMargin: 0.5*view.font.pixelSize

        height: 4*font.pixelSize
        width: 4*font.pixelSize
        Material.background: GlobalSettings.nightMode ? undefined : "white"

        onClicked: {
            centerBindingAnimation.omitAnimationforZoom()
            PlatformAdaptor.vibrateBrief()
            flightMap.zoomLevel += 1
        }
    }

    RoundButton {
        id: zoomOut

        opacity: 0.9
        icon.source: "/icons/material/ic_remove.svg"
        enabled: flightMap.zoomLevel > flightMap.minimumZoomLevel
        autoRepeat: true

        anchors.right: parent.right
        anchors.rightMargin: 0.5*view.font.pixelSize + SafeInsets.right
        anchors.bottom: navBar.top
        anchors.bottomMargin: 1.5*view.font.pixelSize

        height: 4*font.pixelSize
        width: 4*font.pixelSize
        Material.background: GlobalSettings.nightMode ? undefined : "white"

        onClicked: {
            centerBindingAnimation.omitAnimationforZoom()
            PlatformAdaptor.vibrateBrief()
            flightMap.zoomLevel -= 1
        }
    }

    Scale {
        id: leftScale

        anchors.top: northButton.bottom
        anchors.topMargin: 0.5*view.font.pixelSize
        anchors.bottom: followGPSButton.top
        anchors.bottomMargin: 0.5*view.font.pixelSize
        anchors.horizontalCenter: followGPSButton.horizontalCenter

        opacity: Material.theme === Material.Dark ? 0.3 : 1.0
        visible: !scale.visible

        pixelPer10km: flightMap.pixelPer10km
        vertical: true
        width: 30
    }

    Scale {
        id: scale

        anchors.left: followGPSButton.right
        anchors.leftMargin: 0.5*view.font.pixelSize
        anchors.right: zoomIn.left
        anchors.rightMargin: 0.5*view.font.pixelSize
        anchors.verticalCenter: zoomOut.verticalCenter

        opacity: Material.theme === Material.Dark ? 0.3 : 1.0
        visible: parent.height > parent.width

        pixelPer10km: flightMap.pixelPer10km
        vertical: false
        height: 30
    }

    Pane {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: navBar.top
        anchors.bottomMargin: 0.4*view.font.pixelSize
        topPadding: 0
        bottomPadding: 0
        Material.elevation: 2
        opacity: 0.8

        Label {
            id: noCopyrightInfo
            text: "<a href='xx'>"+qsTr("Map Data Copyright Info")+"</a>"
            onLinkActivated: copyrightDialog.open()

            LongTextDialog {
                id: copyrightDialog
                title: qsTr("Map Data Copyright Information")
                text: global.geoMapProvider().copyrightNotice
                standardButtons: Dialog.Cancel
            }
        }
    }

    NavBar {
        id: navBar

        anchors.right: parent.right
        anchors.left: parent.left

        y: parent.height - height
    }

    WaypointDescription {
        id: waypointDescription
        objectName: "waypointDescription"
    }
}
