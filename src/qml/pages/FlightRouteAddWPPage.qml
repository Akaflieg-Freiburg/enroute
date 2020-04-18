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

Page {
    id: page
    title: qsTr("Add Waypoint to Route")
    focus: true

    TextField {
        id: textInput

        anchors.right: parent.right
        anchors.rightMargin: Qt.application.font.pixelSize*2.0
        anchors.left: parent.left
        anchors.leftMargin: Qt.application.font.pixelSize*2.0

        placeholderText: qsTr("Waypoint Name")
        font.pixelSize: Qt.application.font.pixelSize*1.5
        focus: true

        onAccepted: {
            if (wpList.model.length > 0) {
                MobileAdaptor.vibrateBrief()
                flightRoute.append(wpList.model[0])
                stackView.pop()
            }
        }
    }

    Component {
        id: waypointDelegate

        ItemDelegate {
            text: model.modelData.richTextName
            icon.source: "/icons/waypoints/"+model.modelData.get("CAT")+".svg"
            icon.color: "transparent"

            anchors.left: parent.left
            anchors.right: parent.right
            Layout.fillWidth: true

            onClicked: {
                MobileAdaptor.vibrateBrief()
                flightRoute.append(model.modelData)
                stackView.pop()
            }
        }

    }

    ListView {
        id: wpList
        anchors.top: textInput.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        clip: true

        model: geoMapProvider.filteredWaypointObjects(textInput.displayText)
        delegate: waypointDelegate
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
            text: qsTr("<h3>Sorry!</h3><p>No waypoints available. Please make sure that an aviation map is installed.</p>")
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

} // Page
