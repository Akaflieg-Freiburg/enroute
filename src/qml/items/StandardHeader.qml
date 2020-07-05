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

ToolBar {

    ToolButton {
        id: backButton

        anchors.left: parent.left
        anchors.leftMargin: drawer.dragMargin

        icon.source: "/icons/material/ic_arrow_back.svg"

        onClicked: {
            mobileAdaptor.vibrateBrief()
            stackView.pop()
        }

        // Oddly, this seems necessary, or else the color will change
        // on language changes
        Component.onCompleted: icon.color = Material.foreground
    } // ToolButton

    Label {
        anchors.left: backButton.right
        anchors.right: backButton2.left
        anchors.bottom: parent.bottom
        anchors.top: parent.top

        text: stackView.currentItem.title
        elide: Label.ElideRight
        font.bold: true
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter

        // Oddly, this seems necessary, or else the color will change
        // on language changes
        Component.onCompleted: color = Material.foreground
    } // Label

    ToolButton {
        // Invisible element for horizontal centering of the label
        id: backButton2

        anchors.right: parent.right
        anchors.rightMargin: drawer.dragMargin

        icon.source: "/icons/material/ic_arrow_back.svg"
        visible: false

        // Oddly, this seems necessary, or else the color will change
        // on language changes
        Component.onCompleted: icon.color = Material.foreground
    } // ToolButton

} // ToolBar
