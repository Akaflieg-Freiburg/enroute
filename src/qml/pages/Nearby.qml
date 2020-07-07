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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14

import enroute 1.0
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

        currentIndex: sv.currentIndex

        TabButton { text: qsTr("AD") }
        TabButton { text: qsTr("NAV") }
        TabButton { text: qsTr("REP") }
        Material.elevation: 3
    }

    SwipeView {
        id: sv

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        currentIndex: bar.currentIndex

        Component {
            id: waypointDelegate

            ItemDelegate {
                id: idel
                text: model.modelData.richTextName +
                    (satNav.status == SatNav.OK ?
                        ("<br>" + model.modelData.wayFrom(satNav.lastValidCoordinate,
                                                            globalSettings.useMetricUnits)) :
                        "")
                icon.source: "/icons/waypoints/"+model.modelData.get("CAT")+".svg"
                icon.color: "transparent"

                width: sv.width

                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    dialogLoader.active = false
                    dialogLoader.dialogArgs = {waypoint: model.modelData}
                    dialogLoader.text = ""
                    dialogLoader.source = "../dialogs/WaypointDescription.qml"
                    dialogLoader.active = true
                }
            }
        }

        Component {
            id: noData

            Rectangle {
                color: "white"

                Label {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.top: parent.top
                    anchors.topMargin: Qt.application.font.pixelSize*2

                    horizontalAlignment: Text.AlignHCenter
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    text: qsTr("<h3>Sorry!</h3><p>No data available. Please make sure that an aviation map is installed.</p>")
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }

        ListView {
            id: adList

            clip: true

            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Component.onCompleted: adList.model = geoMapProvider.nearbyWaypoints(satNav.lastValidCoordinate, "AD")

            Loader {
                anchors.fill: parent
                active: parent.count == 0
                sourceComponent : noData
            }
        }

        ListView {
            id: naList

            clip: true

            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Component.onCompleted: naList.model = geoMapProvider.nearbyWaypoints(satNav.lastValidCoordinate, "NAV")

            Loader {
                anchors.fill: parent
                active: parent.count == 0
                sourceComponent : noData
            }
        }

        ListView {
            id: rpList

            clip: true

            delegate: waypointDelegate
            ScrollIndicator.vertical: ScrollIndicator {}

            Component.onCompleted: rpList.model = geoMapProvider.nearbyWaypoints(satNav.lastValidCoordinate, "WP")
            
            Loader {
                anchors.fill: parent
                active: parent.count == 0
                sourceComponent : noData
            }
        }

    } // SwipeView

} // Page
