/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

import QtPositioning
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../dialogs"
import "../items"

Page {
    id: page
    title: qsTr("Nearby Waypoints")
    focus: true

    header: StandardHeader {}


    TabBar {
        id: bar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right

        currentIndex: sv.currentIndex

        TabButton { text: "AD" }
        TabButton { text: "NAV" }
        TabButton { text: "MET" }
        TabButton { text: "REP" }
        Material.elevation: 3
    }

    SwipeView {
        id: sv

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: SafeInsets.bottom
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right

        clip: true
        currentIndex: bar.currentIndex

        Component {
            id: waypointDelegate

            WaypointDelegate {
                required property var model
                waypoint: model.modelData
            }
        }

        Component {
            id: stationDelegate

            Item {
                width: stationList.width
                height: idel.height

                // Background color according to METAR/FAA flight category
                Rectangle {
                    anchors.fill: parent
                    color: model.modelData.hasMETAR ? model.modelData.metar.flightCategoryColor : "transparent"
                    opacity: 0.2
                }

                WordWrappingItemDelegate {
                    leftPadding: SafeInsets.left+16
                    rightPadding: SafeInsets.right+16

                    id: idel
                    text: {
                        var result = model.modelData.twoLineTitle

                        var wayTo  = Navigator.aircraft.describeWay(PositionProvider.positionInfo.coordinate(), model.modelData.coordinate)
                        if (wayTo !== "")
                            result = result + "<br>" + wayTo

                        if (model.modelData.hasMETAR)
                            result = result + "<br>" + model.modelData.metar.summary

                        return result
                    }
                    icon.source: model.modelData.icon
                    icon.color: "transparent"

                    width: parent.width

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        weatherReport.weatherStation = model.modelData
                        weatherReport.open()
                    }
                }
            }
        }

        ListView {
            id: adList

            clip: true

            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Component.onCompleted: adList.model = GeoMapProvider.nearbyWaypoints(PositionProvider.lastValidCoordinate, "AD")

            Label {
                anchors.fill: parent
                anchors.topMargin: font.pixelSize*2
                visible: parent.count === 0

                horizontalAlignment: Text.AlignHCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: qsTr("<h3>Sorry!</h3><p>No aerodrome data available. Please make sure that an aviation map is installed.</p>")
            }
        }

        ListView {
            id: naList

            clip: true

            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Component.onCompleted: naList.model = GeoMapProvider.nearbyWaypoints(PositionProvider.lastValidCoordinate, "NAV")

            Label {
                anchors.fill: parent
                anchors.topMargin: font.pixelSize*2
                visible: parent.count === 0

                horizontalAlignment: Text.AlignHCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: qsTr("<h3>Sorry!</h3><p>No navaid data available.</p>")
            }
        }

        // List of weather stations
        ListView {
            id: stationList

            clip: true

            model: WeatherDataProvider.weatherStations
            delegate: stationDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Rectangle {  // No data label
                anchors.fill: parent
                color: "white"
                visible: stationList.count == 0

                Text {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top

                    leftPadding: view.font.pixelSize
                    rightPadding: view.font.pixelSize
                    topPadding: 2*view.font.pixelSize

                    horizontalAlignment: Text.AlignHCenter
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                    text: qsTr("<h3>Sorry!</h3><p>No METAR/TAF data available. You can restart the download manually using the item 'Update METAR/TAF' from the three-dot menu at the top right corner of the screen.</p>")
                }
            }

            Rectangle {  // Download in progress label
                id: downloadIndicator

                anchors.fill: parent

                color: "white"
                visible: WeatherDataProvider.downloading && !WeatherDataProvider.backgroundUpdate

                Text {
                    id: downloadIndicatorLabel

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: view.font.pixelSize*2

                    horizontalAlignment: Text.AlignHCenter
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                    text: qsTr("<h3>Download in progress…</h3><p>Please stand by while we download METAR/TAF data from the Aviation Weather Center…</p>")
                } // downloadIndicatorLabel

                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: downloadIndicatorLabel.bottom
                    anchors.topMargin: 10
                }

                // The Connections and the SequentialAnimation here provide a fade-out animation for the downloadindicator.
                // Without this, the downaloadIndication would not be visible on very quick downloads, leaving the user
                // without any feedback if the download did actually take place.
                Connections {
                    target: WeatherDataProvider
                    function onDownloadingChanged () {
                        if (WeatherDataProvider.downloading && !WeatherDataProvider.backgroundUpdate) {
                            downloadIndicator.visible = true
                            downloadIndicator.opacity = 1.0
                        } else
                            fadeOut.start()
                    }
                }
                SequentialAnimation{
                    id: fadeOut
                    NumberAnimation { target: downloadIndicator; property: "opacity"; to:1.0; duration: 400 }
                    NumberAnimation { target: downloadIndicator; property: "opacity"; to:0.0; duration: 400 }
                    NumberAnimation { target: downloadIndicator; property: "visible"; to:1.0; duration: 20}
                }
            }


            // Refresh METAR/TAF data on overscroll
            property int refreshFlick: 0
            onFlickStarted: {
                refreshFlick = atYBeginning
            }

            onFlickEnded: {
                if ( atYBeginning && refreshFlick ) {
                    PlatformAdaptor.vibrateBrief()
                    console.warn("XX")
                    WeatherDataProvider.update(false)
                }
            }

            // Try and update METAR/TAF as soon as someone opens this page if the current list of stations
            // is empty. This is not a background update, we want user interaction.
            Component.onCompleted: {
                if (stationList.count === 0)
                    WeatherDataProvider.update(false)
                else
                    WeatherDataProvider.update(true)
            }

            // Show error when weather cannot be updated -- but not if we are running a background upate
            Connections {
                target: WeatherDataProvider
                function onError (message) {
                    if (WeatherDataProvider.backgroundUpdate)
                        return
                    dialogLoader.active = false
                    dialogLoader.title = qsTr("Update Error")
                    dialogLoader.text = qsTr("<p>Failed to update the list of stations.</p><p>Reason: %1.</p>").arg(message)
                    dialogLoader.source = "dialogs/ErrorDialog.qml"
                    dialogLoader.active = true
                }
            }

        }

        ListView {
            id: rpList

            clip: true

            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Component.onCompleted: rpList.model = global.geoMapProvider().nearbyWaypoints(PositionProvider.lastValidCoordinate, "WP")
            
            Label {
                anchors.fill: parent
                anchors.topMargin: font.pixelSize*2
                visible: parent.count === 0

                horizontalAlignment: Text.AlignHCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: qsTr("<h3>Sorry!</h3><p>No reporting point data available.</p>")
            }
        }

    } // SwipeView

    // Manual update button in footer
    footer: Pane {
        width: parent.width
        bottomPadding: SafeInsets.bottom+16
        leftPadding: SafeInsets.left+16
        rightPadding: SafeInsets.right+16

        Material.elevation: 3
        visible: (sunLabel.text !== "") || (qnhLabel.text !== "")

        GridLayout {
            anchors.fill: parent
            columns: 2

            Icon {
                visible: qnhLabel.text !== ""
                source: "/icons/material/ic_speed.svg"
            }
            Label {
                id: qnhLabel
                visible: qnhLabel.text !== ""
                Layout.fillWidth: true
                text: WeatherDataProvider.QNHInfo
            }

            Icon {
                visible: sunLabel.text !== ""
                source: "/icons/material/ic_wb_sunny.svg"
            }
            Label {
                id: sunLabel
                visible: sunLabel.text !== ""
                Layout.fillWidth: true
                text: WeatherDataProvider.sunInfo
            }

        }

    }

    WeatherReport {
        id: weatherReport
        objectName: "weatherReport"
    }
} // Page
