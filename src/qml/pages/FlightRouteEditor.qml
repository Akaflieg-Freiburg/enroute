/***************************************************************************
 *   Copyright (C) 2019-2026 by Stefan Kebekus                             *
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
import QtQml.Models
import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Effects
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
                            Global.dialogLoader.active = false
                            Global.dialogLoader.setSource("../dialogs/LongTextDialog.qml", {
                                                              title: qsTr("Import files"),
                                                              text: qsTr("Locate your file in the browser, then select 'Open with' from the share menu, and choose Enroute"),
                                                              standardButtons: Dialog.Ok})
                            Global.dialogLoader.active = true
                        } else if (isAndroid) {
                            FileExchange.openFilePicker("")
                        } else {
                            importFileDialog.open()
                        }
                    }
                    FileDialog {
                        id: importFileDialog

                        acceptLabel: qsTr("Import")
                        rejectLabel: qsTr("Cancel")

                        fileMode: FileDialog.OpenFile

                        // Setting a non-trivial name filter on Android means we cannot select any
                        // files at all.
                        nameFilters: Qt.platform.os === "android" ? undefined :
                                                                    [qsTr("FPL File (*.fpl)"),
                                                                     qsTr("GeoJSON File (*.geojson *.json)"),
                                                                     qsTr("GPX File (*.gpx)"),
                                                                     qsTr("PLN File (*.pln)")]

                        onAccepted: {
                            PlatformAdaptor.vibrateBrief()
                            importFileDialog.close()
                            FileExchange.processFileOpenRequest(importFileDialog.selectedFile)
                        }
                        onRejected: {
                            PlatformAdaptor.vibrateBrief()
                            importFileDialog.close()
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
                            var errorString = FileExchange.shareContent(Navigator.flightRoute.toGeoJSON(), "application/geo+json", "geojson", Navigator.flightRoute.suggestedFilename())
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
                            var errorString = FileExchange.shareContent(Navigator.flightRoute.toGpx(), "application/gpx+xml", "gpx", Navigator.flightRoute.suggestedFilename())
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

                            var errorString = FileExchange.viewContent(Navigator.flightRoute.toGeoJSON(), "application/geo+json", "geojson", "FlightRoute")
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

                            var errorString = FileExchange.viewContent(Navigator.flightRoute.toGpx(), "application/gpx+xml", "gpx", "FlightRoute")
                            if (errorString !== "") {
                                shareErrorDialogLabel.text = errorString
                                shareErrorDialog.open()
                            } else
                                toast.doToast(qsTr("Flight route opened in other app"))
                        }
                    }

                }

                MenuItem {
                    text: qsTr("Copy as Flight Plan")
                    enabled: (Navigator.flightRoute.size > 0) && (sv.currentIndex === 0)

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false

                        var flightPlanText = Navigator.flightRoute.toVfrFlightPlan()
                        if (flightPlanText !== "") {
                            var success = PlatformAdaptor.setClipboardText(flightPlanText)
                            if (success) {
                                toast.doToast(qsTr("Flight plan copied to clipboard"))
                            } else {
                                toast.doToast(qsTr("Failed to copy flight plan"))
                            }
                        } else {
                            toast.doToast(qsTr("No flight route to copy"))
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
                        if (Librarian.contains(Navigator.flightRoute)) {
                            Navigator.flightRoute.clear()
                            toast.doToast(qsTr("Flight route cleared"))
                        } else
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

            DecoratedListView {
                id: routeView

                anchors.fill: parent

                clip: true

                // Index of the waypoint currently picked up for dragging, or -1
                // while idle. Doubles as a guard that stops the list from flicking
                // while a drag is in progress.
                property int draggedIndex: -1
                interactive: draggedIndex === -1

                displaced: Transition {
                    NumberAnimation { properties: "y"; duration: 150; easing.type: Easing.OutQuad }
                }

                model: DelegateModel {
                    id: routeDelegateModel

                    model: Navigator.flightRoute.waypoints

                    delegate: Item {
                        id: dragItem

                        required property int index
                        required property waypoint modelData

                        width: routeView.width
                        height: content.height

                        // While a row is being dragged, slide it into whatever slot its
                        // centre currently overlaps, so the other rows open a gap. Driven
                        // by geometry (indexAt) rather than a DropArea: in this delegate the
                        // DropArea does not reliably receive the drag-move events.
                        function updateDrag() {
                            if (!dragHandler.active) {
                                return
                            }
                            let c = content.mapToItem(routeView.contentItem, content.width/2, content.height/2)
                            let idx = routeView.indexAt(c.x, c.y)
                            if (idx < 0) {
                                idx = (c.y <= 0) ? 0 : routeView.count-1
                            }
                            let cur = dragItem.DelegateModel.itemsIndex
                            if (idx !== cur) {
                                routeDelegateModel.items.move(cur, idx)
                            }
                        }

                        // While dragging near the top or bottom edge of the viewport,
                        // scroll the list so a waypoint can be moved across a route that is
                        // longer than the screen. Speed ramps up towards the very edge.
                        function autoScrollStep() {
                            if (!dragHandler.active) {
                                return
                            }
                            let margin = routeView.height*0.15
                            let center = content.y + content.height/2
                            let maxContentY = Math.max(0, routeView.contentHeight - routeView.height)
                            if (maxContentY <= 0) {
                                return
                            }
                            let step = 0
                            if ((center < margin) && (routeView.contentY > 0)) {
                                let pUp = Math.max(0, Math.min(1, (margin-center)/margin))
                                step = -(3 + 12*pUp)
                            } else if ((center > routeView.height-margin) && (routeView.contentY < maxContentY)) {
                                let pDown = Math.max(0, Math.min(1, (center-(routeView.height-margin))/margin))
                                step = 3 + 12*pDown
                            }
                            if (step !== 0) {
                                routeView.contentY = Math.max(0, Math.min(maxContentY, routeView.contentY + step))
                                dragItem.updateDrag()
                            }
                        }

                        Rectangle {
                            id: content

                            anchors.horizontalCenter: parent.horizontalCenter
                            anchors.verticalCenter: parent.verticalCenter
                            width: dragItem.width
                            height: itemColumn.implicitHeight
                            color: Global.pageBackgroundColor

                            onYChanged: dragItem.updateDrag()

                            // Drives edge auto-scrolling for the whole duration of a drag.
                            Timer {
                                running: dragHandler.active
                                repeat: true
                                interval: 16
                                onTriggered: dragItem.autoScrollStep()
                            }

                            // While dragging, lift the row above the others with a drop
                            // shadow and a slight shrink, so the sliding rows stay visible
                            // at the left and right edges. Both are only active mid-drag.
                            Behavior on scale { NumberAnimation { duration: 100 } }
                            layer.enabled: dragHandler.active
                            layer.effect: MultiEffect {
                                shadowEnabled: true
                                shadowColor: "#000000"
                                shadowOpacity: 0.35
                                shadowBlur: 0.7
                                shadowVerticalOffset: 4
                                blurMax: 24
                            }

                            // While dragging, detach the row from its slot and lift it
                            // above the other delegates so it can float under the finger.
                            states: State {
                                when: dragHandler.active
                                ParentChange { target: content; parent: routeView }
                                AnchorChanges {
                                    target: content
                                    anchors.horizontalCenter: undefined
                                    anchors.verticalCenter: undefined
                                }
                                PropertyChanges { content.scale: 0.95 }
                            }

                            Column {
                                id: itemColumn

                                anchors.left: parent.left
                                anchors.right: parent.right
                                anchors.top: parent.top
                                spacing: 0

                                RowLayout {
                                    width: itemColumn.width

                                    WaypointDelegate {
                                        Layout.fillWidth: true
                                        waypoint: dragItem.modelData
                                    }

                                    ToolButton {
                                        id: editButton

                                        visible: dragItem.modelData.icon.indexOf("WP") !== -1
                                        icon.source: "/icons/material/ic_mode_edit.svg"
                                        onClicked: {
                                            PlatformAdaptor.vibrateBrief()
                                            wpEditor.waypoint = dragItem.modelData
                                            wpEditor.index = dragItem.index
                                            wpEditor.open()
                                        }
                                    }

                                    // A drag handle. This must NOT be a Button: an
                                    // AbstractButton grabs the pointer and the DragHandler
                                    // inside it would never activate. A plain Item works.
                                    Item {
                                        id: dragHandle

                                        implicitWidth: editButton.implicitWidth
                                        implicitHeight: editButton.implicitHeight
                                        Layout.alignment: Qt.AlignVCenter

                                        Icon {
                                            anchors.centerIn: parent
                                            width: 24
                                            height: 24
                                            source: "/icons/material/ic_drag_handle.svg"
                                        }

                                        DragHandler {
                                            id: dragHandler

                                            target: content
                                            xAxis.enabled: false
                                            yAxis.enabled: true

                                            onActiveChanged: {
                                                if (active) {
                                                    routeView.draggedIndex = dragItem.DelegateModel.itemsIndex
                                                } else {
                                                    let from = routeView.draggedIndex
                                                    let to = dragItem.DelegateModel.itemsIndex
                                                    routeView.draggedIndex = -1
                                                    if ((from >= 0) && (from !== to)) {
                                                        PlatformAdaptor.vibrateBrief()
                                                        Navigator.flightRoute.move(from, to)
                                                    }
                                                }
                                            }
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

                                                enabled: dragItem.index > 0
                                                onTriggered: {
                                                    PlatformAdaptor.vibrateBrief()
                                                    wpMenu.close() // Necessary on some devices, or else menu will stay open

                                                    Navigator.flightRoute.moveUp(dragItem.index)
                                                }
                                            }

                                            Action {
                                                text: qsTr("Move Down")

                                                enabled: dragItem.index < Navigator.flightRoute.size-1
                                                onTriggered: {
                                                    PlatformAdaptor.vibrateBrief()
                                                    wpMenu.close() // Necessary on some devices, or else menu will stay open

                                                    Navigator.flightRoute.moveDown(dragItem.index)
                                                }
                                            }

                                            Action {
                                                text: qsTr("Remove")

                                                onTriggered: {
                                                    PlatformAdaptor.vibrateBrief()
                                                    wpMenu.close() // Necessary on some devices, or else menu will stay open

                                                    Navigator.flightRoute.removeWaypoint(dragItem.index)
                                                }
                                            }

                                            Rectangle {
                                                height: 1
                                                Layout.fillWidth: true
                                                color: Global.dividerColor
                                            }

                                            Action {
                                                text: qsTr("Add to waypoint library")
                                                enabled: {
                                                    // Mention waypoints, in order to update
                                                    WaypointLibrary.waypoints

                                                    return (dragItem.modelData.category === "WP") && !WaypointLibrary.hasNearbyEntry(dragItem.modelData)
                                                }

                                                onTriggered: {
                                                    PlatformAdaptor.vibrateBrief()
                                                    wpMenu.close() // Necessary on some devices, or else menu will stay open

                                                    WaypointLibrary.add(dragItem.modelData)
                                                    toast.doToast(qsTr("Added %1 to waypoint library.").arg(dragItem.modelData.extendedName))
                                                }
                                            }

                                        }
                                    }
                                }

                                ItemDelegate {
                                    width: itemColumn.width

                                    visible: dragItem.index < Navigator.flightRoute.size-1
                                    icon.source: "/icons/vertLine.svg"
                                    enabled: false
                                    text: {
                                        // Mention units
                                        Navigator.aircraft.horizontalDistanceUnit
                                        Navigator.aircraft.fuelConsumptionUnit

                                        // dragItem.index is transiently -1 while a delegate is
                                        // being torn down, so guard against a missing leg.
                                        let leg = Navigator.flightRoute.legs[dragItem.index]
                                        if (leg === undefined)
                                            return ""
                                        return leg.description(Navigator.wind, Navigator.aircraft)
                                    }
                                }
                            }
                        }

                    }
                }
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

    // Lets the DemoRunner switch to the "Wind" tab when generating screenshots.
    Connections {
        target: DemoRunner

        function onRequestShowWindTab() {
            sv.currentIndex = 1
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


    CenteringDialog {
        id: flightRouteAddWPDialog

        title: qsTr("Add Waypoint to Route")
        modal: true

        standardButtons: DialogButtonBox.Cancel

        // Focus the filter field on open so the user can type immediately.
        defaultFocusItem: textInput

        Component {
            id: waypointDelegate

            WordWrappingItemDelegate {
                text: model.modelData.twoLineTitle
                icon.source: model.modelData.icon

                width: wpList.width

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    Navigator.flightRoute.append(model.modelData)
                    flightRouteAddWPDialog.close()
                }
            }

        }

        ColumnLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true

                text: qsTr("Choose a waypoint from the list below or <a href='xx'>enter coordinates manually</a>.")
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
                visible: textInput.displayText === ""
                onLinkActivated: {
                    PlatformAdaptor.vibrateBrief()
                    flightRouteAddWPDialog.close()
                    addbyCoordinates.open()
                }
            }

            Item {
                Layout.preferredHeight: textInput.font.pixelSize
                visible: textInput.displayText === ""
            }

            MyTextField {
                id: textInput

                Layout.fillWidth: true

                placeholderText: qsTr("Filter by Name")

                focus: true

                onAccepted: {
                    if (wpList.model.length > 0) {
                        PlatformAdaptor.vibrateBrief()
                        Navigator.flightRoute.append(wpList.model[0])
                        flightRouteAddWPDialog.close()
                    }
                }

                // On iOS17, the property displayText sees many bounces.
                onDisplayTextChanged: debounceTimer.restart()
            }

            Label {
                Layout.fillWidth: true
                Layout.topMargin: font.pixelSize

                text: qsTr("<h3>Sorry!</h3><p>No waypoints match your filter.</p>")
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
                horizontalAlignment: Text.AlignHCenter

                visible: wpList.model.length === 0
            }

            DecoratedListView {
                id: wpList

                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight

                clip: true

                // Debounce timer to update the property model only 200ms after the last change of textInput.displayText
                Timer {
                    id: debounceTimer
                    interval: 200 // 200ms
                    onTriggered: wpList.model = GeoMapProvider.filteredWaypoints(textInput.displayText)
                }

                model: GeoMapProvider.filteredWaypoints(textInput.displayText)
                delegate: waypointDelegate

                Label {
                    anchors.fill: wpList
                    anchors.topMargin: font.pixelSize*2

                    visible: (wpList.count === 0)
                    horizontalAlignment: Text.AlignHCenter
                    textFormat: Text.StyledText
                    wrapMode: Text.Wrap
                    text: (textInput.text === "")
                          ? qsTr("<h3>Sorry!</h3><p>No waypoints available. Please make sure that an aviation map is installed.</p>")
                          : qsTr("<h3>Sorry!</h3><p>No waypoints match your filter criteria.</p>")
                    onLinkActivated: Qt.openUrlExternally(link)
                }

            }

        }

        onOpened: textInput.clear()

        Connections {
            target: DemoRunner

            function onRequestOpenFlightRouteAddWPDialog() {
                flightRouteAddWPDialog.open()
            }
        }

    }

    WaypointEditor {
        id: addbyCoordinates

        title: qsTr("Add Waypoint to Route")
        modal: true

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            let newWP = waypoint.copy()
            newWP.name = newName
            newWP.notes = newNotes
            newWP.coordinate = QtPositioning.coordinate(newLatitude, newLongitude, newAltitudeMeter)
            Navigator.flightRoute.append(newWP)
            addbyCoordinates.close()
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
            clearDialog.close()
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
            wpEditor.close()
        }
    }
} // Page
