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
 * On accepted, call resultFlight() to get the edited Flight value. This is
 * editFlight with the user's changes applied, see FlightLog.applyEdits().
 */

CenteringDialog {
    id: flightEditorDialog

    property string flightUuid: ""
    property flight editFlight

    // Output values read by the caller
    readonly property string newDepartureICAO: codeOrName(depField.text)
    readonly property string newArrivalICAO: codeOrName(arrField.text)
    readonly property string newDate: dateField.date
    readonly property string newOffBlockTime: offBlockField.text
    readonly property string newStartTime: startField.text
    readonly property string newLandingTime: landingField.text
    readonly property string newOnBlockTime: onBlockField.text
    readonly property string newPilotName: pilotField.text
    readonly property string newAircraftCallsign: callsignField.text
    readonly property string newComments: commentsField.text

    // True if the input can be saved. Date and start time are mandatory. The
    // other times may be left empty but must be valid otherwise: a time that
    // FlightLog cannot interpret would be dropped from the saved flight.
    readonly property bool acceptableInput: depField.displayText !== ""
                                            && FlightLog.isValidDate(dateField.date)
                                            && FlightLog.isValidTime(startField.text)
                                            && isValidOrEmptyTime(offBlockField.text)
                                            && isValidOrEmptyTime(landingField.text)
                                            && isValidOrEmptyTime(onBlockField.text)
                                            && (landingsField.text === "" || landingsField.acceptableInput)

    // Give keyboard focus to the departure field when the dialog opens
    defaultFocusItem: depField

    modal: true
    title: qsTr("Edit Flight")

    // The airfield fields hold an ICAO code or, for an airfield without one,
    // its name: flight detection and the "Add Flight" button fill in
    // Waypoint.shortName. Codes are stored in upper case, so that they are
    // found in the maps. Anything longer than a code is a name and is kept as
    // entered.
    function codeOrName(text) {
        var trimmed = text.trim()
        return trimmed.length <= 4 ? trimmed.toUpperCase() : trimmed
    }

    // Check the text of a time field that may be left empty
    function isValidOrEmptyTime(text) {
        return text.trim() === "" || FlightLog.isValidTime(text)
    }

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
             + "-" + String(dt.getUTCMonth() + 1).padStart(2, '0')
             + "-" + dt.getUTCDate().toString().padStart(2, '0')
    }

    // The flight as it will be saved: editFlight with the content of the
    // fields applied. The summary at the bottom of the dialog shows this
    // flight, too.
    function resultFlight() {
        var edited = FlightLog.createFlight(
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
        // A flight has at least one landing. An empty field means one.
        edited.landingCount = Math.max(1, parseInt(landingsField.text) || 1)
        return FlightLog.applyEdits(editFlight, edited)
    }

    onAboutToShow: {
        if (editFlight.startTime.getTime && !isNaN(editFlight.startTime.getTime())) {
            dateField.date = utcDate(editFlight.startTime)
            startField.text = utcTime(editFlight.startTime)
        } else {
            dateField.date = utcDate(new Date())
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
    }

    DecoratedScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        clip: true

        // If the virtual keyboard comes up, make sure that the focused field
        // is visible. Scroll no further than necessary: the departure field
        // has focus when the dialog opens, and the date above it should stay
        // in view. The fields are children of the grid, so their y is a
        // position in the scrolled content. This handler runs before the
        // contentItem is resized, hence availableHeight rather than the
        // height of the contentItem.
        onHeightChanged: {
            const control = ApplicationWindow.activeFocusControl
            const flickable = contentItem as Flickable
            if (control === null || flickable === null || control.parent !== grid)
                return
            let y = flickable.contentY
            if (control.y + control.height > y + availableHeight)
                y = control.y + control.height - availableHeight
            if (control.y < y)
                y = control.y
            flickable.contentY = Math.max(0, Math.min(y, contentHeight - availableHeight))
        }

        GridLayout {
            id: grid

            width: flightEditorDialog.availableWidth
            columns: 2

            // The placeholder text of a Material text field moves onto the
            // upper border of the field once the field has focus or content,
            // and sticks out above the field by half its height. Leave enough
            // room that it stays clear of the field above.
            rowSpacing: 0.85 * flightEditorDialog.font.pixelSize

            Label {
                id: dateLabel
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Date (UTC)")
            }
            DateInput {
                id: dateField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                utc: true
                color: FlightLog.isValidDate(date) ? dateLabel.color : "red"
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Departure (ICAO)")
            }
            TextInputField {
                id: depField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize * 5
                placeholderText: qsTr("e.g. EDTF")
                inputMethodHints: Qt.ImhUppercaseOnly | Qt.ImhNoPredictiveText
                focus: true
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Arrival (ICAO)")
            }
            TextInputField {
                id: arrField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize * 5
                placeholderText: qsTr("e.g. EDFE")
                inputMethodHints: Qt.ImhUppercaseOnly | Qt.ImhNoPredictiveText
            }

            // Separator: an empty row, so that the groups are two row
            // spacings apart
            Item {
                Layout.columnSpan: 2
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Off-Block (UTC)")
            }
            TextInputField {
                id: offBlockField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("HH:mm (optional)")
                inputMethodHints: Qt.ImhTime
                color: flightEditorDialog.isValidOrEmptyTime(text) ? dateLabel.color : "red"
                onTextEdited: flightEditorDialog.formatTimeInput(offBlockField)
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Start (UTC)")
            }
            TextInputField {
                id: startField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: "HH:mm"
                inputMethodHints: Qt.ImhTime
                color: flightEditorDialog.isValidOrEmptyTime(text) ? dateLabel.color : "red"
                onTextEdited: flightEditorDialog.formatTimeInput(startField)
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Landing (UTC)")
            }
            TextInputField {
                id: landingField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: "HH:mm"
                inputMethodHints: Qt.ImhTime
                color: flightEditorDialog.isValidOrEmptyTime(text) ? dateLabel.color : "red"
                onTextEdited: flightEditorDialog.formatTimeInput(landingField)
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("On-Block (UTC)")
            }
            TextInputField {
                id: onBlockField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("HH:mm (optional)")
                inputMethodHints: Qt.ImhTime
                color: flightEditorDialog.isValidOrEmptyTime(text) ? dateLabel.color : "red"
                onTextEdited: flightEditorDialog.formatTimeInput(onBlockField)
            }

            // Separator: an empty row, so that the groups are two row
            // spacings apart
            Item {
                Layout.columnSpan: 2
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Pilot")
            }
            TextInputField {
                id: pilotField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("Pilot name (optional)")
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Aircraft")
            }
            TextInputField {
                id: callsignField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("Callsign")
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Comments")
            }
            TextInputField {
                id: commentsField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: qsTr("Optional comments")
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Landings")
            }
            TextInputField {
                id: landingsField
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: "1"
                inputMethodHints: Global.digitsInputHint
                validator: IntValidator { bottom: 1; top: 99 }
                color: text === "" || acceptableInput ? dateLabel.color : "red"
            }

            // Separator: an empty row, so that the groups are two row
            // spacings apart
            Item {
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
                    if (dist.isFinite() && dist.toNM() >= 0.1)
                        parts.push(qsTr("Distance: %1").arg(Navigator.aircraft.horizontalDistanceToString(dist)))
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
            enabled: flightEditorDialog.acceptableInput
        }
    }

}
