/***************************************************************************
 *   Copyright (C) 2026 by Stefan Kebekus                                  *
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import akaflieg_freiburg.enroute

import "../dialogs"
import "../items"

Page {
    id: page
    title: qsTr("Flight Log")

    property bool isAndroidOrIos: Qt.platform.os === "android" || Qt.platform.os === "ios"

    // Selection state
    property bool selectionMode: false
    property var selectedIndices: []

    function toggleSelection(idx) {
        var arr = page.selectedIndices.slice()
        var pos = arr.indexOf(idx)
        if (pos >= 0) {
            arr.splice(pos, 1)
        } else {
            arr.push(idx)
        }
        page.selectedIndices = arr
    }

    function exitSelectionMode() {
        page.selectionMode = false
        page.selectedIndices = []
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
                if (page.selectionMode) {
                    page.exitSelectionMode()
                } else {
                    stackView.pop()
                }
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: page.selectionMode
                  ? (page.selectedIndices.length === 0
                     ? qsTr("Select flights")
                     : qsTr("%1 selected").arg(page.selectedIndices.length))
                  : stackView.currentItem.title
            elide: Label.ElideRight
            font.pixelSize: 20
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_more_vert.svg"
            onClicked: {
                PlatformAdaptor.vibrateBrief()
                headerMenuX.open()
            }

            AutoSizingMenu {
                id: headerMenuX
                cascade: true

                MenuItem {
                    text: qsTr("Automatic flight detection")
                    checkable: true
                    checked: GlobalSettings.autoFlightDetection
                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        GlobalSettings.autoFlightDetection = checked
                    }
                }

                MenuItem {
                    text: qsTr("Record GPS track")
                    checkable: true
                    enabled: GlobalSettings.autoFlightDetection
                    checked: GlobalSettings.autoFlightDetection && FlightLog.trackRecording
                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        FlightLog.trackRecording = checked
                    }
                }

                MenuItem {
                    text: qsTr("Show live flight trace on map")
                    checkable: true
                    enabled: GlobalSettings.autoFlightDetection && FlightLog.trackRecording
                    checked: GlobalSettings.autoFlightDetection && FlightLog.trackRecording && FlightLog.showCurrentFlightTrace
                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        FlightLog.showCurrentFlightTrace = checked
                    }
                }

                MenuSeparator {}

                MenuItem {
                    text: page.isAndroidOrIos ? qsTr("Share as ForeFlight CSV…") : qsTr("Export as ForeFlight CSV…")
                    enabled: FlightLog.count > 0

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        var errorString = FileExchange.shareContent(FlightLog.exportToForeFlight(page.selectedIndices), "text/*", "csv", qsTr("FlightLog"))
                        if (errorString === "abort") { toast.doToast(qsTr("Aborted")); return }
                        if (errorString !== "") { shareErrorDialog.text = errorString; shareErrorDialog.open(); return }
                        toast.doToast(page.isAndroidOrIos ? qsTr("Flight log shared") : qsTr("Flight log exported"))
                        page.exitSelectionMode()
                    }
                }

                MenuItem {
                    text: page.isAndroidOrIos ? qsTr("Share as Flightlog JSON…") : qsTr("Export as Flightlog JSON…")
                    enabled: FlightLog.count > 0

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        var errorString = FileExchange.shareContent(FlightLog.exportToJSON(page.selectedIndices), "text/*", "json", qsTr("FlightLog"))
                        if (errorString === "abort") { toast.doToast(qsTr("Aborted")); return }
                        if (errorString !== "") { shareErrorDialog.text = errorString; shareErrorDialog.open(); return }
                        toast.doToast(page.isAndroidOrIos ? qsTr("Flight log shared") : qsTr("Flight log exported"))
                        page.exitSelectionMode()
                    }
                }

                MenuItem {
                    text: qsTr("Hide Track from Map")
                    enabled: FlightLog.displayedTrackIndex >= 0

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        FlightLog.hideTrack()
                    }
                }

                MenuItem {
                    text: page.selectionMode ? qsTr("Remove Selected Flights…") : qsTr("Clear Flight Log")
                    enabled: page.selectionMode ? page.selectedIndices.length > 0 : FlightLog.count > 0

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        if (page.selectionMode) {
                            removeSelectedDialog.open()
                        } else {
                            clearDialog.open()
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right

        // Detection status indicator
        Label {
            id: detectionStatusLabel

            Layout.fillWidth: true
            Layout.leftMargin: font.pixelSize
            Layout.rightMargin: font.pixelSize
            Layout.topMargin: font.pixelSize / 4.0

            visible: FlightLog.detectionState !== FlightDetector.Idle
            horizontalAlignment: Text.AlignHCenter

            font.pixelSize: 14
            opacity: 0.8
            color: FlightLog.detectionState === FlightDetector.InFlight ? "#4CAF50"
                 : FlightLog.detectionState === FlightDetector.LandingPhase ? "#2196F3"
                 : "#FF9800"
            text: {
                switch (FlightLog.detectionState) {
                case FlightDetector.TakeoffPhase:
                    return "✈ " + qsTr("Takeoff detected — confirming altitude…")
                case FlightDetector.InFlight:
                    return "✈ " + (FlightLog.trackRecording
                        ? qsTr("In flight — recording…")
                        : qsTr("In flight"))
                case FlightDetector.LandingPhase:
                    return "✈ " + qsTr("Landing detected — confirming…")
                case FlightDetector.Idle:
                default:
                    return ""
                }
            }
        }

        // Manual "End Flight" button, visible when in flight and auto-detection
        // cannot detect the landing (e.g. no nearby ICAO airport, GPS issues).
        Button {
            Layout.alignment: Qt.AlignHCenter
            Layout.topMargin: 4
            Layout.bottomMargin: 4

            visible: FlightLog.detectionState === FlightDetector.InFlight
                  || FlightLog.detectionState === FlightDetector.LandingPhase
            text: qsTr("End Flight")
            icon.source: "/icons/material/ic_flight_land.svg"
            highlighted: true

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                endFlightDialog.open()
            }
        }

        DecoratedListView {
            id: flightList

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true

            model:
            Binding {
                flightList.model: FlightLog.flights
                delayed: true
            }

            delegate: flightDelegate
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true

            visible: (flightList.count === 0)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            leftPadding: font.pixelSize * 2
            rightPadding: font.pixelSize * 2

            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: qsTr("<h3>No flights recorded</h3><p>Flights will be automatically recorded when takeoff and landing are detected near airfields. You can also add flights manually using the button below.</p>")
        }
    }

    Component {
        id: flightDelegate

        RowLayout {
            width: flightList.width
            height: iDel.height

            // Checkbox visible in selection mode
            CheckBox {
                visible: page.selectionMode
                checked: page.selectedIndices.indexOf(index) >= 0
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    page.toggleSelection(index)
                }
            }

            SwipeToDeleteDelegate {
                id: iDel
                Layout.fillWidth: true

                contentItem: ColumnLayout {
                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: iDel.font.pixelSize
                    anchors.rightMargin: iDel.font.pixelSize
                    anchors.verticalCenter: parent.verticalCenter
                    spacing: 2

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 4

                        Label {
                            Layout.fillWidth: true
                            text: {
                                var dep = modelData.departureICAO !== "" ? modelData.departureICAO : "?"
                                var arr = modelData.arrivalICAO !== "" ? modelData.arrivalICAO : "?"
                                return dep + "  \u2192  " + arr
                            }
                            font.bold: true
                            font.pixelSize: iDel.font.pixelSize * 1.1
                            elide: Text.ElideRight
                        }

                        Image {
                            visible: modelData.hasTrack
                            source: "/icons/material/ic_timeline.svg"
                            sourceSize.width: iDel.font.pixelSize * 1.1
                            sourceSize.height: iDel.font.pixelSize * 1.1
                            opacity: 0.55
                        }
                    }

                    Label {
                        Layout.fillWidth: true
                        text: {
                            if (!modelData.startTime.getTime || isNaN(modelData.startTime.getTime()))
                                return qsTr("No time data")
                            function pad2(n) { return n.toString().padStart(2, '0') }
                            var st = modelData.startTime
                            var dateStr = st.getUTCFullYear() + "-" + pad2(st.getUTCMonth()+1) + "-" + pad2(st.getUTCDate())
                            var startStr = pad2(st.getUTCHours()) + ":" + pad2(st.getUTCMinutes())
                            var lt = modelData.landingTime
                            var landStr = (lt.getTime && !isNaN(lt.getTime()))
                                ? pad2(lt.getUTCHours()) + ":" + pad2(lt.getUTCMinutes()) : "--:--"
                            return dateStr + "   " + startStr + " – " + landStr + " UTC"
                        }
                        font.pixelSize: iDel.font.pixelSize * 0.9
                        opacity: 0.7
                        elide: Text.ElideRight
                    }

                    Label {
                        Layout.fillWidth: true
                        visible: text !== ""
                        text: {
                            var parts = []
                            var ft = modelData.flightTime()
                            if (modelData.aircraftCallsign !== "")
                                parts.push(modelData.aircraftCallsign)
                            if (ft !== "")
                                parts.push(qsTr("Duration: %1").arg(ft))
                            var bt = modelData.blockTime()
                            if (bt !== "")
                                parts.push(qsTr("Block: %1").arg(bt))
                            var dist = modelData.distance()
                            if (modelData.landingCount > 1)
                                parts.push(qsTr("Landings: %1").arg(modelData.landingCount))
                            if (dist.isFinite() && dist.toNM() >= 0.1)
                                parts.push(qsTr("Distance: %1").arg(Navigator.aircraft.horizontalDistanceToString(dist)))
                            if (modelData.pilotName !== "")
                                parts.push(modelData.pilotName)
                            if (modelData.comments !== "")
                                parts.push(modelData.comments)
                            return parts.join("  |  ")
                        }
                        wrapMode: Text.Wrap
                        font.pixelSize: iDel.font.pixelSize * 0.85
                        opacity: 0.6
                        elide: Text.ElideRight
                    }
                }

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    if (page.selectionMode) {
                        page.toggleSelection(index)
                    }
                }

                TapHandler {
                    longPressThreshold: 0.8
                    onLongPressed: {
                        PlatformAdaptor.vibrateBrief()
                        if (!page.selectionMode) {
                            page.selectionMode = true
                        }
                        page.toggleSelection(index)
                    }
                }

                swipe.onCompleted: {
                    PlatformAdaptor.vibrateBrief()
                    removeDialog.flightIndex = index
                    removeDialog.open()
                }
            }

            ToolButton {
                id: editButton

                visible: !page.selectionMode
                icon.source: "/icons/material/ic_mode_edit.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    flightEditor.flightIndex = index
                    flightEditor.editFlight = modelData
                    flightEditor.open()
                }
            }

            ToolButton {
                id: cptMenuButton

                visible: !page.selectionMode
                icon.source: "/icons/material/ic_more_horiz.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    cptMenu.open()
                }

                AutoSizingMenu {
                    id: cptMenu

                    Action {
                        text: FlightLog.displayedTrackIndex === index ? qsTr("Hide from Map") : qsTr("Show on Map")
                        enabled: modelData.hasTrack

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            if (FlightLog.displayedTrackIndex === index) {
                                FlightLog.hideTrack()
                            } else {
                                FlightLog.showTrack(index)
                            }
                        }
                    }

                    Action {
                        text: page.isAndroidOrIos ? qsTr("Share as ForeFlight CSV…") : qsTr("Export as ForeFlight CSV…")

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            var errorString = FileExchange.shareContent(FlightLog.exportToForeFlight([index]), "text/*", "csv", qsTr("FlightLog"))
                            if (errorString === "abort") { toast.doToast(qsTr("Aborted")); return }
                            if (errorString !== "") { shareErrorDialog.text = errorString; shareErrorDialog.open(); return }
                            toast.doToast(page.isAndroidOrIos ? qsTr("Flight shared") : qsTr("Flight exported"))
                        }
                    }

                    Action {
                        text: page.isAndroidOrIos ? qsTr("Share as Flightlog JSON…") : qsTr("Export as Flightlog JSON…")

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            var errorString = FileExchange.shareContent(FlightLog.exportToJSON([index]), "text/*", "json", qsTr("FlightLog"))
                            if (errorString === "abort") { toast.doToast(qsTr("Aborted")); return }
                            if (errorString !== "") { shareErrorDialog.text = errorString; shareErrorDialog.open(); return }
                            toast.doToast(page.isAndroidOrIos ? qsTr("Flight shared") : qsTr("Flight exported"))
                        }
                    }

                    Action {
                        text: page.isAndroidOrIos ? qsTr("Share to IGC…") : qsTr("Export to IGC…")
                        enabled: modelData.hasTrack

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            var errorString = FileExchange.shareContent(FlightLog.exportToIGC(index), "text/*", "igc", qsTr("FlightLog"))
                            if (errorString === "abort") { toast.doToast(qsTr("Aborted")); return }
                            if (errorString !== "") { shareErrorDialog.text = errorString; shareErrorDialog.open(); return }
                            toast.doToast(page.isAndroidOrIos ? qsTr("Track shared") : qsTr("Track exported"))
                        }
                    }

                    Action {
                        text: qsTr("Delete IGC Track…")
                        enabled: modelData.hasTrack

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            removeTrackDialog.flightIndex = index
                            removeTrackDialog.open()
                        }
                    }

                    Action {
                        text: qsTr("Remove…")

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            removeDialog.flightIndex = index
                            removeDialog.open()
                        }
                    }
                }
            }
        }
    }

    footer: Footer {
        ColumnLayout {
            width: parent.width

            Button {
                flat: true
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Add Flight")
                icon.source: "/icons/material/ic_add_circle.svg"

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    addFlightEditor.flightIndex = -1

                    // Prefill aircraft callsign from current aircraft
                    var acName = Navigator.aircraft.name

                    // Prefill departure with last arrival of this aircraft
                    var lastArr = FlightLog.lastArrivalICAO(acName)

                    // Prefill arrival with currently nearest airfield (if not in flight and GPS available)
                    var nearestAD = ""
                    if (FlightLog.detectionState === FlightDetector.Idle) {
                        nearestAD = FlightLog.nearestAirfield().shortName
                    }

                    addFlightEditor.editFlight = FlightLog.createFlight(lastArr, nearestAD, "", "", "", "", "", "", acName, "")
                    addFlightEditor.open()
                }
            }
        }
    }


    //
    // Dialogs
    //

    LongTextDialog {
        id: shareErrorDialog

        title: qsTr("Error Sharing Data…")
        standardButtons: Dialog.Ok
    }

    LongTextDialog {
        id: removeTrackDialog

        property int flightIndex: -1

        title: qsTr("Delete Track?")
        text: qsTr("Once deleted, the recorded track data cannot be restored.")
        standardButtons: Dialog.No | Dialog.Yes

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            FlightLog.removeTrack(removeTrackDialog.flightIndex)
            toast.doToast(qsTr("Track deleted"))
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
        }
    }

    LongTextDialog {
        id: removeDialog

        property int flightIndex: -1

        title: qsTr("Remove Flight?")
        text: qsTr("Once removed, this flight record cannot be restored.")
        standardButtons: Dialog.No | Dialog.Yes

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            FlightLog.removeFlight(removeDialog.flightIndex)
            toast.doToast(qsTr("Flight removed"))
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
        }
    }

    LongTextDialog {
        id: clearDialog

        title: qsTr("Clear Flight Log?")
        text: qsTr("Once cleared, the flight log cannot be restored.")
        standardButtons: Dialog.No | Dialog.Yes

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            while (FlightLog.count > 0) {
                FlightLog.removeFlight(0)
            }
            toast.doToast(qsTr("Flight log cleared"))
        }
    }

    LongTextDialog {
        id: removeSelectedDialog

        title: qsTr("Remove Selected Flights?")
        text: qsTr("Once removed, the selected flight records cannot be restored.")
        standardButtons: Dialog.No | Dialog.Yes

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            // Remove in descending order so indices stay valid
            var sorted = page.selectedIndices.slice().sort(function(a, b) { return b - a })
            for (var i = 0; i < sorted.length; i++) {
                FlightLog.removeFlight(sorted[i])
            }
            toast.doToast(qsTr("Flights removed"))
            page.exitSelectionMode()
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
        }
    }

    LongTextDialog {
        id: endFlightDialog

        title: qsTr("End Flight?")
        text: qsTr("This will set the landing time to the current UTC time. You can edit the flight entry afterwards to correct the details.")
        standardButtons: Dialog.No | Dialog.Yes

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            FlightLog.endFlight()
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
        }
    }

    FlightLogEntryEditor {
        id: flightEditor

        title: qsTr("Edit Flight")

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            FlightLog.updateFlight(flightEditor.flightIndex, flightEditor.resultFlight())
            toast.doToast(qsTr("Flight updated"))
        }
    }

    FlightLogEntryEditor {
        id: addFlightEditor

        title: qsTr("Add Flight")

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            FlightLog.addFlight(addFlightEditor.resultFlight())
            toast.doToast(qsTr("Flight added"))
        }
    }

} // Page
