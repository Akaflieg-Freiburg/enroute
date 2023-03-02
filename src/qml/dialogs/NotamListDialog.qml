/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../items"

CenteringDialog {
    id: notamListDialog

    required property notamList notamList
    required property waypoint waypoint

    modal: true
    standardButtons: Dialog.Close

    Component {
        id: notamDelegate

        ItemDelegate {
            id: delItem
            width: parent ? parent.width : undefined

            required property var model
            property bool read: NotamProvider.isRead(delItem.model.modelData.number)

            contentItem: Label {
                id: lbl
                wrapMode: Text.WordWrap
                text: {
                    GlobalSettings.expandNotamAbbreviations
                    Clock.date

                    delItem.model.modelData.richText()
                }
                textFormat: Text.RichText
                opacity: delItem.read ? 0.5 : 1.0
                Behavior on opacity {
                    NumberAnimation { duration: 200 }
                }
            }

            Label {
                id: myToast
                anchors.centerIn: parent

                text: qsTr("Marked as Read")

                color: "white"
                bottomInset: -5
                topInset: -5
                leftInset: -5
                rightInset: -5

                horizontalAlignment: Text.AlignHCenter
                background: Rectangle {
                    color: Material.primary
                    radius: 5
                }
                opacity: 0
                SequentialAnimation {
                    id: seqA

                    NumberAnimation { target: myToast; property: "opacity"; to: 1; duration: 400 }
                    PauseAnimation { duration: 1000 }
                    NumberAnimation { target: myToast; property: "opacity"; to: 0; duration: 400 }
                }

            }


            onClicked: {
                read = !read
                NotamProvider.setRead(delItem.model.modelData.number, read)
                if (read)
                    seqA.start()
            }
        }

    }

    ColumnLayout {
        anchors.fill: parent

        RowLayout { // Header with icon and name
            id: headX
            Layout.fillWidth: true

            Icon { source: notamListDialog.waypoint.icon }

            Label {
                text: qsTr("Notam for %1").arg(notamListDialog.waypoint.extendedName)
                font.bold: true
                font.pixelSize: 1.2*notamListDialog.font.pixelSize
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignVCenter
                wrapMode: Text.WordWrap
            }
        }

        Label { // Second header line with distance and QUJ
            text: Navigator.aircraft.describeWay(PositionProvider.positionInfo.coordinate(), waypoint.coordinate)
            visible: PositionProvider.receivingPositionInfo
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            wrapMode: Text.WordWrap
        }

        Label { // Incomplete Data warning

            visible: !notamListDialog.notamList.isValid
            text: qsTr("Data potentially outdated. Update requested.")
            Layout.fillWidth: true
            wrapMode: Text.WordWrap

            bottomPadding: 0.2*font.pixelSize
            topPadding: 0.2*font.pixelSize
            leftPadding: 0.2*font.pixelSize
            rightPadding: 0.2*font.pixelSize

            // Background color according to METAR/FAA flight category
            background: Rectangle {
                border.color: "black"
                color: "yellow"
                opacity: 0.2
            }

        }

        ListView {
            id: notamlistview

            Layout.fillHeight: true
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight

            clip: true

            delegate: notamDelegate

            model: notamListDialog.notamList.notams
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        CheckDelegate {
            Layout.fillWidth: true

            text: qsTr("Expand Abbreviations")

            checked: GlobalSettings.expandNotamAbbreviations

            onCheckedChanged: {
                GlobalSettings.expandNotamAbbreviations = checked
            }
        }
    }

}
