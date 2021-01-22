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
    id: flightRoutePage
    title: qsTr("Flight Route")

    Component {
        id: aiX

        GridLayout {
            id: grid
            columns: 3

            columnSpacing: 0
            rowSpacing: 0

            anchors.left: parent.left
            anchors.leftMargin: Qt.application.font.pixelSize
            anchors.right: parent.right

            WordWrappingItemDelegate {
                visible: model.modelData instanceof Waypoint
                icon.source: (model.modelData instanceof Waypoint) ? model.modelData.icon : ""
                Layout.fillWidth: true
                text: model.modelData.twoLineTitle

                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    waypointDescription.waypoint = model.modelData
                    waypointDescription.open()
                }
            }

            ItemDelegate {
                visible: !(model.modelData instanceof Waypoint)
                icon.source: "/icons/vertLine.svg"
                Layout.fillWidth: true
                enabled: false
                text: globalSettings.useMetricUnits ? model.modelData.descriptionMetric : model.modelData.description
            }

            ToolButton {
                id: editButton

                visible: (model.modelData instanceof Waypoint) && (model.modelData.icon.indexOf("WP") !== -1)
                icon.source: "/icons/material/ic_mode_edit.svg"
                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    wpEditor.waypoint = model.modelData
                    wpEditor.open()
                }
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
            }

        }
    }

    header: ToolBar {

        Material.foreground: "white"

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
        }

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
                cascade: true

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
                    enabled: Qt.platform.os !== "android"
                    height: Qt.platform.os !== "android" ? undefined : 0

                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false

                        mobileAdaptor.importContent()
                    }
                }

                AutoSizingMenu {
                    title: Qt.platform.os === "android" ? qsTr("Share …") : qsTr("Export …")
                    enabled: (flightRoute.routeObjects.length > 1) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… to GeoJSON file")
                        onTriggered: {
                            headerMenuX.close()
                            mobileAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = mobileAdaptor.exportContent(flightRoute.toGeoJSON(), "applicatin/geo+json", flightRoute.suggestedFilename())
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            } else
                                toast.doToast(qsTr("Flight route exported"))
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
                            } else
                                toast.doToast(qsTr("Flight route exported"))
                        }
                    }
                }

                AutoSizingMenu {
                    title: qsTr("Open in other app …")
                    enabled: (flightRoute.routeObjects.length > 1) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… in GeoJSON format")

                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = mobileAdaptor.viewContent(flightRoute.toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            } else
                                toast.doToast(qsTr("Flight route opened in other app"))
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
                            } else
                                toast.doToast(qsTr("Flight route opened in other app"))
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
                        toast.doToast(qsTr("Flight route reversed"))
                    }
                }

            }
        }

    }

    TabBar {
        id: bar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        currentIndex: sv.currentIndex
        TabButton { text: qsTr("Route") }
        TabButton { text: qsTr("Wind") }
        TabButton { text: qsTr("ACFT") }
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
                textFormat: Text.StyledText

                text: qsTr("<h2>Empty Route</h2><p>Use the button <strong>Add Waypoint</strong> below.</p>")
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
            id: windTab

            contentWidth: width
            clip: true

            GridLayout {
                anchors.left: parent.left
                anchors.leftMargin: Qt.application.font.pixelSize
                anchors.right: parent.right
                anchors.rightMargin: Qt.application.font.pixelSize

                columns: 4

                Label { Layout.fillHeight: true }
                Label {
                    text: qsTr("Wind")
                    Layout.columnSpan: 4
                    font.pixelSize: Qt.application.font.pixelSize*1.2
                    font.bold: true
                    color: Material.accent
                }

                Label {
                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("Direction")
                }
                TextField {
                    id: windDirection
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    Layout.minimumWidth: Qt.application.font.pixelSize*5
                    validator: DoubleValidator {
                        bottom: wind.minWindDirection
                        top: wind.maxWindDirection
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: {
                        wind.windDirectionInDEG = text
                        windSpeed.focus = true
                    }
                    color: (acceptableInput ? Material.foreground : "red")
                    KeyNavigation.tab: windSpeed
                    text: isFinite(wind.windDirectionInDEG) ? wind.windDirectionInDEG : ""
                    placeholderText: qsTr("undefined")
                }
                Label {
                    text: "°"
                    Layout.alignment: Qt.AlignBaseline
                }
                ToolButton {
                    icon.source: "/icons/material/ic_clear.svg"
                    Layout.alignment: Qt.AlignVCenter
                    enabled: windDirection.text !== ""
                    onClicked: {
                        wind.windDirectionInDEG = -1
                        windDirection.clear()
                    }
                }

                Label {
                    text: qsTr("Speed")
                    Layout.alignment: Qt.AlignBaseline
                }
                TextField {
                    id: windSpeed
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    Layout.minimumWidth: Qt.application.font.pixelSize*5
                    validator: DoubleValidator {
                        bottom: globalSettings.useMetricUnits ? wind.minWindSpeedInKMH : wind.minWindSpeedInKT
                        top: globalSettings.useMetricUnits ? wind.maxWindSpeedInKMH : wind.maxWindSpeedInKT
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: {
                        globalSettings.useMetricUnits ? wind.windSpeedInKMH = text : wind.windSpeedInKT = text
                        focus = false
                    }
                    color: (acceptableInput ? Material.foreground : "red")
                    text: isFinite(wind.windSpeedInKT) ? Math.round(globalSettings.useMetricUnits ? wind.windSpeedInKMH : wind.windSpeedInKT) : ""
                    placeholderText: qsTr("undefined")
                }
                Label {
                    text: globalSettings.useMetricUnits ? "km/h" : "kt"
                    Layout.alignment: Qt.AlignBaseline
                }
                ToolButton {
                    icon.source: "/icons/material/ic_clear.svg"
                    Layout.alignment: Qt.AlignVCenter
                    enabled: windSpeed.text !== ""
                    onClicked: {
                        wind.windSpeedInKT = -1
                        windSpeed.clear()
                    }
                }

            }

        }

        ScrollView {
            id: acftTab

            contentWidth: width
            clip: true

            GridLayout {
                id: aircraftTab
                anchors.left: parent.left
                anchors.leftMargin: Qt.application.font.pixelSize
                anchors.right: parent.right
                anchors.rightMargin: Qt.application.font.pixelSize

                columns: 4

                Label { Layout.fillHeight: true }
                Label {
                    text: qsTr("True Airspeed")
                    Layout.columnSpan: 4
                    font.pixelSize: Qt.application.font.pixelSize*1.2
                    font.bold: true
                    color: Material.accent
                }

                Label {
                    text: qsTr("Cruise")
                    Layout.alignment: Qt.AlignBaseline
                }
                TextField {
                    id: cruiseSpeed
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    Layout.minimumWidth: Qt.application.font.pixelSize*5
                    validator: DoubleValidator {
                        bottom: globalSettings.useMetricUnits ? aircraft.minAircraftSpeedInKMH : aircraft.minAircraftSpeedInKT
                        top: globalSettings.useMetricUnits ? aircraft.maxAircraftSpeedInKMH : aircraft.maxAircraftSpeedInKT
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: {
                        globalSettings.useMetricUnits ? aircraft.cruiseSpeedInKMH = text : aircraft.cruiseSpeedInKT = text
                        descentSpeed.focus = true
                    }
                    color: (acceptableInput ? Material.foreground : "red")
                    KeyNavigation.tab: descentSpeed
                    KeyNavigation.backtab: windSpeed
                    text: isFinite(aircraft.cruiseSpeedInKT) ? Math.round(globalSettings.useMetricUnits ?
                                                                              aircraft.cruiseSpeedInKMH.toString() :
                                                                              aircraft.cruiseSpeedInKT.toString() ) : ""
                    placeholderText: qsTr("undefined")
                }
                Label {
                    text: globalSettings.useMetricUnits ? "km/h" : "kt"
                    Layout.alignment: Qt.AlignBaseline
                }
                ToolButton {
                    icon.source: "/icons/material/ic_clear.svg"
                    Layout.alignment: Qt.AlignVCenter
                    enabled: cruiseSpeed.text !== ""
                    onClicked: {
                        aircraft.cruiseSpeedInKT = -1
                        cruiseSpeed.clear()
                    }
                }

                Label {
                    text: qsTr("Descent")
                    Layout.alignment: Qt.AlignBaseline
                }
                TextField {
                    id: descentSpeed
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    Layout.minimumWidth: Qt.application.font.pixelSize*5
                    validator: DoubleValidator {
                        bottom: globalSettings.useMetricUnits ? aircraft.minAircraftSpeedInKMH : aircraft.minAircraftSpeedInKT
                        top: globalSettings.useMetricUnits ? aircraft.maxAircraftSpeedInKMH : aircraft.maxAircraftSpeedInKT
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: {
                        globalSettings.useMetricUnits ? aircraft.descentSpeedInKMH = text : aircraft.descentSpeedInKT = text
                        fuelConsumption.focus = true
                    }
                    color: (acceptableInput ? Material.foreground : "red")
                    KeyNavigation.tab: fuelConsumption
                    KeyNavigation.backtab: cruiseSpeed
                    text: isFinite(aircraft.descentSpeedInKT) ? Math.round(globalSettings.useMetricUnits ?
                                                                               aircraft.descentSpeedInKMH.toString() :
                                                                               aircraft.descentSpeedInKT.toString() ) : ""
                    placeholderText: qsTr("undefined")
                }
                Label {
                    text: globalSettings.useMetricUnits ? "km/h" : "kt"
                    Layout.alignment: Qt.AlignBaseline
                }
                ToolButton {
                    icon.source: "/icons/material/ic_clear.svg"
                    Layout.alignment: Qt.AlignVCenter
                    enabled: descentSpeed.text !== ""
                    onClicked: {
                        aircraft.descentSpeedInKT = -1
                        descentSpeed.clear()
                    }
                }

                Label { Layout.fillHeight: true }
                Label {
                    text: qsTr("Fuel consumption")
                    Layout.columnSpan: 4
                    font.pixelSize: Qt.application.font.pixelSize*1.2
                    font.bold: true
                    color: Material.accent
                }

                Label {
                    text: qsTr("Cruise")
                    Layout.alignment: Qt.AlignBaseline
                }
                TextField {
                    id: fuelConsumption
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    Layout.minimumWidth: Qt.application.font.pixelSize*5
                    validator: DoubleValidator {
                        bottom: aircraft.minFuelConsuption
                        top: aircraft.maxFuelConsuption
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: {
                        focus = false
                        aircraft.fuelConsumptionInLPH = text
                    }
                    color: (acceptableInput ? Material.foreground : "red")
                    KeyNavigation.tab: windDirection
                    KeyNavigation.backtab: descentSpeed
                    text: isFinite(aircraft.fuelConsumptionInLPH) ? aircraft.fuelConsumptionInLPH.toString() : ""
                    placeholderText: qsTr("undefined")
                }
                Label {
                    text: qsTr("l/h")
                    Layout.alignment: Qt.AlignBaseline
                }
                ToolButton {
                    icon.source: "/icons/material/ic_clear.svg"
                    Layout.alignment: Qt.AlignVCenter
                    enabled: fuelConsumption.text !== ""
                    onClicked: {
                        aircraft.fuelConsumptionInLPH = -1
                        fuelConsumption.clear()
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
                text: globalSettings.useMetricUnits ? flightRoute.summaryMetric : flightRoute.summary
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                visible: flightRoute.summary !== ""
            }

            ToolButton {
                id: addWPButton

                Material.foreground: Material.accent

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
            textFormat: Text.StyledText
        }

        onAccepted: {
            mobileAdaptor.vibrateBrief()
            flightRoute.clear()
            toast.doToast(qsTr("Flight route cleared"))
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
            textFormat: Text.StyledText
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

    WaypointDescription {
        id: waypointDescription
    }

    WaypointEditor {
        id: wpEditor
    }

} // Page
