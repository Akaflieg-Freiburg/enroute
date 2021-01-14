/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

import QtLocation 5.15
import QtPositioning 5.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import enroute 1.0

import QtQml 2.15

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
        property real animatedTrack: satNav.lastValidTrack
        Behavior on animatedTrack { RotationAnimation {duration: 400; direction: RotationAnimation.Shortest } }


        // GESTURES

        // Enable gestures. Make sure that whenever a gesture starts, the property "followGPS" is set to "false"
        gesture.enabled: true
        gesture.acceptedGestures: MapGestureArea.PanGesture|MapGestureArea.PinchGesture|MapGestureArea.RotationGesture
        gesture.onPanStarted: {flightMap.followGPS = false}
        gesture.onPinchStarted: {flightMap.followGPS = false}
        gesture.onRotationStarted: {
            flightMap.followGPS = false
            globalSettings.mapBearingPolicy = GlobalSettings.UserDefinedBearingUp
        }


        //
        // PROPERTY "bearing"
        //

        // Initially, set the bearing to the last saved value
        bearing: savedBearing

        // If "followGPS" is true, then update the map bearing whenever a new GPS position comes in
        Binding on bearing {
            restoreMode: Binding.RestoreBinding
            when: globalSettings.mapBearingPolicy !== GlobalSettings.UserDefinedBearingUp
            value: globalSettings.mapBearingPolicy === GlobalSettings.TTUp ? satNav.lastValidTrack : 0
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
                if (!satNav.isInFlight)
                    return satNav.lastValidCoordinate

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

                return satNav.lastValidCoordinate.atDistanceAndAzimuth(radiusInM, satNav.lastValidTrack)
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
        MapQuickItem {
            id: fiveMinuteBar

            anchorPoint.x: fiveMinuteBarBaseRect.width/2
            anchorPoint.y: fiveMinuteBarBaseRect.height
            coordinate: satNav.lastValidCoordinate
            visible: (!globalSettings.autoFlightDetection || satNav.isInFlight) && (satNav.track >= 0)

            sourceItem: Item{
                Rectangle {
                    id: fiveMinuteBarBaseRect

                    property real animatedGroundSpeedInMetersPerSecond: (!globalSettings.autoFlightDetection || satNav.isInFlight) ?
                                                                            satNav.groundSpeedInMetersPerSecond : 0.0
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

            anchorPoint.x: image.width/2
            anchorPoint.y: image.height/2
            coordinate: satNav.lastValidCoordinate

            sourceItem: Item{
                Image {
                    id: image

                    rotation: flightMap.animatedTrack-flightMap.bearing

                    source:  satNav.icon
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

        MapItemView {
            id: midFieldWaypoints
            model: flightRoute.midFieldWaypoints
            delegate: Component {

                MapQuickItem {

                    anchorPoint.x: image.width/2
                    anchorPoint.y: image.height
                    coordinate: model.modelData.coordinate

                    sourceItem: Item{
                        Image {
                            id: image

                            source:  "/icons/waypoints/WP.svg"
                            sourceSize.width: 20
                            sourceSize.height: 20
                        }
                        Label {
                            anchors.left: image.right
                            text: model.modelData.extendedName
                            visible: flightMap.zoomLevel > 11.0
                            background: Rectangle {
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
                mobileAdaptor.vibrateBrief()
                waypointDescription.waypoint = geoMapProvider.closestWaypoint(flightMap.toCoordinate(Qt.point(mouse.x,mouse.y)),
                                                                              flightMap.toCoordinate(Qt.point(mouse.x+25,mouse.y)),
                                                                              flightRoute)
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

    Rectangle {
        id: noMapWarningRect

        color: "white"
        anchors.centerIn: parent
        width: parent.width*0.6
        height: noMapWarning.height+20
        border.color: "black"
        visible: !mapManager.aviationMaps.hasFile
        Label {
            id: noMapWarning
            anchors.centerIn: parent
            width: parent.width-20
            wrapMode: Text.WordWrap

            text: qsTr("<p><strong>There is no aviation map installed.</strong></p><p>Please open the menu and go to <strong>Settings/Library/Maps</strong>.</p>")
            textFormat: Text.RichText
            color: "red"
        }
    }

    Button {
        id: northButton

        anchors.horizontalCenter: zoomIn.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 0.5*Qt.application.font.pixelSize

        contentItem: ColumnLayout {
            Image {
                Layout.alignment: Qt.AlignHCenter
                id: northArrow

                rotation: -flightMap.bearing

                source: "/icons/NorthArrow.svg"
                sourceSize.width: 44
                sourceSize.height: 44
            }
            Label {
                Layout.alignment: Qt.AlignHCenter
                text: {
                    if (globalSettings.mapBearingPolicy === GlobalSettings.TTUp)
                        return "TT ↑"
                    if (globalSettings.mapBearingPolicy === GlobalSettings.NUp)
                        return "N ↑"
                    return Math.round(flightMap.bearing)+"° ↑"
                }

            }
        }

        onClicked: {
            if (globalSettings.mapBearingPolicy === GlobalSettings.NUp)
                globalSettings.mapBearingPolicy = GlobalSettings.TTUp
            else
                globalSettings.mapBearingPolicy = GlobalSettings.NUp
        }
    }

    Button {
        id: followGPSButton

        opacity: 0.9
        icon.source: "/icons/material/ic_my_location.svg"
        visible: !flightMap.followGPS

        anchors.left: parent.left
        anchors.leftMargin: 0.5*Qt.application.font.pixelSize
        anchors.bottom: navBar.top
        anchors.bottomMargin: 1.5*Qt.application.font.pixelSize

        onClicked: {
            mobileAdaptor.vibrateBrief()
            flightMap.followGPS = true
        }
    }

    Button {
        id: zoomIn

        visible: flightMap.zoomLevel < flightMap.maximumZoomLevel
        autoRepeat: true

        anchors.right: parent.right
        anchors.rightMargin: 0.5*Qt.application.font.pixelSize
        anchors.bottom: zoomOut.top
        anchors.bottomMargin: 0.5*Qt.application.font.pixelSize

        contentItem: Label {
            text: "+"
            font.bold: true
            font.pixelSize: Qt.application.font.pixelSize*1.2
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        onClicked: {
            centerBindingAnimation.omitAnimationforZoom()
            mobileAdaptor.vibrateBrief()
            flightMap.zoomLevel += 1
        }
    }

    Button {
        id: zoomOut

        visible: flightMap.zoomLevel > flightMap.minimumZoomLevel
        autoRepeat: true

        anchors.right: parent.right
        anchors.rightMargin: 0.5*Qt.application.font.pixelSize
        anchors.bottom: navBar.top
        anchors.bottomMargin: 1.5*Qt.application.font.pixelSize

        contentItem: Label {
            text: "-"
            font.bold: true
            font.pixelSize: Qt.application.font.pixelSize*1.2
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
        }

        onClicked: {
            centerBindingAnimation.omitAnimationforZoom()
            mobileAdaptor.vibrateBrief()
            flightMap.zoomLevel -= 1
        }
    }

    Scale {
        id: scale

        anchors.left: followGPSButton.right
        anchors.leftMargin: 0.5*Qt.application.font.pixelSize
        anchors.right: zoomIn.left
        anchors.rightMargin: 0.5*Qt.application.font.pixelSize
        anchors.verticalCenter: followGPSButton.verticalCenter

        useMetricUnits: globalSettings.useMetricUnits
        pixelPer10km: flightMap.pixelPer10km
        height: 30
    }

    Label {
        id: copyrightInfo
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: navBar.top
        anchors.bottomMargin: 0.4*Qt.application.font.pixelSize
        text: geoMapProvider.copyrightNotice
        linkColor: "blue"
        visible: width < parent.width
        onLinkActivated: Qt.openUrlExternally(link)
    }

    Label {
        id: noCopyrightInfo
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: navBar.top
        anchors.bottomMargin: 0.4*Qt.application.font.pixelSize
        text: "<a href='xx'>"+qsTr("Map Data Copyright Info")+"</a>"
        linkColor: "blue"
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

        y: (!globalSettings.autoFlightDetection || satNav.isInFlight) ? view.height - height : view.height
    }

    WaypointDescription {
        id: waypointDescription
    }
}
