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
import QtQuick.Dialogs
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../dialogs"
import "../items"

Page {
    id: flightRoutePage
    title: qsTr("Route and Wind")

    property bool isIos: Qt.platform.os === "ios"
    property bool isAndroid: Qt.platform.os === "android"
    property bool isAndroidOrIos: isAndroid || isIos
    property angle staticAngle
    property speed staticSpeed

    Component {
        id: waypointComponent

        RowLayout {
            id: waypointLayout

            property var waypoint: ({})
            property int index: -1

            WaypointDelegate {
                Layout.fillWidth: true
                waypoint: waypointLayout.waypoint
            }

            ToolButton {
                id: editButton

                visible: waypoint.icon.indexOf("WP") !== -1
                icon.source: "/icons/material/ic_mode_edit.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    wpEditor.waypoint = waypoint
                    wpEditor.index = waypointLayout.index
                    wpEditor.open()
                }
            }

            ToolButton {
                id: wpMenuTB

                icon.source: "/icons/material/ic_more_horiz.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    wpMenu.open()
                }

                AutoSizingMenu {
                    id: wpMenu

                    Action {
                        text: qsTr("Move Up")

                        enabled: index > 0
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            wpMenu.close() // Necessary on some devices, or else menu will stay open

                            Navigator.flightRoute.moveUp(index)
                        }
                    }

                    Action {
                        text: qsTr("Move Down")

                        enabled: index < Navigator.flightRoute.size-1
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            wpMenu.close() // Necessary on some devices, or else menu will stay open

                            Navigator.flightRoute.moveDown(index)
                        }
                    }

                    Action {
                        text: qsTr("Remove")

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            wpMenu.close() // Necessary on some devices, or else menu will stay open

                            Navigator.flightRoute.removeWaypoint(index)
                        }
                    }

                    Rectangle {
                        height: 1
                        Layout.fillWidth: true
                        color: "black"
                    }

                    Action {
                        text: qsTr("Add to waypoint library")
                        enabled: {
                            // Mention waypoints, in order to update
                            WaypointLibrary.waypoints

                            return (waypoint.category === "WP") && !WaypointLibrary.hasNearbyEntry(waypoint)
                        }

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            wpMenu.close() // Necessary on some devices, or else menu will stay open

                            WaypointLibrary.add(waypoint)
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

            property leg leg: ({});

            Layout.fillWidth: true

            ItemDelegate {
                icon.source: "/icons/vertLine.svg"
                Layout.fillWidth: true
                enabled: false
                text: {
                    // Mention units
                    Navigator.aircraft.horizontalDistanceUnit
                    Navigator.aircraft.fuelConsumptionUnit

                    if (leg == null)
                        return ""
                    return leg.description(Navigator.wind, Navigator.aircraft)
                }
            }

        }
    }


    header: PageHeader {

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
                PlatformAdaptor.vibrateBrief()
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
                PlatformAdaptor.vibrateBrief()
                headerMenuX.open()
            }

            Menu {}

            AutoSizingMenu {
                id: headerMenuX
                cascade: true

                topMargin: SafeInsets.top

                MenuItem {
                    text: qsTr("View Library…")
                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        stackView.push("FlightRouteLibrary.qml")
                    }
                }

                MenuItem {
                    text: qsTr("Save to library…")
                    enabled: (Navigator.flightRoute.size > 0) && (sv.currentIndex === 0)
                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        dlgLoader.active = false
                        dlgLoader.source = "../dialogs/FlightRouteSaveDialog.qml"
                        dlgLoader.active = true
                    }
                }

                MenuSeparator { }

                MenuItem {
                    id: menuImport

                    text: qsTr("Import…")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        if (isIos) {
                            iosImportHelpDialog.open()
                        } else {
                            importFileDialog.open()
                        }

                    }

                    FileImportHelpDialog {
                        id: iosImportHelpDialog
                    }

                    FileDialog {
                        id: importFileDialog

                        acceptLabel: qsTr("Import")
                        rejectLabel: qsTr("Cancel")

                        fileMode: FileDialog.OpenFile

                        // Setting a non-trivial name filter on Android means we cannot select any
                        // files at all.
                        nameFilters: Qt.platform.os === "android" ? undefined : [qsTr("GeoJSON File (*.geojson *.json)"),
                                                                                 qsTr("GPX File (*.gpx)")]

                        onAccepted: {
                            PlatformAdaptor.vibrateBrief()
                            close()
                            FileExchange.processFileOpenRequest(importFileDialog.selectedFile)
                        }
                        onRejected: {
                            PlatformAdaptor.vibrateBrief()
                            close()
                        }
                    }
                }

                AutoSizingMenu {
                    title: isAndroidOrIos ? qsTr("Share…") : qsTr("Export…")
                    enabled: (Navigator.flightRoute.size > 0) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… to GeoJSON file")
                        onTriggered: {
                            headerMenuX.close()
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = FileExchange.shareContent(Navigator.flightRoute.toGeoJSON(), "application/geo+json", Navigator.flightRoute.suggestedFilename())
                            if (errorString === "abort") {
                                toast.doToast(qsTr("Aborted"))
                                return
                            }
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                                return
                            }
                            if (isAndroid)
                                toast.doToast(qsTr("Flight route shared"))
                            else (!isIos)
                                toast.doToast(qsTr("Flight route exported"))
                        }
                    }

                    MenuItem {
                        text: qsTr("… to GPX file")
                        onTriggered: {
                            headerMenuX.close()
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false
                            var errorString = FileExchange.shareContent(Navigator.flightRoute.toGpx(), "application/gpx+xml", Navigator.flightRoute.suggestedFilename())
                            if (errorString === "abort") {
                                toast.doToast(qsTr("Aborted"))
                                return
                            }
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                                return
                            }
                            if (isAndroid)
                                toast.doToast(qsTr("Flight route shared"))
                            else
                                toast.doToast(qsTr("Flight route exported"))
                        }
                    }
                }

                AutoSizingMenu {
                    title: qsTr("Open in Other App…")
                    enabled: (Qt.platform.os !== "ios") && (Navigator.flightRoute.size > 0) && (sv.currentIndex === 0)

                    MenuItem {
                        text: qsTr("… in GeoJSON format")

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = FileExchange.viewContent(Navigator.flightRoute.toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
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
                            PlatformAdaptor.vibrateBrief()
                            highlighted = false
                            parent.highlighted = false

                            var errorString = FileExchange.viewContent(Navigator.flightRoute.toGpx(), "application/gpx+xml", "FlightRoute-%1.gpx")
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
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        clearDialog.open()
                    }

                }

                MenuItem {
                    text: qsTr("Reverse")
                    enabled: (Navigator.flightRoute.size > 1) && (sv.currentIndex === 0)

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
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

                leftPadding: font.pixelSize*2
                rightPadding: font.pixelSize*2

                text: qsTr("<h3>Empty Route</h3><p>Use the button <strong>Add Waypoint</strong> below or double click on any point in the moving map.</p>")
            }

            DecoratedScrollView {
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

        DecoratedScrollView {
            id: windTab

            contentWidth: width
            clip: true

            GridLayout {
                anchors.left: parent.left
                anchors.leftMargin: font.pixelSize
                anchors.right: parent.right
                anchors.rightMargin: font.pixelSize

                columns: 3

                Label {
                    Layout.fillHeight: true
                    Layout.columnSpan: 3
                }
                Label {
                    text: qsTr("Wind")
                    Layout.columnSpan: 3
                    font.pixelSize: windTab.font.pixelSize*1.2
                    font.bold: true
                }

                Label {
                    id: colorGlean

                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("Direction from")
                }
                MyTextField {
                    id: windDirection
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    Layout.minimumWidth: font.pixelSize*5
                    validator: IntValidator {
                        bottom: 0
                        top: 360
                    }
                    inputMethodHints: Qt.ImhDigitsOnly
                    property angle myAngle; // Dummy. I do not know how to create an angle otherwise
                    onEditingFinished: {                        
                        Navigator.wind.directionFrom = text === "" ? myAngle.fromDEG(NaN) : myAngle.fromDEG(text)
                        windSpeed.focus = true
                    }
                    color: (acceptableInput ? colorGlean.color : "red")
                    KeyNavigation.tab: windSpeed
                    text: {
                        if (!Navigator.wind.directionFrom.isFinite()) {
                            return ""
                        }
                        return Math.round( Navigator.wind.directionFrom.toDEG() )
                    }
                }
                Label {
                    text: "°"
                    Layout.alignment: Qt.AlignBaseline
                }

                Label {
                    text: qsTr("Speed")
                    Layout.alignment: Qt.AlignBaseline
                }
                MyTextField {
                    id: windSpeed
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    Layout.minimumWidth: font.pixelSize*5
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
                    color: (acceptableInput ? colorGlean.color : "red")
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

            }

        }

    }


    footer: Footer {
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

            Button {
                id: addWPButton

                visible: (sv.currentIndex === 0)
                flat: true

                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Add Waypoint")
                icon.source: "/icons/material/ic_add_circle.svg"

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    flightRouteAddWPDialog.open()
                }
            }
        }
    }

    FlightRouteAddWPDialog {
        id: flightRouteAddWPDialog

        Connections {
            target: DemoRunner

            function onRequestOpenFlightRouteAddWPDialog() {
                flightRouteAddWPDialog.open()
            }
        }

    }

    LongTextDialog {
        id: clearDialog

        title: qsTr("Clear Route?")
        standardButtons: Dialog.No | Dialog.Yes

        text: qsTr("Once erased, the current flight route cannot be restored.")

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            Navigator.flightRoute.clear()
            toast.doToast(qsTr("Flight route cleared"))
        }

        onRejected: {
            PlatformAdaptor.vibrateBrief()
            close()
        }
    }

    Loader {
        id: dlgLoader
        anchors.fill: parent

        onLoaded: {
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
            dlgLoader.active = false
            dlgLoader.source = "../dialogs/FlightRouteAddWPDialog.qml"
            dlgLoader.active = true
        }
    }

    WaypointEditor {
        id: wpEditor

        property int index: -1 // Index of waypoint in flight route

        onAccepted: {
            PlatformAdaptor.vibrateBrief()

            var newWP = waypoint.copy()
            newWP.name = newName
            newWP.notes = newNotes
            newWP.coordinate = QtPositioning.coordinate(newLatitude, newLongitude, newAltitudeMeter)
            Navigator.flightRoute.replaceWaypoint(index, newWP)
            close()
        }
    }
} // Page
