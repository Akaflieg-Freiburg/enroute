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

import enroute 1.0
import "../dialogs"
import "../items"

Page {
    id: flightRoutePage
    title: qsTr("Flight Route")

    Component {
        id: aiX

        GridLayout {
            id: grid
            columns: 3
            rowSpacing: 0

            anchors.left: parent.left
            anchors.leftMargin: Qt.application.font.pixelSize
            anchors.right: parent.right

            ItemDelegate {
                visible: model.modelData instanceof Waypoint
                icon.source: (model.modelData instanceof Waypoint) ? "/icons/waypoints/"+model.modelData.get("CAT")+".svg" : ""
                icon.color: "transparent"
                Layout.fillWidth: true
                text: model.modelData.richTextName

                onClicked: {
                    MobileAdaptor.vibrateBrief()
                    dialogLoader.active = false
                    dialogLoader.waypoint = model.modelData
                    dialogLoader.text = "noRouteButton"
                    dialogLoader.source = "../dialogs/WaypointDescription.qml"
                    dialogLoader.active = true
                }
            }

            ItemDelegate {
                visible: !(model.modelData instanceof Waypoint)
                icon.source: "/icons/vertLine.svg"
                icon.color: "transparent"
                Layout.fillWidth: true
                enabled: false
                text: model.modelData.description
            }

            ToolButton {
                id: wpMenuTB

                visible: model.modelData instanceof Waypoint
                icon.source: "/icons/material/ic_more_horiz.svg"
                onClicked: {
                    MobileAdaptor.vibrateBrief()
                    wpMenu.popup()
                }

                Menu {
                    id: wpMenu

                    Action {
                        text: qsTr("Move Up")
                        icon.source: "/icons/material/ic_keyboard_arrow_up.svg"

                        enabled: model.modelData !== flightRoute.firstWaypointObject
                        onTriggered: {
                            MobileAdaptor.vibrateBrief()
                            flightRoute.moveUp(model.modelData)
                        }
                    } // Action

                    Action {
                        text: qsTr("Move Down")
                        icon.source: "/icons/material/ic_keyboard_arrow_down.svg"

                        enabled: model.modelData !== flightRoute.lastWaypointObject
                        onTriggered: {
                            MobileAdaptor.vibrateBrief()
                            flightRoute.moveDown(model.modelData)
                        }
                    } // Action

                    Action {
                        text: qsTr("Remove")
                        icon.source: "/icons/material/ic_delete.svg"

                        onTriggered: {
                            MobileAdaptor.vibrateBrief()
                            flightRoute.removeWaypoint(model.modelData)
                        }
                    } // Action
                }
            } // ToolButton

        } // GridLayout
    } // Component

    header: ToolBar {

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.leftMargin: drawer.dragMargin

            icon.source: "/icons/material/ic_arrow_back.svg"
            onClicked: {
                MobileAdaptor.vibrateBrief()
                if (stackView.depth > 1) {
                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        } // ToolButton

        Label {
            anchors.left: backButton.right
            anchors.right: headerMenuToolButton.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.bold: true
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            visible: (sv.currentIndex === 0)
            icon.source: "/icons/material/ic_more_vert.svg"
            onClicked: {
                MobileAdaptor.vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu {
                id: headerMenuX

                MenuItem {
                    id: loadItem

                    text: qsTr("Load Route…")
                    icon.source: "/icons/material/ic_open_in_new.svg"
                    enabled: true

                    onTriggered: {
                        MobileAdaptor.vibrateBrief()
                        dialogLoader.active = false
                        dialogLoader.source = "../dialogs/FlightRouteOpenDialog.qml"
                        dialogLoader.active = true
                    }
                }

                MenuItem {
                    id: saveItem

                    text: qsTr("Save Route…")
                    icon.source: "/icons/material/ic_library_add.svg"
                    enabled: (flightRoute.routeObjects.length > 1) && (sv.currentIndex === 0)

                    onTriggered: {
                        MobileAdaptor.vibrateBrief()
                        dialogLoader.active = false
                        dialogLoader.source = "../dialogs/FlightRouteSaveDialog.qml"
                        dialogLoader.active = true
                    }
                }

                MenuItem {
                    id: manageItem

                    text: qsTr("Manage Library…")
                    icon.source: "/icons/material/ic_library_books.svg"

                    onTriggered: {
                        MobileAdaptor.vibrateBrief()
                        stackView.push("FlightRouteManageLibraryPage.qml")
                    }
                }

                Rectangle {
                    height: 1
                    color: Material.primary
                }

                MenuItem {
                    id: clearItem

                    text: qsTr("Clear Route")
                    icon.source: "/icons/material/ic_delete.svg"
                    enabled: (flightRoute.routeObjects.length > 0) && (sv.currentIndex === 0)

                    onTriggered: {
                        MobileAdaptor.vibrateBrief()
                        clearDialog.open()
                    }

                }

                MenuItem {
                    id: reverseItem

                    text: qsTr("Reverse Route")
                    icon.source: "/icons/material/ic_swap_vert.svg"
                    enabled: (flightRoute.routeObjects.length > 1) && (sv.currentIndex === 0)

                    onTriggered: {
                        MobileAdaptor.vibrateBrief()
                        flightRoute.reverse()
                    }
                }

            }
        } // ToolButton

    } // ToolBar

    TabBar {
        id: bar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        currentIndex: sv.currentIndex
        TabButton { text: qsTr("Route") }
        TabButton { text: qsTr("Aircraft and Wind") }
        Material.elevation: 3
    } // TabBar

    SwipeView{
        id: sv

        anchors.top: bar.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        currentIndex: bar.currentIndex

        Item {
            id: flightRouteTab

            Label {
                anchors.fill: parent
                visible: flightRoute.isEmpty

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment : Text.AlignVCenter
                textFormat: Text.RichText

                text: qsTr("<h2>Empty Route</h2><p>Use the button 'Add Waypoint' below.</p>")
            }

            ListView {
                anchors.fill: parent

                model: flightRoute.routeObjects
                delegate: aiX
                clip: true
                ScrollIndicator.vertical: ScrollIndicator {}
            }


        }

        ScrollView {
            contentWidth: sv.width

            GridLayout {

                id: windAndAircraftTab

                anchors.left: parent.left
                anchors.leftMargin: Qt.application.font.pixelSize
                anchors.right: parent.right
                anchors.rightMargin: Qt.application.font.pixelSize

                columns: 4

                Label { Layout.fillHeight: true }
                Label {
                    text: qsTr("Aircraft")
                    Layout.columnSpan: 4
                    font.pixelSize: Qt.application.font.pixelSize*1.2
                    font.bold: true
                    color: Material.primary
                }

                Label { text: qsTr("Cruise Speed") }
                TextField {
                    id: cruiseSpeed
                    Layout.fillWidth: true
                    validator: DoubleValidator {
                        bottom: aircraft.minAircraftSpeed
                        top: aircraft.maxAircraftSpeed
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: aircraft.cruiseSpeedInKT = text
                    color: (acceptableInput ? "black" : "red")
                    KeyNavigation.tab: descentSpeed
                    KeyNavigation.backtab: windSpeed
                    text: isFinite(aircraft.cruiseSpeedInKT) ? aircraft.cruiseSpeedInKT.toString() : ""
                    placeholderText: qsTr("undefined")
                }
                Label { text: "kt TAS" }
                ToolButton {
                    icon.source: "/icons/material/ic_delete.svg"
                    enabled: cruiseSpeed.text !== ""
                    onClicked: {
                        aircraft.cruiseSpeedInKT = -1
                        cruiseSpeed.clear()
                    }
                }

                Label { text: qsTr("Descent Speed") }
                TextField {
                    id: descentSpeed
                    Layout.fillWidth: true
                    validator: DoubleValidator {
                        bottom: aircraft.minAircraftSpeed
                        top: aircraft.maxAircraftSpeed
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: aircraft.descentSpeedInKT = text
                    color: (acceptableInput ? "black" : "red")
                    KeyNavigation.tab: fuelConsumption
                    KeyNavigation.backtab: cruiseSpeed
                    text: isFinite(aircraft.descentSpeedInKT) ? aircraft.descentSpeedInKT.toString() : ""
                    placeholderText: qsTr("undefined")
                }
                Label { text: "kt TAS" }
                ToolButton {
                    icon.source: "/icons/material/ic_delete.svg"
                    enabled: descentSpeed.text !== ""
                    onClicked: {
                        aircraft.descentSpeedInKT = -1
                        descentSpeed.clear()
                    }
                }

                Label { text: qsTr("Fuel consumption") }
                TextField {
                    id: fuelConsumption
                    Layout.fillWidth: true
                    validator: DoubleValidator {
                        bottom: aircraft.minFuelConsuption
                        top: aircraft.maxFuelConsuption
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: aircraft.fuelConsumptionInLPH = text
                    color: (acceptableInput ? "black" : "red")
                    KeyNavigation.tab: windDirection
                    KeyNavigation.backtab: descentSpeed
                    text: isFinite(aircraft.fuelConsumptionInLPH) ? aircraft.fuelConsumptionInLPH.toString() : ""
                    placeholderText: qsTr("undefined")
                }
                Label { text: qsTr("l/h") }
                ToolButton {
                    icon.source: "/icons/material/ic_delete.svg"
                    enabled: fuelConsumption.text !== ""
                    onClicked: {
                        aircraft.fuelConsumptionInLPH = -1
                        fuelConsumption.clear()
                    }
                }

                Label { Layout.fillHeight: true }
                Label {
                    text: qsTr("Wind")
                    Layout.columnSpan: 4
                    font.pixelSize: Qt.application.font.pixelSize*1.2
                    font.bold: true
                    color: Material.primary
                }

                Label { text: qsTr("Direction") }
                TextField {
                    id: windDirection
                    Layout.fillWidth: true
                    validator: DoubleValidator {
                        bottom: wind.minWindDirection
                        top: wind.maxWindDirection
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: wind.windDirectionInDEG = text
                    color: (acceptableInput ? "black" : "red")
                    KeyNavigation.tab: windSpeed
                    KeyNavigation.backtab: fuelConsumption
                    text: isFinite(wind.windDirectionInDEG) ? wind.windDirectionInDEG : ""
                    placeholderText: qsTr("undefined")
                }
                Label { text: "°" }
                ToolButton {
                    icon.source: "/icons/material/ic_delete.svg"
                    enabled: windDirection.text !== ""
                    onClicked: {
                        wind.windDirectionInDEG = -1
                        windDirection.clear()
                    }
                }

                Label { text: qsTr("Speed") }
                TextField {
                    id: windSpeed
                    Layout.fillWidth: true
                    validator: DoubleValidator {
                        bottom: wind.minWindSpeed
                        top: wind.maxWindSpeed
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: wind.windSpeedInKT = text
                    color: (acceptableInput ? "black" : "red")
                    KeyNavigation.tab: cruiseSpeed
                    KeyNavigation.backtab: windDirection
                    text: isFinite(wind.windSpeedInKT) ? wind.windSpeedInKT : ""
                    placeholderText: qsTr("undefined")
                }
                Label { text: "kt" }
                ToolButton {
                    icon.source: "/icons/material/ic_delete.svg"
                    enabled: windSpeed.text !== ""
                    onClicked: {
                        wind.windSpeedInKT = -1
                        windSpeed.clear()
                    }
                }

                Label { Layout.fillHeight: true }
            }
        }
    }

    footer: Pane {
        width: parent.width
        Material.elevation: 3

        ColumnLayout {
            width: parent.width
            Label {
                id: summary

                Layout.fillWidth: true
                text: flightRoute.summary
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                visible: flightRoute.summary !== ""
            }

            ToolButton {
                id: addWPButton

                visible: (sv.currentIndex === 0)
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Add Waypoint")
                icon.source: "/icons/material/ic_add_circle.svg"

                onClicked: {
                    MobileAdaptor.vibrateBrief()
                    dialogLoader.active = false
                    dialogLoader.source = "../dialogs/FlightRouteAddWPDialog.qml"
                    dialogLoader.active = true
                }
            }
        }
    }

    Dialog {
        id: clearDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Clear route?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            width: clearDialog.availableWidth

            text: qsTr("Once erased, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

        onAccepted: {
            MobileAdaptor.vibrateBrief()
            flightRoute.clear()
        }
        onRejected: {
            MobileAdaptor.vibrateBrief()
            close()
        }
    }

    Loader {
        id: dlgLoader
        anchors.fill: parent

        property string title
        property string text
        property Waypoint waypoint

        onLoaded: {
            item.anchors.centerIn = dialogLoader
            item.modal = true
            item.open()
        }

        Connections {
            target: sensorGesture
            onDetected: dialogLoader.active = false
        }
    }

    Shortcut {
        sequence: "Ctrl+a"
        onActivated: {
            dialogLoader.active = false
            dialogLoader.source = "../dialogs/FlightRouteAddWPDialog.qml"
            dialogLoader.active = true
        }
    }

} // Page
