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
    title: qsTr("Route and Wind")

    Component {
        id: waypointComponent

        RowLayout {
            id: waypointLayout

            property var waypoint: ({})
            property int index: -1

            WordWrappingItemDelegate {
                icon.source: waypoint.icon
                Layout.fillWidth: true
                text: waypoint.twoLineTitle

                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    waypointDescription.waypoint = waypoint
                    waypointDescription.open()
                }
            }

            ToolButton {
                id: editButton

                visible: waypoint.icon.indexOf("WP") !== -1
                icon.source: "/icons/material/ic_mode_edit.svg"
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    wpEditor.waypoint = waypoint
                    wpEditor.index = waypointLayout.index
                    wpEditor.open()
                }
            }

            ToolButton {
                id: wpMenuTB

                icon.source: "/icons/material/ic_more_horiz.svg"
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    wpMenu.popup()
                }

                Menu {
                    id: wpMenu

                    Action {
                        text: qsTr("Move Up")

                        enabled: index > 0
                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            global.navigator().flightRoute.moveUp(index)
                        }
                    }

                    Action {
                        text: qsTr("Move Down")

                        enabled: index < global.navigator().flightRoute.size-1
                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            global.navigator().flightRoute.moveDown(index)
                        }
                    }

                    Action {
                        text: qsTr("Remove")

                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            global.navigator().flightRoute.removeWaypoint(index)
                        }
                    }

                    Rectangle {
                        height: 1
                        Layout.fillWidth: true
                        color: Material.primary
                    }

                    Action {
                        text: qsTr("Add to waypoint library")
                        enabled: {
                            // Mention waypoints, in order to update
                            global.waypointLibrary().waypoints

                            return (waypoint.category === "WP") && !global.waypointLibrary().hasNearbyEntry(waypoint)
                        }

                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            global.waypointLibrary().add(waypoint)
                            toast.doToast(qsTr("Added %1 to waypoint library.").arg(waypoint.extendedName))
                        }
                    }

                }
            }

        }
    }

    Component {
        id: legComponent

        ColumnLayout {
            id: grid

            property var leg: ({});

            Layout.fillWidth: true

            ItemDelegate {
                icon.source: "/icons/vertLine.svg"
                Layout.fillWidth: true
                enabled: false
                text: {
                    // Mention units
                    global.navigator().aircraft.horizontalDistanceUnit
                    global.navigator().aircraft.fuelConsumptionUnit

                    if (leg === null)
                        return ""
                    return leg.description(global.navigator().wind, global.navigator().aircraft)
                }
            }

        }
    }


    header: ToolBar {

        Material.foreground: "white"
        height: 60

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                global.platformAdaptor().vibrateBrief()
                stackView.pop()
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.pixelSize: 20
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            visible: (sv.currentIndex === 0)
            icon.source: "/icons/material/ic_more_vert.svg"
            onClicked: {
                global.platformAdaptor().vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu {
                id: headerMenuX
                cascade: true

                MenuItem {
                    text: qsTr("View Library…")
                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        highlighted = false
                        stackView.push("FlightRouteLibrary.qml")
                    }
                }

                MenuItem {
                    text: qsTr("Save to library…")
                    enabled: (global.navigator().flightRoute.size > 0) && (sv.currentIndex === 0)
                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        highlighted = false
                        dialogLoader.active = false
                        dialogLoader.source = "../dialogs/FlightRouteSaveDialog.qml"
                        dialogLoader.active = true
                    }
                }

                MenuSeparator { }

                MenuItem {
                    text: qsTr("Import…")
                    enabled: Qt.platform.os !== "android"
                    height: Qt.platform.os !== "android" ? undefined : 0

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        highlighted = false

                        global.platformAdaptor().importContent()
                    }
                }

                AutoSizingMenu {
                    title: Qt.platform.os === "android" ? qsTr("Share…") : qsTr("Export…")
                    enabled: (global.navigator().flightRoute.size > 0) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… to GeoJSON file")
                        onTriggered: {
                            headerMenuX.close()
                            global.platformAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = global.platformAdaptor().shareContent(global.navigator().flightRoute.toGeoJSON(), "application/geo+json", global.navigator().flightRoute.suggestedFilename())
                            if (errorString === "abort") {
                                toast.doToast(qsTr("Aborted"))
                                return
                            }
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                                return
                            }
                            if (Qt.platform.os === "android")
                                toast.doToast(qsTr("Flight route shared"))
                            else
                                toast.doToast(qsTr("Flight route exported"))
                        }
                    }

                    MenuItem {
                        text: qsTr("… to GPX file")
                        onTriggered: {
                            headerMenuX.close()
                            global.platformAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = global.platformAdaptor().shareContent(global.navigator().flightRoute.toGpx(), "application/gpx+xml", global.navigator().flightRoute.suggestedFilename())
                            if (errorString === "abort") {
                                toast.doToast(qsTr("Aborted"))
                                return
                            }
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                                return
                            }
                            if (Qt.platform.os === "android")
                                toast.doToast(qsTr("Flight route shared"))
                            else
                                toast.doToast(qsTr("Flight route exported"))
                        }
                    }
                }

                AutoSizingMenu {
                    title: qsTr("Open in Other App…")
                    enabled: (global.navigator().flightRoute.size > 0) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… in GeoJSON format")

                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = global.platformAdaptor().viewContent(global.navigator().flightRoute.toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
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
                            global.platformAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = global.platformAdaptor().viewContent(global.navigator().flightRoute.toGpx(), "application/gpx+xml", "FlightRoute-%1.gpx")
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
                    enabled: (global.navigator().flightRoute.size > 0) && (sv.currentIndex === 0)

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        highlighted = false
                        clearDialog.open()
                    }

                }

                MenuItem {
                    text: qsTr("Reverse")
                    enabled: (global.navigator().flightRoute.size > 0) && (sv.currentIndex === 0)

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        highlighted = false
                        global.navigator().flightRoute.reverse()
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

        Material.elevation: 3
    }

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

                visible: global.navigator().flightRoute.size === 0

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment : Text.AlignVCenter
                textFormat: Text.RichText
                wrapMode: Text.Wrap

                leftPadding: view.font.pixelSize*2
                rightPadding: view.font.pixelSize*2

                text: qsTr("<h3>Empty Route</h3><p>Use the button <strong>Add Waypoint</strong> below or double click on any point in the moving map.</p>")
            }

            ScrollView {
                anchors.fill: parent

                contentHeight: co.height
                contentWidth: parent.width

                // The visibility behavior of the vertical scroll bar is a little complex.
                // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

                clip: true

                ColumnLayout {
                    id: co
                    width: parent.width

                    Connections {
                        target: global.navigator().flightRoute
                        function onWaypointsChanged() {
                            co.createItems()
                        }
                    }

                    Component.onCompleted: co.createItems()

                    function createItems() {
                        // Delete old text items
                        co.children = {}

                        if (global.navigator().flightRoute.size > 0) {
                            // Create first waypointComponent
                            waypointComponent.createObject(co, {waypoint: global.navigator().flightRoute.waypoints[0], index: 0});

                            // Create leg description items
                            var legs = global.navigator().flightRoute.legs
                            var j
                            for (j=0; j<legs.length; j++) {
                                legComponent.createObject(co, {leg: legs[j]});
                                waypointComponent.createObject(co, {waypoint: legs[j].endPoint, index: j+1});
                            }
                        }
                    }
                } // ColumnLayout

            }

        }

        ScrollView {
            id: windTab

            contentWidth: width
            clip: true

            GridLayout {
                anchors.left: parent.left
                anchors.leftMargin: view.font.pixelSize
                anchors.right: parent.right
                anchors.rightMargin: view.font.pixelSize

                columns: 4

                Label { Layout.fillHeight: true }
                Label {
                    text: qsTr("Wind")
                    Layout.columnSpan: 4
                    font.pixelSize: view.font.pixelSize*1.2
                    font.bold: true
                    color: Material.accent
                }

                Label {
                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("Direction from")
                }
                TextField {
                    id: windDirection
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    Layout.minimumWidth: view.font.pixelSize*5
                    validator: IntValidator {
                        bottom: 0
                        top: 360
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: {
                        global.navigator().wind.directionFrom = angle.fromDEG(text)
                        windSpeed.focus = true
                    }
                    color: (acceptableInput ? Material.foreground : "red")
                    KeyNavigation.tab: windSpeed
                    text: {
                        if (!global.navigator().wind.directionFrom.isFinite()) {
                            return ""
                        }
                        return Math.round( global.navigator().wind.directionFrom.toDEG() )
                    }
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
                        global.navigator().wind.directionFrom = angle.nan()
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
                    Layout.minimumWidth: view.font.pixelSize*5
                    validator: DoubleValidator {
                        bottom: {
                            switch(global.navigator().aircraft.horizontalDistanceUnit) {
                            case Aircraft.NauticalMile:
                                return global.navigator().wind.minWindSpeed.toKN()
                            case Aircraft.Kilometer:
                                return global.navigator().wind.minWindSpeed.toKMH()
                            case Aircraft.StatuteMile :
                                return global.navigator().wind.minWindSpeed.toMIL()
                            }
                            return NaN
                        }
                        top: {
                            switch(global.navigator().aircraft.horizontalDistanceUnit) {
                            case Aircraft.NauticalMile:
                                return global.navigator().wind.maxWindSpeed.toKN()
                            case Aircraft.Kilometer:
                                return global.navigator().wind.maxWindSpeed.toKMH()
                            case Aircraft.StatuteMile :
                                return global.navigator().wind.maxWindSpeed.toMIL()
                            }
                            return NaN
                        }
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: {
                        switch(global.navigator().aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            global.navigator().wind.speed = speed.fromKN(text)
                            break;
                        case Aircraft.Kilometer:
                            global.navigator().wind.speed = speed.fromKMH(text)
                            break;
                        case Aircraft.StatuteMile :
                            global.navigator().wind.speed = speed.fromMPH(text)
                            break;
                        }
                        focus = false
                    }
                    color: (acceptableInput ? Material.foreground : "red")
                    text: {
                        if (!global.navigator().wind.speed.isFinite()) {
                            return ""
                        }
                        switch(global.navigator().aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return Math.round( global.navigator().wind.speed.toKN() )
                        case Aircraft.Kilometer:
                            return Math.round( global.navigator().wind.speed.toKMH() )
                        case Aircraft.StatuteMile :
                            return Math.round( global.navigator().wind.speed.toMPH() )
                        }
                        return NaN
                    }
                    placeholderText: qsTr("undefined")
                }
                Label {
                    text: {
                        switch(global.navigator().aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return "kn"
                        case Aircraft.Kilometer:
                            return "km/h"
                        case Aircraft.StatuteMile:
                            return "mph"
                        }
                        return ""

                    }
                    Layout.alignment: Qt.AlignBaseline
                }
                ToolButton {
                    icon.source: "/icons/material/ic_clear.svg"
                    Layout.alignment: Qt.AlignVCenter
                    enabled: windSpeed.text !== ""
                    onClicked: {
                        global.navigator().wind.speed = speed.fromKN(-1)
                        windSpeed.clear()
                    }
                }

            }

        }

    }


    footer: Pane {
        width: parent.width
        Material.elevation: 3

        ColumnLayout {
            width: parent.width


            Label {
                Layout.fillWidth: true
                text: qsTr("One waypoint: direct route from ownship position")

                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                visible: (global.navigator().flightRoute.size === 1)&&(sv.currentIndex === 0)
            }

            Label {
                id: summary

                Layout.fillWidth: true
                text: global.navigator().flightRoute.summary
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                visible: text !== ""
            }

            ToolButton {
                id: addWPButton

                Material.foreground: Material.accent

                visible: (sv.currentIndex === 0)
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Add Waypoint")
                icon.source: "/icons/material/ic_add_circle.svg"

                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    flightRouteAddWPDialog.open()
                }
            }
        }
    }

    FlightRouteAddWPDialog {
        id: flightRouteAddWPDialog

        Connections {
            target: global.demoRunner()

            function onRequestOpenFlightRouteAddWPDialog() {
                flightRouteAddWPDialog.open()
            }
        }

    }

    Dialog {
        id: clearDialog

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 5.15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Clear Route?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-view.font.pixelSize, 40*view.font.pixelSize)
        height: Math.min(parent.height-view.font.pixelSize, implicitHeight)

        Label {
            width: clearDialog.availableWidth

            text: qsTr("Once erased, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            global.navigator().flightRoute.clear()
            toast.doToast(qsTr("Flight route cleared"))
        }
        onRejected: {
            global.platformAdaptor().vibrateBrief()
            close()
        }
    }

    Loader {
        id: dlgLoader
        anchors.fill: parent

        property string title
        property string text
        property var waypoint

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

        title: qsTr("Error Exporting Data…")
        width: Math.min(parent.width-view.font.pixelSize, 40*view.font.pixelSize)

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

        property int index: -1 // Index of waypoint in flight route

        onAccepted: {
            global.navigator().flightRoute.renameWaypoint(index, newName)
            global.navigator().flightRoute.relocateWaypoint(index, newLatitude, newLongitude)
            close()
        }
    }

} // Page
