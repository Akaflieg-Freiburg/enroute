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

import QtLocation 5.14
import QtPositioning 5.14
import QtQuick 2.14
import QtQuick.Controls 2.14

import enroute 1.0

import QtQml 2.14

import ".."

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

        property bool followGPS: true
        property real animatedTrack: satNav.lastValidTrack
        Behavior on animatedTrack { RotationAnimation {duration: 400; direction: RotationAnimation.Shortest } }


        // GESTURES

        // Enable gestures. Make sure that whenever a gesture starts, the property "followGPS" is set to "false"
        gesture.enabled: true
        gesture.acceptedGestures: MapGestureArea.PanGesture|MapGestureArea.PinchGesture|MapGestureArea.RotationGesture
        gesture.onPanStarted: {flightMap.followGPS = false}
        gesture.onRotationStarted: {flightMap.followGPS = false}


        // PROPERTY "bearing"

        // Initially, set the bearing to the last saved value
        bearing: savedBearing

        // If "followGPS" is true, then update the map bearing whenever a new GPS position comes in
        Connections {
            id: trackChangedConnection

            target: satNav
            function onLastValidTrackChanged() {
                if (flightMap.followGPS === true) {
                    if (!globalSettings.autoFlightDetection || satNav.isInFlight) {
                        flightMap.bearing = satNav.track
                    } else {
                        flightMap.bearing = 0.0
                    }
                }
            }
        }

        // We expect GPS updates every second. So, we choose an animation of duration 1000ms here, to obtain a flowing movement
        Behavior on bearing { RotationAnimation {duration: 1000; direction: RotationAnimation.Shortest } }


        // PROPERTY "center"

        // If "followGPS" is true, then update the map center whenever a new GPS position comes in
        Binding on center {
            id: centerBinding

            restoreMode: Binding.RestoreBinding
            when: flightMap.followGPS === true
            value: satNav.lastValidCoordinate
        }

        // We expect GPS updates every second. So, we choose an animation of duration 1000ms here, to obtain a flowing movement
        Behavior on center { CoordinateAnimation { duration: 1000 } }

        // PROPERTY "zoomLevel"

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
            line.color: 'green'
            path: flightRoute.geoPath
            opacity: (flightMap.zoomLevel < 11.0) ? 1.0 : 0.3
        }

        // Mouse Area, in order to receive mouse clicks
        MouseArea {
            anchors.fill: parent

            onPressAndHold: onDoubleClicked(mouse)

            onDoubleClicked: {
                mobileAdaptor.vibrateBrief()
                dialogLoader.active = false
                dialogLoader.dialogArgs = {waypoint: geoMapProvider.closestWaypoint(
                                                         flightMap.toCoordinate(Qt.point(mouse.x,mouse.y)),
                                                         flightMap.toCoordinate(Qt.point(mouse.x+25,mouse.y)))}
                dialogLoader.text = ""
                dialogLoader.source = "../dialogs/WaypointDescription.qml"
                dialogLoader.active = true
            }
        }

        // Oddly, this is necessary, or else the system will try to reset
        // the write-once property 'plugin' on language changes
        Component.onCompleted: plugin = mapPlugin
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

    Image {
        id: northArrow

        anchors.horizontalCenter: zoomIn.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: 0.5*Qt.application.font.pixelSize

        rotation: -flightMap.bearing

        source: "/icons/NorthArrow.svg"
        sourceSize.width: 44
        sourceSize.height: 44
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
            trackChangedConnection.onLastValidTrackChanged()
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

    NavBar {
        id: navBar

        anchors.right: parent.right
        anchors.left: parent.left

        y: (!globalSettings.autoFlightDetection || satNav.isInFlight) ? view.height - height : view.height

        Behavior on y { PropertyAnimation {duration: 400 } }
    }
}
