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

import QtPositioning
import QtQuick
import QtQuick.Controls
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
        TabButton { text: "REP" }
//        Material.elevation: 3
    }

    SwipeView {
        id: sv

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
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

        DecoratedListView {
            id: adList

            clip: true

            delegate: waypointDelegate

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

        DecoratedListView {
            id: naList

            clip: true

            delegate: waypointDelegate

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

        DecoratedListView {
            id: rpList

            clip: true

            delegate: waypointDelegate

            Component.onCompleted: rpList.model = GeoMapProvider.nearbyWaypoints(PositionProvider.lastValidCoordinate, "WP")
            
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
    }

    footer: Footer {
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

} // Page
