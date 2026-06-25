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

import QtQuick
import QtQuick.Controls

import akaflieg_freiburg.enroute

pragma ComponentBehavior: Bound


// This is a version of ListView that indicates if more elemenent can be seen through scrolling

ListView {
    id: listView

    // Desktop platforms get a mouse-interactive scroll bar and a focus highlight;
    // touch platforms get a thin, non-interactive scroll indicator.
    readonly property bool isDesktop: (Qt.platform.os !== "android") && (Qt.platform.os !== "ios")

    Control { id: fontGlean }

    Label {
        anchors.left: listView.left
        anchors.leftMargin: 4*font.pixelSize
        anchors.right: listView.right
        anchors.rightMargin: 4*font.pixelSize
        anchors.top: listView.top

        background: Rectangle {
            color: GlobalSettings.nightMode ? "black" : "white"
            opacity: 0.8
        }

        opacity: listView.atYBeginning ? 0.0 : 1.0
        Behavior on opacity { NumberAnimation { duration: 200 } }

        font.pixelSize: 0.8*fontGlean.font.pixelSize
        text: "▲ " + qsTr("more") + " ▲"

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignTop
    }

    Label {
        anchors.bottom: listView.bottom
        anchors.left: listView.left
        anchors.leftMargin: 4*font.pixelSize
        anchors.right: listView.right
        anchors.rightMargin: 4*font.pixelSize

        background: Rectangle {
            color: GlobalSettings.nightMode ? "black" : "white"
            opacity: 0.8
        }

        opacity: parent.atYEnd ? 0.0 : 1.0
        Behavior on opacity { NumberAnimation { duration: 200 } }

        font.pixelSize: 0.8*fontGlean.font.pixelSize
        text: "▼ " + qsTr("more") + " ▼"

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignBottom
    }

    highlight: Rectangle {
        visible: listView.activeFocus && listView.isDesktop
        border.color: "black"
        color: "#10000000"
        radius: 5
    }
    highlightMoveDuration: 200

    focus: true

    // Return/Enter act like a click on the highlighted item. Delegates opt in by
    // exposing a clicked() signal (e.g. WaypointDelegate); lists whose delegates
    // do not have one are unaffected and let the key event propagate.
    Keys.onReturnPressed: (event) => listView.activateCurrentItem(event)
    Keys.onEnterPressed: (event) => listView.activateCurrentItem(event)

    function activateCurrentItem(event) {
        if (listView.currentItem && listView.currentItem.clicked) {
            listView.currentItem.clicked()
            event.accepted = true
        }
    }

    // Home/End/PageUp/PageDown navigation. Implemented as focus-scoped Keys
    // handlers rather than Shortcuts: several DecoratedListViews can be alive at
    // once (e.g. the tabs on the Nearby page), and identical window-context
    // Shortcuts collide as "ambiguous" and stop firing. Keys reach only the
    // focused list, so there is no collision.
    Keys.onPressed: (event) => {
        switch (event.key) {
        case Qt.Key_Home:
            listView.currentIndex = 0
            event.accepted = true
            break
        case Qt.Key_End:
            listView.currentIndex = listView.count-1
            event.accepted = true
            break
        case Qt.Key_PageUp:
            listView.currentIndex = Math.max(0, listView.currentIndex - 5)
            event.accepted = true
            break
        case Qt.Key_PageDown:
            listView.currentIndex = Math.min(listView.count-1, listView.currentIndex + 5)
            event.accepted = true
            break
        }
    }

    // Single source of truth for the vertical scroll affordance — lists using
    // DecoratedListView should NOT set their own. A mouse-interactive ScrollBar
    // on desktop (drag/click to scroll), the thin ScrollIndicator on touch.
    ScrollBar.vertical: ScrollBar {
        policy: ScrollBar.AsNeeded
        visible: listView.isDesktop
    }
    ScrollIndicator.vertical: ScrollIndicator {
        visible: !listView.isDesktop
    }
}
