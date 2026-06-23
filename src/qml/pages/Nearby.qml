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

    // Control that receives active focus when this page becomes current in the
    // StackView. The navigation layer (main.qml) reads this and focuses it on
    // push and after the drawer closes; pages that don't set it are unaffected.
    // SwipeView is a FocusScope, so focusing it delegates active focus to
    // whichever child currently has focus:true — and that is bound to the
    // visible tab, so focus automatically follows the tab (see bindings below).
    property Item defaultFocusItem: sv

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
        TabButton { text: "WP" }
        TabButton { text: "NAV" }
        TabButton { icon.source: "/icons/material/ic_search.svg" }
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

            // Only the visible tab's control claims focus within contentScope.
            focus: SwipeView.isCurrentItem

            delegate: waypointDelegate

            Component.onCompleted: adList.model = GeoMapProvider.nearbyWaypoints(PositionProvider.lastValidCoordinate, "AD")

            Label {
                anchors.fill: parent
                anchors.topMargin: font.pixelSize*2
                visible: parent.count === 0

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: qsTr("<h3>Sorry!</h3><p>No aerodrome data available. Please make sure that an aviation map is installed.</p>")
            }
        }

        DecoratedListView {
            id: wpList

            clip: true

            focus: SwipeView.isCurrentItem

            delegate: waypointDelegate

            Component.onCompleted: wpList.model = GeoMapProvider.nearbyWaypoints(PositionProvider.lastValidCoordinate, "WP")

            Label {
                anchors.fill: parent
                anchors.topMargin: font.pixelSize*2
                visible: parent.count === 0

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: qsTr("<h3>Sorry!</h3><p>No waypoints available.</p>")
            }
        }

        DecoratedListView {
            id: naList

            clip: true

            focus: SwipeView.isCurrentItem

            delegate: waypointDelegate

            Component.onCompleted: naList.model = GeoMapProvider.nearbyWaypoints(PositionProvider.lastValidCoordinate, "NAV")

            Label {
                anchors.fill: parent
                anchors.topMargin: font.pixelSize*2
                visible: parent.count === 0

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                text: qsTr("<h3>Sorry!</h3><p>No navaid data available.</p>")
            }
        }

        ColumnLayout {
            id: searchTab

            Item {
                Layout.preferredHeight: textInput.font.pixelSize/4.0
            }

            MyTextField {
                id: textInput

                Layout.fillWidth: true
                Layout.leftMargin: font.pixelSize/2.0
                Layout.rightMargin: font.pixelSize/2.0

                // Search tab focuses the filter field, not its result list.
                focus: searchTab.SwipeView.isCurrentItem

                placeholderText: qsTr("Filter by Name")
            }

            DecoratedListView {
                id: naList2

                Layout.fillHeight: true
                Layout.fillWidth: true

                clip: true

                // The filter field owns focus on this tab, so the list opts out
                // (overrides DecoratedListView's default focus:true).
                focus: false

                delegate: waypointDelegate

                Binding {
                    naList2.model: GeoMapProvider.filteredWaypoints(textInput.displayText)
                    delayed: true
                }

                Label {
                    anchors.fill: parent
                    anchors.topMargin: font.pixelSize*2
                    visible: parent.count === 0

                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                    text: qsTr("<h3>Sorry!</h3><p>No waypoints match your filter.</p>")
                }
            }
        }
    } // SwipeView


} // Page
