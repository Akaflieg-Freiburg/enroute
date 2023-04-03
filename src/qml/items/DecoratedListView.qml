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


// This is a version of ListView that indicates if more elemenent can be seen through scrolling

ListView {
    id: listView

    Control { id: fontGlean }

    Rectangle { // Label "more" at top of listview
        opacity: parent.atYBeginning ? 0.0 : 1.0
        Behavior on opacity { NumberAnimation { duration: 200 } }

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        height: 2*topLabel.implicitHeight

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#ffffffff" }
            GradientStop { position: 0.3; color: "#ffffffff" }
            GradientStop { position: 1.0; color: "#00ffffff" }
        }

        Label {
            id: topLabel

            anchors.fill: parent

            font.pixelSize: 0.8*fontGlean.font.pixelSize
            text: "▲ " + qsTr("more") + " ▲"

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignTop
        }
    }

    Rectangle { // Label "more" at bottom of listview
        opacity: parent.atYEnd ? 0.0 : 1.0
        Behavior on opacity { NumberAnimation { duration: 200 } }

        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        height: 2*bottomLabel.implicitHeight

        gradient: Gradient {
            GradientStop { position: 0.0; color: "#00ffffff" }
            GradientStop { position: 0.7; color: "#ffffffff" }
            GradientStop { position: 1.0; color: "#ffffffff" }
        }

        Label {
            id: bottomLabel

            anchors.fill: parent

            font.pixelSize: 0.8*fontGlean.font.pixelSize
            color: "#202020"
            text: "▼ " + qsTr("more") + " ▼"

            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignBottom
        }
    }

    ScrollIndicator.vertical: ScrollIndicator {}
}
