/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

import QtGraphicalEffects 1.15
import QtLocation 5.15
import QtPositioning 5.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import enroute 1.0

import QtQml 2.15

import "."
import ".."
import "../dialogs"

Item {
    id: page

    Plugin {
        id: mapPlugin
        name: "mapboxgl"

        PluginParameter {
            name: "mapboxgl.mapping.additional_style_urls"
            value: geoMapProvider.styleFileURL
        }

    }

    FlightMap {
        id: flightMap
        objectName: "flightMap"

        anchors.fill: parent

        geoJSON: geoMapProvider.geoJSON
        copyrightsVisible: false // We have our own copyrights notice

        property bool followGPS: true
        property real animatedTrack: positionProvider.lastValidTT.isFinite() ? positionProvider.lastValidTT.toDEG() : 0
        Behavior on animatedTrack { RotationAnimation {duration: 400; direction: RotationAnimation.Shortest } }


        // GESTURES

        // Enable gestures. Make sure that whenever a gesture starts, the property "followGPS" is set to "false"
        gesture.enabled: true
        gesture.acceptedGestures: MapGestureArea.PanGesture|MapGestureArea.PinchGesture
        gesture.onPanStarted: {flightMap.followGPS = false}
        gesture.onPinchStarted: {flightMap.followGPS = false}
        gesture.onRotationStarted: {
            flightMap.followGPS = false
            global.settings().mapBearingPolicy = GlobalSettings.UserDefinedBearingUp
        }


        //
        // PROPERTY "bearing"
        //

        // Initially, set the bearing to the last saved value
        bearing: savedBearing

        // If "followGPS" is true, then update the map bearing whenever a new GPS position comes in
        Binding on bearing {
            restoreMode: Binding.RestoreBinding
            when: global.settings().mapBearingPolicy !== GlobalSettings.UserDefinedBearingUp
            value: global.settings().mapBearingPolicy === GlobalSettings.TTUp ? positionProvider.lastValidTT.toDEG() : 0
        }

        // We expect GPS updates every second. So, we choose an animation of duration 1000ms here, to obtain a flowing movement
        Behavior on bearing { RotationAnimation {duration: 1000; direction: RotationAnimation.Shortest } }


        //
        // PROPERTY "center"
        //

        // Initially, set the center to the last saved value
        center: savedCenter

        // If "followGPS" is true, then update the map center whenever a new GPS position comes in
        // or the zoom level changes
        Binding on center {
            id: centerBinding

            restoreMode: Binding.RestoreNone
            when: flightMap.followGPS === true
            value: {
                // If not in flight, then aircraft stays in center of display
                if (!global.navigator.isInFlight)
                    return positionProvider.lastValidCoordinate
                if (!positionProvider.lastValidTT.isFinite())
                    return positionProvider.lastValidCoordinate

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

                return positionProvider.lastValidCoordinate.atDistanceAndAzimuth(radiusInM, positionProvider.lastValidTT.toDEG())
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
        zoomLevel: savedZoomLevel

        // Animate changes in zoom level for visually smooth transition
        Behavior on zoomLevel { NumberAnimation { duration: 400 } }


        // ADDITINAL MAP ITEMS
        MapCircle { // Circle for nondirectional traffic warning
            center: positionProvider.lastValidCoordinate
            Behavior on center {
                CoordinateAnimation { duration: 1000 }
                enabled: flarmAdaptor.trafficObjectWithoutPosition.animate
            }
            radius: Math.max(500, flarmAdaptor.trafficObjectWithoutPosition.hDistM)
            Behavior on radius {
                NumberAnimation { duration: 1000 }
                enabled: flarmAdaptor.trafficObjectWithoutPosition.animate
            }
            color: flarmAdaptor.trafficObjectWithoutPosition.color
            Behavior on color {
                ColorAnimation { duration: 400 }
                enabled: flarmAdaptor.trafficObjectWithoutPosition.animate
            }
            opacity: 0.3
            visible: flarmAdaptor.trafficObjectWithoutPosition.valid
        }

        TrafficLabel { // Label for nondirectional traffic warning
            trafficInfo: flarmAdaptor.trafficObjectWithoutPosition
        }

        MapItemView { // Labels for traffic opponents
            model: flarmAdaptor.trafficObjects4QML
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
            coordinate: positionProvider.lastValidCoordinate
            visible: (global.navigator().isInFlight) && (positionProvider.positionInfo.trueTrack().isFinite())

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

                    property real animatedGroundSpeedInMetersPerSecond: (global.navigator().isInFlight) ? positionProvider.positionInfo.groundSpeed().toMPS() : 0.0
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

            anchorPoint.x: imageOP.width/2
            anchorPoint.y: imageOP.height/2
            coordinate: positionProvider.lastValidCoordinate

            Connections {
                // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
                // is not updated when the height of the map changes. It does get updated when the
                // width of the map changes. We use the undocumented method polishAndUpdate() here.
                target: flightMap
                function onHeightChanged() { ownPosition.polishAndUpdate() }
            }

            sourceItem: Item {
                Image {
                    id: imageOP

                    rotation: flightMap.animatedTrack-flightMap.bearing

                    source: {
                        var pInfo = positionProvider.positionInfo

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

            line.width: 3
            line.color: '#008000' //'green'
            path: flightRoute.geoPath
            opacity: (flightMap.zoomLevel < 11.0) ? 1.0 : 0.3
        }

        MapItemView { // Traffic opponents
            model: flarmAdaptor.trafficObjects4QML
            delegate: Component {
                Traffic {
                    trafficInfo: model.modelData
                }
            }
        }

        MapItemView {
            id: midFieldWaypoints
            model: flightRoute.midFieldWaypoints
            delegate: Component {

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
        }

        // Mouse Area, in order to receive mouse clicks
        MouseArea {
            anchors.fill: parent
            propagateComposedEvents: true

            onWheel: {
                flightMap.followGPS = false
                wheel.accepted = false
            }

            onPressAndHold: onDoubleClicked(mouse)

            onDoubleClicked: {
                global.mobileAdaptor().vibrateBrief()
                var wp = geoMapProvider.closestWaypoint(flightMap.toCoordinate(Qt.point(mouse.x,mouse.y)),
                                                        flightMap.toCoordinate(Qt.point(mouse.x+25,mouse.y)),
                                                        flightRoute)
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

        onCopyrightLinkActivated: Qt.openUrlExternally(link)
    }

    BrightnessContrast { // Graphical effects: increase contrast, reduce brightness in dark mode
        anchors.fill: flightMap
        source: flightMap
        brightness: Material.theme == Material.Dark ? -0.9 : -0.2
        contrast: Material.theme == Material.Dark ? 0.6 : 0.2
    }

    Rectangle {
        id: noMapWarningRect

        color: "white"
        anchors.centerIn: parent
        width: parent.width*0.6
        height: noMapWarning.height+20
        border.color: "black"
        visible: !global.mapManager().aviationMaps.hasFile
        Label {
            id: noMapWarning
            anchors.centerIn: parent
            width: parent.width-20
            wrapMode: Text.WordWrap

            text: qsTr("<p><strong>There is no aviation map installed.</strong></p>
<p>In order to install a map, please open the menu using the button ☰ in the upper left corner of this screen.
Choose <strong>Library/Maps</strong> to open the map management page.</p>")
            textFormat: Text.StyledText
            color: "red"
        }
    }

    RoundButton {
        id: northButton

        anchors.horizontalCenter: zoomIn.horizontalCenter
        anchors.top: page.top
        anchors.topMargin: 0.5*Qt.application.font.pixelSize

        height: 66
        width:  66

        icon.source: "/icons/NorthArrow.svg"


        contentItem: Image {
            Layout.alignment: Qt.AlignHCenter
            id: northArrow

            opacity: global.settings().nightMode ? 0.3 : 1.0
            rotation: -flightMap.bearing

            source: "/icons/NorthArrow.svg"
        }

        onClicked: {
            if (global.settings().mapBearingPolicy === GlobalSettings.NUp) {
                global.settings().mapBearingPolicy = GlobalSettings.TTUp
                toast.doToast(qsTr("Map Mode: Track Up"))
            } else {
                global.settings().mapBearingPolicy = GlobalSettings.NUp
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
        anchors.leftMargin: 0.5*Qt.application.font.pixelSize
        anchors.bottom: navBar.top
        anchors.bottomMargin: 1.5*Qt.application.font.pixelSize

        height: 66
        width:  66

        onClicked: {
            global.mobileAdaptor().vibrateBrief()
            flightMap.followGPS = true
            toast.doToast(qsTr("Map Mode: Autopan"))
        }
    }

    RoundButton {
        id: zoomIn

        opacity: 0.9
        icon.source: "/icons/material/ic_add.svg"
        enabled: flightMap.zoomLevel < flightMap.maximumZoomLevel
        autoRepeat: true

        anchors.right: parent.right
        anchors.rightMargin: 0.5*Qt.application.font.pixelSize
        anchors.bottom: zoomOut.top
        anchors.bottomMargin: 0.5*Qt.application.font.pixelSize

        height: 66
        width:  66

        onClicked: {
            centerBindingAnimation.omitAnimationforZoom()
            global.mobileAdaptor().vibrateBrief()
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
        anchors.rightMargin: 0.5*Qt.application.font.pixelSize
        anchors.bottom: navBar.top
        anchors.bottomMargin: 1.5*Qt.application.font.pixelSize

        height: 66
        width:  66

        onClicked: {
            centerBindingAnimation.omitAnimationforZoom()
            global.mobileAdaptor().vibrateBrief()
            flightMap.zoomLevel -= 1
        }
    }

    Scale {
        id: leftScale

        anchors.top: northButton.bottom
        anchors.topMargin: 0.5*Qt.application.font.pixelSize
        anchors.bottom: followGPSButton.top
        anchors.bottomMargin: 0.5*Qt.application.font.pixelSize
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
        anchors.leftMargin: 0.5*Qt.application.font.pixelSize
        anchors.right: zoomIn.left
        anchors.rightMargin: 0.5*Qt.application.font.pixelSize
        anchors.verticalCenter: followGPSButton.verticalCenter

        opacity: Material.theme === Material.Dark ? 0.3 : 1.0
        visible: parent.height > parent.width

        pixelPer10km: flightMap.pixelPer10km
        vertical: false
        height: 30
    }

    Label {
        id: copyrightInfo
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: navBar.top
        anchors.bottomMargin: 0.4*Qt.application.font.pixelSize

        text: geoMapProvider.copyrightNotice
        visible: width < parent.width
        onLinkActivated: Qt.openUrlExternally(link)
    }

    Label {
        id: noCopyrightInfo
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: navBar.top
        anchors.bottomMargin: 0.4*Qt.application.font.pixelSize
        text: "<a href='xx'>"+qsTr("Map Data Copyright Info")+"</a>"
        visible: !copyrightInfo.visible
        onLinkActivated: copyrightDialog.open()

        LongTextDialog {
            id: copyrightDialog
            title: qsTr("Map Data Copyright Information")
            text: geoMapProvider.copyrightNotice.replace("•", "<br><br>").replace("•", "<br><br>").replace("•", "<br><br>")
            standardButtons: Dialog.Cancel
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
    }
}
