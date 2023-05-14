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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

import akaflieg_freiburg.enroute


// This is a version of ListView that indicates if more elemenent can be seen through scrolling.
// The method "Component.onCompleted" adds two children to the ScrollView. This way,
// the children are not scrolling, but shown on top of the ScrollView.

ScrollView {
    id: scrollView

    Component {
        id: topItem

        Label {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            background: Rectangle {
                color: GlobalSettings.nightMode ? "black" : "white"
                opacity: 0.8
            }

            opacity: (scrollView.contentItem.contentY === 0) ? 0.0 : 1.0
            Behavior on opacity { NumberAnimation { duration: 200 } }

            font.pixelSize: 0.8*scrollView.font.pixelSize
            text: "▲ " + qsTr("more") + " ▲"

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
        }
    }

    Component {
        id: bottomItem

        Label {
            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            background: Rectangle {
                color: GlobalSettings.nightMode ? "black" : "white"
                opacity: 0.8
            }

            opacity: (scrollView.contentItem.contentY >= scrollView.contentHeight-scrollView.height ) ? 0.0 : 1.0
            Behavior on opacity { NumberAnimation { duration: 200 } }

            font.pixelSize: 0.8*scrollView.font.pixelSize
            text: "▼ " + qsTr("more") + " ▼"

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignBottom
        }
    }

    Component.onCompleted: {
        topItem.createObject(scrollView)
        bottomItem.createObject(scrollView)
    }


}
