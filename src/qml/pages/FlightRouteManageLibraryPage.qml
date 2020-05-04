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
import QtQuick.Layouts 1.14

import "../items"

Page {
    id: page
    title: qsTr("Flight Route Library")
    focus: true

    header: StandardHeader {}

    TextField {
        id: textInput

        anchors.right: parent.right
        anchors.rightMargin: Qt.application.font.pixelSize*2.0
        anchors.left: parent.left
        anchors.leftMargin: Qt.application.font.pixelSize*2.0

        placeholderText: qsTr("Filter Flight Route Names")
        font.pixelSize: Qt.application.font.pixelSize*1.5
    }

    Component {
        id: flightRouteDelegate

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            Layout.fillWidth: true
            height: iDel.heigt

            ItemDelegate {
                id: iDel
                Layout.fillWidth: true

                text: modelData
                icon.source: "/icons/material/ic_directions.svg"
                icon.color: "transparent"
            }

            ToolButton {
                id: cptMenuButton

                icon.source: "/icons/material/ic_more_horiz.svg"

                onClicked: {
                    MobileAdaptor.vibrateBrief()
                    cptMenu.popup()
                }

                AutoSizingMenu {
                    id: cptMenu

                    Action {
                        id: removeAction

                        text: qsTr("Remove from device")
                        icon.source: "/icons/material/ic_delete.svg"

                        onTriggered: {
                            MobileAdaptor.vibrateBrief()
                        }
                    }
                }

            } // ToolButton

        }

    }

    ListView {
        id: wpList
        anchors.top: textInput.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        clip: true

        model: librarian.flightRoutes(textInput.text)
        delegate: flightRouteDelegate
        ScrollIndicator.vertical: ScrollIndicator {}
    }

    Rectangle {
        anchors.top: textInput.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        color: "white"
        visible: wpList.count === 0

        Label {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Qt.application.font.pixelSize*2

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: qsTr("<h3>Sorry!</h3><p>No flight routes available. To add a route here, chose 'Flight Route' from the main menu, edit a route and save it to the library.</p>")
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

} // Page
