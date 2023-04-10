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

        Rectangle { // Label "more" at top of listview
            opacity: (scrollView.contentItem.contentY === 0) ? 0.0 : 1.0
            Behavior on opacity { NumberAnimation { duration: 200 } }

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right

            height: 2*topLabel.implicitHeight

            gradient: Gradient {
                GradientStop { position: 0.0; color: GlobalSettings.nightMode ? "#ff000000" : "#ffffffff" }
                GradientStop { position: 0.3; color: GlobalSettings.nightMode ? "#ff000000" : "#ffffffff" }
                GradientStop { position: 1.0; color: GlobalSettings.nightMode ? "#00000000" : "#00ffffff" }
            }

            Control { id: topFontGlean }

            Label {
                id: topLabel

                anchors.fill: parent

                font.pixelSize: 0.8*topFontGlean.font.pixelSize
                text: "▲ " + qsTr("more") + " ▲"
                color: GlobalSettings.nightMode ? "white" : "black"

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignTop
            }
        }
    }

    Component {
        id: bottomItem

        Rectangle { // Label "more" at bottom of listview
            opacity: (scrollView.contentItem.contentY >= scrollView.contentHeight-scrollView.height ) ? 0.0 : 1.0
            Behavior on opacity { NumberAnimation { duration: 200 } }

            anchors.bottom: parent.bottom
            anchors.left: parent.left
            anchors.right: parent.right

            height: 2*bottomLabel.implicitHeight

            gradient: Gradient {
                GradientStop { position: 0.0; color: GlobalSettings.nightMode ? "#00000000" : "#00ffffff" }
                GradientStop { position: 0.7; color: GlobalSettings.nightMode ? "#ff000000" : "#ffffffff" }
                GradientStop { position: 1.0; color: GlobalSettings.nightMode ? "#ff000000" : "#ffffffff" }
            }

            Control { id: bottomFontGlean }

            Label {
                id: bottomLabel

                anchors.fill: parent

                font.pixelSize: 0.8*bottomFontGlean.font.pixelSize
                text: "▼ " + qsTr("more") + " ▼"
                color: GlobalSettings.nightMode ? "white" : "black"

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignBottom
            }
        }
    }


    Component.onCompleted: {
        topItem.createObject(scrollView)
        bottomItem.createObject(scrollView)
    }


}
