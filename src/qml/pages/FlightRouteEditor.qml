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

import QtQml 2.12
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
                    mobileAdaptor.vibrateBrief()
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
                    mobileAdaptor.vibrateBrief()
                    wpMenu.popup()
                }

                Menu {
                    id: wpMenu

                    Action {
                        text: qsTr("Move Up")

                        enabled: model.modelData !== flightRoute.firstWaypointObject
                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
                            flightRoute.moveUp(model.modelData)
                        }
                    } // Action

                    Action {
                        text: qsTr("Move Down")

                        enabled: model.modelData !== flightRoute.lastWaypointObject
                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
                            flightRoute.moveDown(model.modelData)
                        }
                    } // Action

                    Action {
                        text: qsTr("Remove")

                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
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
                mobileAdaptor.vibrateBrief()
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
                mobileAdaptor.vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu {
                id: headerMenuX

                MenuItem {
                    text: qsTr("Open from library …")
                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false
                        dialogLoader.active = false
                        dialogLoader.source = "../dialogs/FlightRouteOpenDialog.qml"
                        dialogLoader.active = true
                    }
                }

                MenuItem {
                    text: qsTr("Save to library …")
                    enabled: (flightRoute.routeObjects.length > 1) && (sv.currentIndex === 0)
                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false
                        dialogLoader.active = false
                        dialogLoader.source = "../dialogs/FlightRouteSaveDialog.qml"
                        dialogLoader.active = true
                    }
                }

                MenuItem {
                    text: qsTr("View Library …")
                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false
                        stackView.push("FlightRouteLibrary.qml")
                    }
                }

                MenuSeparator { }

                MenuItem {
                    text: qsTr("Import …")
                    visible: Qt.platform.os !== "android"

                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false

                        mobileAdaptor.importContent()
                    }
                }

                Menu {
                    title: qsTr("Export …")
                    enabled: (flightRoute.routeObjects.length > 1) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… to GeoJson file")
                        onTriggered: {
                            headerMenuX.close()
                            mobileAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = mobileAdaptor.exportContent(flightRoute.toGeoJSON(), "applicatin/geo+json", flightRoute.suggestedFilename())
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            }
                        }
                    }

                    MenuItem {
                        text: qsTr("… to GPX file")
                        onTriggered: {
                            headerMenuX.close()
                            mobileAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = mobileAdaptor.exportContent(flightRoute.toGpx(), "application/gpx+xml", flightRoute.suggestedFilename())
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            }
                        }
                    }
                }

                Menu {
                    title: qsTr("Open in other app …")
                    enabled: (flightRoute.routeObjects.length > 1) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… in GeoJson format")

                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = mobileAdaptor.viewContent(flightRoute.toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            }

                        }
                    }

                    MenuItem {
                        text: qsTr("… in GPX format")

                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = mobileAdaptor.viewContent(flightRoute.toGpx(), "application/gpx+xml", "FlightRoute-%1.gpx")
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            }

                        }
                    }

                }

                MenuSeparator { }

                MenuItem {
                    text: qsTr("Clear")
                    enabled: (flightRoute.routeObjects.length > 0) && (sv.currentIndex === 0)

                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false
                        clearDialog.open()
                    }

                }

                MenuItem {
                    text: qsTr("Reverse")
                    enabled: (flightRoute.routeObjects.length > 1) && (sv.currentIndex === 0)

                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false
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
                        bottom: globalSettings.useMetricUnits ? aircraft.minAircraftSpeedInKMH : aircraft.minAircraftSpeedInKT
                        top: globalSettings.useMetricUnits ? aircraft.maxAircraftSpeedInKMH : aircraft.maxAircraftSpeedInKT
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: globalSettings.useMetricUnits ? aircraft.cruiseSpeedInKMH = text : aircraft.cruiseSpeedInKT = text
                    color: (acceptableInput ? "black" : "red")
                    KeyNavigation.tab: descentSpeed
                    KeyNavigation.backtab: windSpeed
                    text: isFinite(aircraft.cruiseSpeedInKT) ? Math.round(globalSettings.useMetricUnits ?
                                                                              aircraft.cruiseSpeedInKMH.toString() :
                                                                              aircraft.cruiseSpeedInKT.toString() ) : ""
                    placeholderText: qsTr("undefined")
                }
                Label { text: globalSettings.useMetricUnits ? "km/h TAS" : "kt TAS" }
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
                        bottom: globalSettings.useMetricUnits ? aircraft.minAircraftSpeedInKMH : aircraft.minAircraftSpeedInKT
                        top: globalSettings.useMetricUnits ? aircraft.maxAircraftSpeedInKMH : aircraft.maxAircraftSpeedInKT
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: globalSettings.useMetricUnits ? aircraft.descentSpeedInKMH = text : aircraft.descentSpeedInKT = text
                    color: (acceptableInput ? "black" : "red")
                    KeyNavigation.tab: fuelConsumption
                    KeyNavigation.backtab: cruiseSpeed
                    text: isFinite(aircraft.descentSpeedInKT) ? Math.round(globalSettings.useMetricUnits ?
                                                                              aircraft.descentSpeedInKMH.toString() :
                                                                              aircraft.descentSpeedInKT.toString() ) : ""
                    placeholderText: qsTr("undefined")
                }
                Label { text: globalSettings.useMetricUnits ? "km/h TAS" : "kt TAS" }
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
                        bottom: globalSettings.useMetricUnits ? wind.minWindSpeedInKMH : wind.minWindSpeedInKT
                        top: globalSettings.useMetricUnits ? wind.maxWindSpeedInKMH : wind.maxWindSpeedInKT
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: globalSettings.useMetricUnits ? wind.windSpeedInKMH = text : wind.windSpeedInKT = text
                    color: (acceptableInput ? "black" : "red")
                    KeyNavigation.tab: cruiseSpeed
                    KeyNavigation.backtab: windDirection
                    text: isFinite(wind.windSpeedInKT) ? Math.round(globalSettings.useMetricUnits ? wind.windSpeedInKMH : wind.windSpeedInKT) : ""
                    placeholderText: qsTr("undefined")
                }
                Label { text: globalSettings.useMetricUnits ? "km/h" : "kt" }
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
                    mobileAdaptor.vibrateBrief()
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
            mobileAdaptor.vibrateBrief()
            flightRoute.clear()
        }
        onRejected: {
            mobileAdaptor.vibrateBrief()
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
            function onDetected(gesture) {
                dialogLoader.active = false
            }
        }
    }

    Dialog {
        id: shareErrorDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Error exporting data…")
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)

        Label {
            id: shareErrorDialogLabel
            width: shareErrorDialog.availableWidth
            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

        standardButtons: Dialog.Ok
        modal: true

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
