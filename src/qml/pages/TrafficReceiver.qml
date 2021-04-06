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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import enroute 1.0
import "../dialogs"
import "../items"

Page {
    id: trafficReceiverPage
    title: qsTr("Traffic Receiver")

    header: ToolBar {

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.leftMargin: drawer.dragMargin

            icon.source: "/icons/material/ic_arrow_back.svg"
            icon.color: "white"
            onClicked: {
                mobileAdaptor.vibrateBrief()
                if (stackView.depth > 1) {
                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        }

        Label {
            anchors.left: backButton.right
            anchors.right: headerMenuToolButton.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top

            text: stackView.currentItem.title
            color: "white"
            elide: Label.ElideRight
            font.bold: true
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            icon.source: "/icons/material/ic_help_outline.svg"
            icon.color: "white"
            onClicked: {
                mobileAdaptor.vibrateBrief()
                trafficHelp.open()
            }

        }

    }


    ScrollView {
        id: view

        anchors.fill: parent
        clip: true

        anchors.topMargin: Qt.application.font.pixelSize
        anchors.bottomMargin: Qt.application.font.pixelSize
        anchors.leftMargin: Qt.application.font.pixelSize
        anchors.rightMargin: Qt.application.font.pixelSize

        contentWidth: width

        // The visibility behavior of the vertical scroll bar is a little complex.
        // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

        ColumnLayout {
            width: view.width
            implicitWidth: view.width

            Label {
                Layout.fillWidth: true

                text: qsTr("<h3>Status</h3>")
                font.bold: true
                textFormat: Text.MarkdownText
            }

            Label { // Status
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4

                text: flarmAdaptor.statusString

                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                bottomPadding: 0.6*Qt.application.font.pixelSize
                topPadding: 0.6*Qt.application.font.pixelSize
                leftPadding: 0.2*Qt.application.font.pixelSize
                rightPadding: 0.2*Qt.application.font.pixelSize

                leftInset: -4
                rightInset: -4

                // Background color according to METAR/FAA flight category
                background: Rectangle {
                    border.color: "black"
                    color: (flarmAdaptor.receivingHeartbeat) ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }
            }


            Label {
                Layout.fillWidth: true

                text:   {
                    if (flarmAdaptor.receivingHeartbeat)
                        return qsTr("<p>Well done! Go flying. Give yourself a pat on the back.</p>")
                    else
                        return qsTr("
<h3>How to connect your device to the traffic receiver</h3>

<ul style=\"margin-left:-25px;\">
<li>Make sure that your traffic receiver has an integrated Wi-Fi interface that acts as a wireless access point. Bluetooth devices are currently not supported.</li>
<li>Use the 'WLAN Settings' of your device to enter the WLAN network deployed by your traffic receiver.</li>
<li>Once your device has entered the WLAN network, use the button at the bottom of the page to connect the <strong>Enroute Flight Navigation</strong> to the traffic data stream.</li>
</ul>
") + qsTr("
<p>If no traffic data has arrived after a few seconds, somthing has gone wrong.</p>

<ul style=\"margin-left:-25px;\">
<li>Make sure that your device has entered the WLAN network deployed by your traffic receiver.  If not, then use the button at the bottem of the screen to abort the connection attempt.</li>
<li>Some traffic receivers protect the data stream with an additional password. This is currently not supported.</li>
<li>Click on the question mark in the page title to open a more detailed help dialog.</li>
</ul>
")
}
                textFormat: Text.RichText
                wrapMode: Text.WordWrap

            }
        }

    }


    footer: Pane {
        width: parent.width
        Material.elevation: 3

        ToolButton {
            anchors.centerIn: parent
            width: Math.min(implicitWidth, parent.width-Qt.application.font.pixelSize)

            text: (flarmAdaptor.receivingHeartbeat) ? qsTr("Disconnect from Traffic Receiver") : qsTr("Connect to Traffic Receiver")

            icon.source: (flarmAdaptor.receivingHeartbeat) ? "/icons/material/ic_cancel.svg" : "/icons/material/ic_tap_and_play.svg"

            Layout.alignment: Qt.AlignHCenter
            Material.foreground: Material.accent

            enabled: !timer.running
            onClicked: {
                if (flarmAdaptor.receivingHeartbeat)
                    flarmAdaptor.disconnectFromTrafficReceiver()
                else
                    flarmAdaptor.connectToTrafficReceiver()
                timer.running = true;
            }
            Timer {
                id: timer
                interval: 1000
            }

        }

    }


    LongTextDialog {
        id: trafficHelp
        standardButtons: Dialog.Ok
        anchors.centerIn: parent

        title: qsTr("Connect your traffic receiver")
        text: librarian.getStringFromRessource(":text/flarmSetup.md")
    }

}
