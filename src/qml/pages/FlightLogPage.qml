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

            icon.source: "/icons/material/ic_more_vert.svg"
            onClicked: {
                PlatformAdaptor.vibrateBrief()
                headerMenuX.open()
            }

            AutoSizingMenu {
                id: headerMenuX
                cascade: true

                MenuItem {
                    text: qsTr("Clear Flight Log")
                    enabled: FlightLog.count > 0

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        clearDialog.open()
                    }
                }
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right

        // Auto-detection toggle
        SwitchDelegate {
            id: autoDetectSwitch

            Layout.fillWidth: true

            text: qsTr("Automatic flight detection")
            checked: GlobalSettings.autoFlightDetection

            onToggled: {
                PlatformAdaptor.vibrateBrief()
                GlobalSettings.autoFlightDetection = checked
            }
        }

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
                    return "✈ " + qsTr("In flight — recording…")
                case FlightDetector.LandingPhase:
                    return "✈ " + qsTr("Landing detected — confirming…")
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

                    Label {
                        Layout.fillWidth: true
                        text: {
                            var dep = modelData.departureICAO !== "" ? modelData.departureICAO : "?"
                            var arr = modelData.arrivalICAO !== "" ? modelData.arrivalICAO : "?"
                            return dep + "  →  " + arr
                        }
                        font.bold: true
                        font.pixelSize: iDel.font.pixelSize * 1.1
                        elide: Text.ElideRight
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
                            if (ft !== "")
                                parts.push(qsTr("Duration: %1").arg(ft))
                            var dist = modelData.distance()
                            if (dist.isFinite())
                                parts.push(qsTr("Distance: %1").arg(Navigator.aircraft.horizontalDistanceToString(dist)))
                            if (modelData.aircraftCallsign !== "")
                                parts.push(modelData.aircraftCallsign)
                            if (modelData.landingCount > 1)
                                parts.push(qsTr("Landings: %1").arg(modelData.landingCount))
                            if (modelData.pilotName !== "")
                                parts.push(modelData.pilotName)
                            if (modelData.comments !== "")
                                parts.push(modelData.comments)
                            return parts.join("  |  ")
                        }
                        font.pixelSize: iDel.font.pixelSize * 0.85
                        opacity: 0.6
                        elide: Text.ElideRight
                    }
                }

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    flightEditor.flightIndex = index
                    flightEditor.editFlight = modelData
                    flightEditor.open()
                }

                swipe.onCompleted: {
                    PlatformAdaptor.vibrateBrief()
                    removeDialog.flightIndex = index
                    removeDialog.open()
                }
            }

            ToolButton {
                id: editButton

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

                icon.source: "/icons/material/ic_more_horiz.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    cptMenu.open()
                }

                AutoSizingMenu {
                    id: cptMenu

                    Action {
                        text: qsTr("Export to IGC…")
                        enabled: modelData.hasTrack

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            FlightLog.exportToIGC(index)
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
                        nearestAD = FlightLog.nearestAirfield().ICAOCode
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
