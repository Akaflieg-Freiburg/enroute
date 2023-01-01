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

import QtPositioning
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
    id: flightRoutePage
    title: qsTr("Route and Wind")

    property speed staticSpeed

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
                            Navigator.flightRoute.moveUp(index)
                        }
                    }

                    Action {
                        text: qsTr("Move Down")

                        enabled: index < Navigator.flightRoute.size-1
                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            Navigator.flightRoute.moveDown(index)
                        }
                    }

                    Action {
                        text: qsTr("Remove")

                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            Navigator.flightRoute.removeWaypoint(index)
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
                    Navigator.aircraft.horizontalDistanceUnit
                    Navigator.aircraft.fuelConsumptionUnit

                    if (leg === null)
                        return ""
                    return leg.description(Navigator.wind, Navigator.aircraft)
                }
            }

        }
    }


    header: ToolBar {

        Material.foreground: "white"
        height: 60 + SafeInsets.top
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: SafeInsets.top

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

            Menu {}

            AutoSizingMenu {
                id: headerMenuX
                cascade: true

                topMargin: SafeInsets.top

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
                    enabled: (Navigator.flightRoute.size > 0) && (sv.currentIndex === 0)
                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        highlighted = false
                        dialogLoader.active = false
                        dialogLoader.source = "dialogs/FlightRouteSaveDialog.qml"
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

                        global.fileExchange().importContent()
                    }
                }

                AutoSizingMenu {
                    title: Qt.platform.os === "android" ? qsTr("Share…") : qsTr("Export…")
                    enabled: (Navigator.flightRoute.size > 0) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… to GeoJSON file")
                        onTriggered: {
                            headerMenuX.close()
                            global.platformAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = global.fileExchange().shareContent(Navigator.flightRoute.toGeoJSON(), "application/geo+json", Navigator.flightRoute.suggestedFilename())
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
                            var errorString = global.fileExchange().shareContent(Navigator.flightRoute.toGpx(), "application/gpx+xml", Navigator.flightRoute.suggestedFilename())
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
                    enabled: (Navigator.flightRoute.size > 0) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… in GeoJSON format")

                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = global.fileExchange().viewContent(Navigator.flightRoute.toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
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

                            var errorString = global.fileExchange().viewContent(Navigator.flightRoute.toGpx(), "application/gpx+xml", "FlightRoute-%1.gpx")
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
                    enabled: (Navigator.flightRoute.size > 0) && (sv.currentIndex === 0)

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        highlighted = false
                        clearDialog.open()
                    }

                }

                MenuItem {
                    text: qsTr("Reverse")
                    enabled: (Navigator.flightRoute.size > 0) && (sv.currentIndex === 0)

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        highlighted = false
                        Navigator.flightRoute.reverse()
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
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right

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
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right

        clip: true
        currentIndex: bar.currentIndex

        Item {
            id: flightRouteTab

            Label {
                anchors.fill: parent

                visible: Navigator.flightRoute.size === 0

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

                contentWidth: availableWidth

                clip: true

                ColumnLayout {
                    id: co
                    width: parent.width

                    Connections {
                        target: Navigator.flightRoute
                        function onWaypointsChanged() {
                            co.createItems()
                        }
                    }

                    Component.onCompleted: co.createItems()

                    function createItems() {
                        // Delete old text items
                        co.children = {}

                        if (Navigator.flightRoute.size > 0) {
                            // Create first waypointComponent
                            waypointComponent.createObject(co, {waypoint: Navigator.flightRoute.waypoints[0], index: 0});

                            // Create leg description items
                            var legs = Navigator.flightRoute.legs
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
                    property angle myAngle; // Dummy. I do not know how to create an angle otherwise
                    onEditingFinished: {
                        Navigator.wind.directionFrom = myAngle.fromDEG(text)
                        windSpeed.focus = true
                    }
                    color: (acceptableInput ? Material.foreground : "red")
                    KeyNavigation.tab: windSpeed
                    text: {
                        if (!Navigator.wind.directionFrom.isFinite()) {
                            return ""
                        }
                        return Math.round( Navigator.wind.directionFrom.toDEG() )
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
                        Navigator.wind.directionFrom = angle.nan()
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
                            switch(Navigator.aircraft.horizontalDistanceUnit) {
                            case Aircraft.NauticalMile:
                                return Navigator.wind.minWindSpeed.toKN()
                            case Aircraft.Kilometer:
                                return Navigator.wind.minWindSpeed.toKMH()
                            case Aircraft.StatuteMile :
                                return Navigator.wind.minWindSpeed.toMPH()
                            }
                            return NaN
                        }
                        top: {
                            switch(Navigator.aircraft.horizontalDistanceUnit) {
                            case Aircraft.NauticalMile:
                                return Navigator.wind.maxWindSpeed.toKN()
                            case Aircraft.Kilometer:
                                return Navigator.wind.maxWindSpeed.toKMH()
                            case Aircraft.StatuteMile :
                                return Navigator.wind.maxWindSpeed.toMPH()
                            }
                            return NaN
                        }
                        notation: DoubleValidator.StandardNotation
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    onEditingFinished: {
                        switch(Navigator.aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            Navigator.wind.speed = flightRoutePage.staticSpeed.fromKN(text)
                            break;
                        case Aircraft.Kilometer:
                            Navigator.wind.speed = flightRoutePage.staticSpeed.fromKMH(text)
                            break;
                        case Aircraft.StatuteMile :
                            Navigator.wind.speed = flightRoutePage.staticSpeed.fromMPH(text)
                            break;
                        }
                        focus = false
                    }
                    color: (acceptableInput ? Material.foreground : "red")
                    text: {
                        if (!Navigator.wind.speed.isFinite()) {
                            return ""
                        }
                        switch(Navigator.aircraft.horizontalDistanceUnit) {
                        case Aircraft.NauticalMile:
                            return Math.round( Navigator.wind.speed.toKN() )
                        case Aircraft.Kilometer:
                            return Math.round( Navigator.wind.speed.toKMH() )
                        case Aircraft.StatuteMile :
                            return Math.round( Navigator.wind.speed.toMPH() )
                        }
                        return NaN
                    }
                    placeholderText: qsTr("undefined")
                }
                Label {
                    text: {
                        switch(Navigator.aircraft.horizontalDistanceUnit) {
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
                        Navigator.wind.speed = flightRoutePage.staticSpeed.fromKN(-1)
                        windSpeed.clear()
                    }
                }

            }

        }

    }


    footer: Pane {
        width: parent.width
        height: implicitHeight
        Material.elevation: 3
        bottomPadding: SafeInsets.bottom

        ColumnLayout {
            width: parent.width

            Label {
                Layout.fillWidth: true
                text: qsTr("One waypoint: direct route from ownship position")

                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.StyledText
                visible: (Navigator.flightRoute.size === 1)&&(sv.currentIndex === 0)
            }

            Label {
                id: summary

                Layout.fillWidth: true
                text: Navigator.flightRoute.summary
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

    CenteringDialog {
        id: clearDialog

        title: qsTr("Clear Route?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            width: clearDialog.availableWidth

            text: qsTr("Once erased, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            Navigator.flightRoute.clear()
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

    CenteringDialog {
        id: shareErrorDialog

        title: qsTr("Error Exporting Data…")
        standardButtons: Dialog.Ok
        modal: true

        Label {
            id: shareErrorDialogLabel
            width: shareErrorDialog.availableWidth
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }
    }

    Shortcut {
        sequence: "Ctrl+a"
        onActivated: {
            dialogLoader.active = false
            dialogLoader.source = "dialogs/FlightRouteAddWPDialog.qml"
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
            global.platformAdaptor().vibrateBrief()

            var newWP = waypoint.copy()
            newWP.name = newName
            newWP.notes = newNotes
            newWP.coordinate = QtPositioning.coordinate(newLatitude, newLongitude, newAltitudeMeter)
            Navigator.flightRoute.replaceWaypoint(index, newWP)
            close()
        }
    }

} // Page
