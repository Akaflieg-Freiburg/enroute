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
    id: settingsPage
    title: qsTr("Settings")

    header: StandardHeader {}

    ScrollView {
        id: view
        anchors.fill: parent
        anchors.topMargin: Qt.application.font.pixelSize
        ScrollIndicator.vertical: ScrollIndicator {}

        ColumnLayout {
            width: settingsPage.width
            implicitWidth: settingsPage.width

            Label {
                Layout.leftMargin: Qt.application.font.pixelSize
                Layout.fillWidth: true
                text: qsTr("Moving Map")
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.primary
            }

            SwitchDelegate {
                id: hideUpperAsp
                text: qsTr("Hide Airspaces above FL100") + (globalSettings.hideUpperAirspaces ? `<br><font color="#606060" size="2">`
                                                                                                + qsTr("Upper airspaces currently hidden")
                                                                                                +"</font>"
                                                                                              : `<br><font color="#606060" size="2">`
                                                                                                + qsTr("All airspaces currently shown")
                                                                                                + `</font>`
                                                            )
                icon.source: "/icons/material/ic_map.svg"
                icon.color: Material.primary
                Layout.fillWidth: true
                Component.onCompleted: hideUpperAsp.checked = globalSettings.hideUpperAirspaces
                onToggled: {
                    mobileAdaptor.vibrateBrief()
                    globalSettings.hideUpperAirspaces = hideUpperAsp.checked
                }
            }

            Label {
                Layout.leftMargin: Qt.application.font.pixelSize
                text: qsTr("Libraries")
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.primary
            }

            ItemDelegate {
                text: qsTr("Flight Routes")
                icon.source: "/icons/material/ic_directions.svg"
                icon.color: Material.primary
                Layout.fillWidth: true

                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    stackView.push("FlightRouteLibrary.qml")
                    drawer.close()
                }
            }

            ItemDelegate {
                text: qsTr("Maps") + (MapManager.aviationMapUpdatesAvailable ? `<br><font color="#606060" size="2">`+qsTr("Updates available") + "</font>" : "")
                icon.source: "/icons/material/ic_map.svg"
                icon.color: Material.primary
                Layout.fillWidth: true

                enabled: !satNav.isInFlight
                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    stackView.push("MapManager.qml")
                    drawer.close()
                }
            }

            Label {
                Layout.leftMargin: Qt.application.font.pixelSize
                text: qsTr("System")
                font.pixelSize: Qt.application.font.pixelSize*1.2
                font.bold: true
                color: Material.primary
            }

            SwitchDelegate {
                id: preferEnglish
                text: qsTr("Prefer English")
                icon.source: "/icons/material/ic_translate.svg"
                icon.color: Material.primary
                visible: globalSettings.hasTranslation
                Layout.fillWidth: true
                Component.onCompleted: preferEnglish.checked = globalSettings.preferEnglish
                onCheckedChanged: {
                    mobileAdaptor.vibrateBrief()
                    globalSettings.preferEnglish = preferEnglish.checked
                }
            }

            ItemDelegate {
                text: qsTr("SatNav Status")+`<br><font color="#606060" size="2">`+qsTr("Current Status") + `: ${satNav.statusAsString}</font>`
                icon.source: "/icons/material/ic_satellite.svg"
                icon.color: Material.primary
                Layout.fillWidth: true
                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    dialogLoader.active = false
                    dialogLoader.source = "../dialogs/SatNavStatusDialog.qml"
                    dialogLoader.active = true
                }
            }

        } // ColumnLayout
    } // Scrollview

} // Page
