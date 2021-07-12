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
import QtQuick.Layouts 1.15

import "../dialogs"
import "../items"

Page {
    id: settingsPage
    title: qsTr("Settings")

    header: StandardHeader {}

    ScrollView {
        id: view
        anchors.fill: parent
        anchors.topMargin: Qt.application.font.pixelSize

        ColumnLayout {
            width: settingsPage.width
            implicitWidth: settingsPage.width

            Label {
                Layout.leftMargin: Qt.application.font.pixelSize
                Layout.fillWidth: true
                text: qsTr("Moving Map")
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            SwitchDelegate {
                id: hideUpperAsp
                text: qsTr("Hide Airspaces ≥ FL100") + (
                          global.settings().hideUpperAirspaces ? (
                                                                  `<br><font color="#606060" size="2">`
                                                                  + qsTr("Upper airspaces hidden")
                                                                  +"</font>"
                                                                  ) : (
                                                                  `<br><font color="#606060" size="2">`
                                                                  + qsTr("All airspaces shown")
                                                                  + `</font>`
                                                                  )
                          )
                icon.source: "/icons/material/ic_map.svg"
                Layout.fillWidth: true
                Component.onCompleted: {
                    hideUpperAsp.checked = global.settings().hideUpperAirspaces
                }
                onToggled: {
                    global.mobileAdaptor().vibrateBrief()
                    global.settings().hideUpperAirspaces = hideUpperAsp.checked
                }
            }

            SwitchDelegate {
                id: hideGlidingSectors
                text: qsTr("Hide Gliding Sectors") + (
                          global.settings().hideGlidingSectors ? (
                                                                  `<br><font color="#606060" size="2">`
                                                                  + qsTr("Gliding sectors hidden")
                                                                  +"</font>"
                                                                  ) : (
                                                                  `<br><font color="#606060" size="2">`
                                                                  + qsTr("Gliding sectors shown")
                                                                  + `</font>`
                                                                  )
                          )
                icon.source: "/icons/material/ic_map.svg"
                Layout.fillWidth: true
                Component.onCompleted: {
                    hideGlidingSectors.checked = global.settings().hideGlidingSectors
                }
                onToggled: {
                    global.mobileAdaptor().vibrateBrief()
                    global.settings().hideGlidingSectors = hideGlidingSectors.checked
                }
            }

            Label {
                Layout.leftMargin: Qt.application.font.pixelSize
                text: qsTr("System")
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            SwitchDelegate {
                id: useMetricUnits
                text: qsTr("Use metric units")
                      + `<br><font color="#606060" size="2">`
                      + ( global.settings().useMetricUnits ?
                             qsTr("Speed in km/h, distance in km") :
                             qsTr("Speed in kn, distance in nm")
                         )
                      + "</font>"
                icon.source: "/icons/material/ic_speed.svg"
                Layout.fillWidth: true
                Component.onCompleted: useMetricUnits.checked = global.settings().useMetricUnits
                onCheckedChanged: {
                    global.mobileAdaptor().vibrateBrief()
                    global.settings().useMetricUnits = useMetricUnits.checked
                }
            }

            SwitchDelegate {
                id: nightMode
                text: qsTr("Night mode")
                icon.source: "/icons/material/ic_brightness_3.svg"
                Layout.fillWidth: true
                Component.onCompleted: {
                    nightMode.checked = global.settings().nightMode
                }
                onToggled: {
                    global.mobileAdaptor().vibrateBrief()
                    global.settings().nightMode = nightMode.checked
                }
            }

            Label {
                Layout.leftMargin: Qt.application.font.pixelSize
                text: qsTr("Help")
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your traffic receiver…")
                onClicked: trafficHelp.open()

                LongTextDialog {
                    id: trafficHelp
                    standardButtons: Dialog.Ok

                    title: qsTr("Connect your traffic receiver")
                    text: librarian.getStringFromRessource(":text/flarmSetup.md")
                }
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your flight simulator…")
                onClicked: simulatorHelp.open()

                LongTextDialog {
                    id: simulatorHelp
                    standardButtons: Dialog.Ok

                    title: qsTr("Connect your flight simulator")
                    text: librarian.getStringFromRessource(":text/simulatorSetup.md")
                }
            }

            Item { // Spacer
                height: 3
            }

        } // ColumnLayout
    }

} // Page
