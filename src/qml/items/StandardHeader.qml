/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15

ToolBar {

    Material.foreground: "white"
    height: 60

    ToolButton {
        id: backButton

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        visible: Qt.platform.os !== "android"
        width: Qt.platform.os !== "android" ? undefined : 0

        icon.source: "/icons/material/ic_arrow_back.svg"

        onClicked: {
            global.mobileAdaptor().vibrateBrief()
            stackView.pop()
        }
    }

    Label {
        id: lbl

        anchors.verticalCenter: parent.verticalCenter

        anchors.left: parent.left
        anchors.leftMargin: Qt.platform.os !== "android" ? 72 : 16
        anchors.right: parent.right

        text: stackView.currentItem.title
        elide: Label.ElideRight
        font.pixelSize: 20
        verticalAlignment: Qt.AlignVCenter
    }

} // ToolBar
