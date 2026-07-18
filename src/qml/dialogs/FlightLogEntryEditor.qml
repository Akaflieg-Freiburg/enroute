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
import "../items"

/* Flight log entry editor dialog
 *
 * Usage: set property editFlight and flightUuid, then open().
 * On accepted, call resultFlight() to get the edited Flight value.
 */

CenteringDialog {
    id: flightEditorDialog

    property string flightUuid: ""
    property flight editFlight

    // Output values read by the caller
    readonly property string newDepartureICAO: depField.text.toUpperCase()
    readonly property string newArrivalICAO: arrField.text.toUpperCase()
    readonly property string newDate: dateField.text
    readonly property string newOffBlockTime: offBlockField.text
    readonly property string newStartTime: startField.text
    readonly property string newLandingTime: landingField.text
    readonly property string newOnBlockTime: onBlockField.text
    readonly property string newPilotName: pilotField.text
    readonly property string newAircraftCallsign: callsignField.text
    readonly property string newComments: commentsField.text

    modal: true
    title: qsTr("Edit Flight")

    // Auto-format a bare 4-digit time like "1800" → "18:00"
    function formatTimeInput(field) {
        var t = field.text
        if (/^\d{4}$/.test(t)) {
            field.text = t.substring(0, 2) + ":" + t.substring(2, 4)
        }
    }

    // Format a QDateTime as UTC time string "HH:mm"
    function utcTime(dt) {
        return dt.getUTCHours().toString().padStart(2, '0')
             + ":" + dt.getUTCMinutes().toString().padStart(2, '0')
    }

    // Format a QDateTime as UTC date string "yyyy-MM-dd"
    function utcDate(dt) {
        return dt.getUTCFullYear().toString()
             + "-" + (dt.getUTCMonth() + 1).toString().padStart(2, '0')
             + "-" + dt.getUTCDate().toString().padStart(2, '0')
    }

    function resultFlight() {
        var f = FlightLog.createFlight(
            newDepartureICAO,
            newArrivalICAO,
            newDate,
            newOffBlockTime,
            newStartTime,
            newLandingTime,
            newOnBlockTime,
            newPilotName,
            newAircraftCallsign,
            newComments
        )
        f.landingCount = parseInt(landingsField.text) || 1
        return f
    }

    onAboutToShow: {
        if (editFlight.startTime.getTime && !isNaN(editFlight.startTime.getTime())) {
            dateField.text = utcDate(editFlight.startTime)
            startField.text = utcTime(editFlight.startTime)
        } else {
            dateField.text = utcDate(new Date())
            startField.text = ""
        }

        if (editFlight.landingTime.getTime && !isNaN(editFlight.landingTime.getTime())) {
            landingField.text = utcTime(editFlight.landingTime)
        } else {
            landingField.text = ""
        }

        if (editFlight.offBlockTime.getTime && !isNaN(editFlight.offBlockTime.getTime())) {
            offBlockField.text = utcTime(editFlight.offBlockTime)
        } else {
            offBlockField.text = ""
        }

        if (editFlight.onBlockTime.getTime && !isNaN(editFlight.onBlockTime.getTime())) {
            onBlockField.text = utcTime(editFlight.onBlockTime)
        } else {
            onBlockField.text = ""
        }

        depField.text = editFlight.departureICAO
        arrField.text = editFlight.arrivalICAO
        pilotField.text = editFlight.pilotName
        callsignField.text = editFlight.aircraftCallsign
        commentsField.text = editFlight.comments
        landingsField.text = editFlight.landingCount > 0 ? editFlight.landingCount.toString() : "1"
        depField.focus = true
    }

    DecoratedScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        clip: true

        onHeightChanged: {
            if (activeFocusControl != null) {
                contentItem.contentY = activeFocusControl.y
            }
        }

        GridLayout {
            width: availableWidth
            columns: 2

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Date")
            }
            MyTextField {
                id: dateField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: "yyyy-MM-dd"
                inputMethodHints: Qt.ImhDate
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Departure (ICAO)")
            }
            MyTextField {
                id: depField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize * 5
                placeholderText: qsTr("e.g. EDTF")
                inputMethodHints: Qt.ImhUppercaseOnly | Qt.ImhNoPredictiveText
                maximumLength: 4
                focus: true
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Arrival (ICAO)")
            }
            MyTextField {
                id: arrField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize * 5
                placeholderText: qsTr("e.g. EDFE")
                inputMethodHints: Qt.ImhUppercaseOnly | Qt.ImhNoPredictiveText
                maximumLength: 4
            }

            // Separator
            Label {
                Layout.columnSpan: 2
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Off-Block (UTC)")
            }
            MyTextField {
                id: offBlockField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("HH:mm (optional)")
                inputMethodHints: Qt.ImhTime
                onTextEdited: formatTimeInput(offBlockField)
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Start (UTC)")
            }
            MyTextField {
                id: startField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: "HH:mm"
                inputMethodHints: Qt.ImhTime
                onTextEdited: formatTimeInput(startField)
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Landing (UTC)")
            }
            MyTextField {
                id: landingField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: "HH:mm"
                inputMethodHints: Qt.ImhTime
                onTextEdited: formatTimeInput(landingField)
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("On-Block (UTC)")
            }
            MyTextField {
                id: onBlockField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("HH:mm (optional)")
                inputMethodHints: Qt.ImhTime
                onTextEdited: formatTimeInput(onBlockField)
            }

            // Separator
            Label {
                Layout.columnSpan: 2
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Pilot")
            }
            MyTextField {
                id: pilotField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("Pilot name (optional)")
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Aircraft")
            }
            MyTextField {
                id: callsignField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("Callsign")
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Comments")
            }
            MyTextField {
                id: commentsField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("Optional comments")
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Landings")
            }
            MyTextField {
                id: landingsField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: "1"
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator { bottom: 0; top: 99 }
            }

            // Separator
            Label {
                Layout.columnSpan: 2
            }

            // Calculated values (read-only)
            Label {
                Layout.columnSpan: 2
                Layout.fillWidth: true
                font.italic: true
                opacity: 0.7
                wrapMode: Text.Wrap
                text: {
                    var f = flightEditorDialog.resultFlight()
                    var parts = []
                    var ft = f.flightTime()
                    if (ft !== "")
                        parts.push(qsTr("Duration: %1").arg(ft))
                    var bt = f.blockTime()
                    if (bt !== "")
                        parts.push(qsTr("Block: %1").arg(bt))
                    var dist = f.distance()
                    if (dist.isFinite())
                        parts.push(qsTr("Distance: %1 NM").arg(dist.toNM().toFixed(1)))
                    if (parts.length === 0)
                        return qsTr("Enter times and ICAO codes to see calculated values")
                    return parts.join("  |  ")
                }
            }
        }
    }

    footer: DialogButtonBox {

        Button {
            text: qsTr("Cancel")
            flat: true
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }

        Button {
            text: qsTr("OK")
            flat: true
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            enabled: depField.displayText !== ""
                     && dateField.displayText !== "" && startField.displayText !== ""
        }
    }

}
