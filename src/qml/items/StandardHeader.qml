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

ColoredToolBar {

    height: 60 + SafeInsets.top
    leftPadding: SafeInsets.left
    rightPadding: SafeInsets.right
    topPadding: SafeInsets.top

    ToolButton {
        id: backButton

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter

        icon.source: "/icons/material/ic_arrow_back.svg"

        onClicked: {
            PlatformAdaptor.vibrateBrief()
            stackView.pop()
        }
    }

    Label {
        id: lbl

        anchors.verticalCenter: parent.verticalCenter

        anchors.left: parent.left
        anchors.leftMargin: 72
        anchors.right: parent.right

        text: stackView.currentItem.title
        elide: Label.ElideRight
        font.pixelSize: 20
        verticalAlignment: Qt.AlignVCenter
    }

} // ToolBar
