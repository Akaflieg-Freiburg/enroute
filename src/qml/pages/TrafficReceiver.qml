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

    header: StandardHeader {}

    ScrollView {
        id: sView

        anchors.fill: parent
        contentWidth: availableWidth // Disable horizontal scrolling

        clip: true

        bottomPadding: view.font.pixelSize + SafeInsets.bottom
        leftPadding: view.font.pixelSize + SafeInsets.left
        rightPadding: view.font.pixelSize + SafeInsets.right
        topPadding: view.font.pixelSize

        ColumnLayout {
            width: sView.availableWidth

            Label {
                Layout.fillWidth: true

                text: qsTr("Connection Status")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            Label { // Status
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4

                text: global.trafficDataProvider().statusString

                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                bottomPadding: 0.6*view.font.pixelSize
                topPadding: 0.6*view.font.pixelSize
                leftPadding: 0.2*view.font.pixelSize
                rightPadding: 0.2*view.font.pixelSize

                leftInset: -4
                rightInset: -4

                // Background color according to METAR/FAA flight category
                background: Rectangle {
                    border.color: "black"
                    color: (global.trafficDataProvider().receivingHeartbeat) ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }
            }

            Label {
                Layout.fillWidth: true
                visible: global.trafficDataProvider().receivingHeartbeat

                text: qsTr("Traffic Data Receiver Status")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            Label {
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4

                visible: global.trafficDataProvider().receivingHeartbeat

                bottomPadding: 0.6*view.font.pixelSize
                topPadding: 0.6*view.font.pixelSize
                leftPadding: 0.2*view.font.pixelSize
                rightPadding: 0.2*view.font.pixelSize

                leftInset: -4
                rightInset: -4

                property string myText: {
                    if (global.trafficDataProvider().trafficReceiverRuntimeError === "")
                        return global.trafficDataProvider().trafficReceiverSelfTestError
                    if (global.trafficDataProvider().trafficReceiverSelfTestError === "")
                        return global.trafficDataProvider().trafficReceiverRuntimeError
                    return global.trafficDataProvider().trafficReceiverRuntimeError + "<br>" + global.trafficDataProvider().trafficReceiverSelfTestError
                }

                text: (myText === "") ? qsTr("No problem reported") : myText
                wrapMode: Text.WordWrap

                background: Rectangle {
                    border.color: "black"
                    color: (parent.myText === "") ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }

            }


            Button {
                Layout.alignment: Qt.AlignHCenter
                icon.source: "/icons/material/ic_tap_and_play.svg"
                text: qsTr("Connect to Traffic Receiver")
                enabled: !timer.running
                visible: !global.trafficDataProvider().receivingHeartbeat
                onClicked: {
                    global.trafficDataProvider().connectToTrafficReceiver()
                    timer.running = true;
                }
                Timer {
                    id: timer
                    interval: 1000
                }
            }

            Item {
                height: view.font.pixelSize*0.5
                Layout.columnSpan: 2
            }

            Label {
                Layout.fillWidth: true
                visible: !global.trafficDataProvider().receivingHeartbeat

                text: qsTr("Help")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                visible: !global.trafficDataProvider().receivingHeartbeat
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your traffic receiver…")
                onClicked: openManual("02-steps/traffic.html")
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                visible: !global.trafficDataProvider().receivingHeartbeat
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your flight simulator…")
                onClicked: openManual("02-steps/simulator.html")
            }

        }

    }

}
