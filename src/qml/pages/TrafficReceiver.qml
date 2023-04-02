/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import enroute 1.0
import "../dialogs"
import "../items"

Page {
    id: trafficReceiverPage
    objectName: "TrafficReceiverPage"

    title: qsTr("Traffic Data Receiver")

    required property var appWindow

    header: StandardHeader {}

    ScrollView {
        id: sView

        anchors.fill: parent
        contentWidth: availableWidth // Disable horizontal scrolling

        clip: true

        bottomPadding: font.pixelSize + SafeInsets.bottom
        leftPadding: font.pixelSize + SafeInsets.left
        rightPadding: font.pixelSize + SafeInsets.right
        topPadding: font.pixelSize

        ColumnLayout {
            width: sView.availableWidth

            Label {
                Layout.fillWidth: true

                text: qsTr("Connection Status")
                font.pixelSize: sView.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            Label { // Status
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4

                text: TrafficDataProvider.statusString

                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                bottomPadding: 0.6*font.pixelSize
                topPadding: 0.6*font.pixelSize
                leftPadding: 0.2*font.pixelSize
                rightPadding: 0.2*font.pixelSize

                leftInset: -4
                rightInset: -4

                // Background color according to METAR/FAA flight category
                background: Rectangle {
                    border.color: "black"
                    color: (TrafficDataProvider.receivingHeartbeat) ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }
            }

            Label {
                Layout.fillWidth: true
                visible: TrafficDataProvider.receivingHeartbeat

                text: qsTr("Traffic Data Receiver Status")
                font.pixelSize: sView.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            Label {
                id: problemStatus

                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4

                visible: TrafficDataProvider.receivingHeartbeat

                bottomPadding: 0.6*font.pixelSize
                topPadding: 0.6*font.pixelSize
                leftPadding: 0.2*font.pixelSize
                rightPadding: 0.2*font.pixelSize

                leftInset: -4
                rightInset: -4

                property string myText: {
                    if (TrafficDataProvider.trafficReceiverRuntimeError === "")
                        return TrafficDataProvider.trafficReceiverSelfTestError
                    if (TrafficDataProvider.trafficReceiverSelfTestError === "")
                        return TrafficDataProvider.trafficReceiverRuntimeError
                    return TrafficDataProvider.trafficReceiverRuntimeError + "<br>" + TrafficDataProvider.trafficReceiverSelfTestError
                }

                text: (myText === "") ? qsTr("No problem reported") : myText
                wrapMode: Text.WordWrap

                background: Rectangle {
                    border.color: "black"
                    color: (problemStatus.myText === "") ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }

            }


            Button {
                Layout.alignment: Qt.AlignHCenter
                icon.source: "/icons/material/ic_tap_and_play.svg"
                text: qsTr("Connect")
                enabled: !timer.running
                visible: !TrafficDataProvider.receivingHeartbeat
                onClicked: {
                    TrafficDataProvider.connectToTrafficReceiver()
                    timer.running = true;
                }
                Timer {
                    id: timer
                    interval: 1000
                }
            }

            Item {
                Layout.preferredHeight: sView.font.pixelSize*0.5
                Layout.columnSpan: 2
            }

            Label {
                Layout.fillWidth: true
                visible: !TrafficDataProvider.receivingHeartbeat

                text: qsTr("Help")
                font.pixelSize: sView.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                visible: !TrafficDataProvider.receivingHeartbeat
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your traffic receiver…")
                onClicked: trafficReceiverPage.appWindow.openManual("02-steps/traffic.html")
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                visible: !TrafficDataProvider.receivingHeartbeat
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your flight simulator…")
                onClicked: trafficReceiverPage.appWindow.openManual("02-steps/simulator.html")
            }

        }

    }

}
