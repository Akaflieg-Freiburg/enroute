/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../dialogs"
import "../items"

Page {
    id: pg

    required property var dialogLoader
    required property var stackView

    title: qsTr("Visual Approach Charts")

    Component {
        id: approachChartItem

        WordWrappingItemDelegate {
            width: parent ? parent.width : undefined

            required property var model

            text: model.modelData.name
            icon.source: "/icons/material/ic_map.svg"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                Global.currentVAC = model.modelData
                stackView.pop()
            }
        }
    }


    header: PageHeader {

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
                pg.stackView.pop()
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: pg.title
            elide: Label.ElideRight
            font.pixelSize: 20
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_info_outline.svg"
            onClicked: {
                PlatformAdaptor.vibrateBrief()
                helpDialog.open()
            }
        }
    }

    DecoratedListView {
        anchors.fill: parent
        anchors.bottomMargin: SafeInsets.bottom

        clip: true
        model: {
            // Mention downloadable in order to get updates
            VACLibrary.vacs

            return VACLibrary.vacsByDistance(PositionProvider.lastValidCoordinate)
        }
        delegate: approachChartItem
        ScrollIndicator.vertical: ScrollIndicator {}
    }

    Label {
        anchors.fill: parent
        anchors.bottomMargin: font.pixelSize
        anchors.leftMargin: font.pixelSize
        anchors.rightMargin: font.pixelSize
        anchors.topMargin: font.pixelSize

        background: Rectangle {color: "white"}
        visible: VACLibrary.isEmpty

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment : Text.AlignVCenter
        textFormat: Text.RichText
        wrapMode: Text.Wrap

        text: "<h3>"+ qsTr("Sorry!") + "</h3><p>" + qsTr("There are no approach charts installed. The <a href='x'>manual</a> explains how to install and use them.")+"</p>"
        onLinkActivated: openManual("03-tutorialAdvanced/04-vac.html")

    }

    LongTextDialog {
        id: helpDialog

        title: qsTr("Visual Approach Charts")
        text: "<p>"+qsTr("This page presents the visual approach charts that are installed in your system, sorted by distance to the current position. Click on an entry to open a moving map that includes the selected VAC.")+"</p>"
              +"<p>"+qsTr("In order to manage your collection of visual approach charts, go back to the main map view, open the main menu and go to 'Library/Maps and Data'.")+"</p>"

        standardButtons: Dialog.Ok
    }

} // Page

