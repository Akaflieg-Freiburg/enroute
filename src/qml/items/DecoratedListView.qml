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
        visible: listView.activeFocus
        border.color: "black"
        color: "#10000000"
        radius: 5
    }
    highlightMoveDuration: 200

    focus: true

    Shortcut {
        sequences: [StandardKey.MoveToEndOfDocument]
        onActivated: listView.currentIndex = listView.count-1
    }
    Shortcut {
        sequences: [StandardKey.MoveToStartOfDocument]
        onActivated: listView.currentIndex = 0
    }
    Shortcut {
        sequences: [StandardKey.MoveToNextPage]
        onActivated: listView.currentIndex = Math.min(listView.count-1, listView.currentIndex + 5)
    }
    Shortcut {
        sequences: [StandardKey.MoveToPreviousPage]
        onActivated: listView.currentIndex = Math.max(0, listView.currentIndex - 5)
    }

    ScrollIndicator.vertical: ScrollIndicator {}
}
