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
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../items"

/* Flight log entry editor dialog
 *
 * Usage: set property editFlight and flightUuid, then open().
 * On accepted, call resultFlight() to get the edited Flight value. This is
 * editFlight with the user's changes applied, see FlightLog.applyEdits().
 *
 * All dates and times are UTC. The date is the date of the takeoff. The other
 * times are placed around the takeoff by FlightLog.createFlight(), and a
 * marker such as "+1 Sat 22 Nov" next to a time shows when it falls on
 * another day.
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

    // The flight as it will be saved. The day markers and the lines at the
    // bottom of the dialog show this flight.
    readonly property flight preview: resultFlight()

    // True if a field holds text that cannot be saved. Mandatory fields that
    // are empty do not count here.
    readonly property bool hasInvalidEntries: !offBlockField.acceptableTime
                                              || !startField.acceptableTime
                                              || !landingField.acceptableTime
                                              || !onBlockField.acceptableTime
                                              || (landingsField.text !== "" && !landingsField.acceptableInput)

    // True if the input can be saved. Departure, date and takeoff time are
    // mandatory. The other times may be left empty but must be valid
    // otherwise: a time that FlightLog cannot interpret would be dropped from
    // the saved flight.
    readonly property bool acceptableInput: depField.displayText.trim() !== ""
                                            && FlightLog.isValidDate(dateField.date)
                                            && startField.text.trim() !== ""
                                            && !hasInvalidEntries

    // If true, the date follows the takeoff time, see followTakeoff(). This
    // is the case when the dialog opens for a flight without a takeoff time
    // (a new flight), until the user picks a date.
    property bool autoDate: false

    modal: true
    title: qsTr("Edit Flight")

    // The airfield fields hold an ICAO code or, for an airfield without one,
    // its name: flight detection and the "Add Flight" button fill in
    // Waypoint.shortName. Codes are stored in upper case, so that they are
    // found in the maps. Anything longer than a code is a name and is kept as
    // entered.
    function codeOrName(text: string): string {
        const trimmed = text.trim()
        return trimmed.length <= 4 ? trimmed.toUpperCase() : trimmed
    }

    // Format a QDateTime as UTC time string "HH:mm", or "" if it is invalid
    function utcTime(dt: date): string {
        if (isNaN(dt.getTime()))
            return ""
        return dt.getUTCHours().toString().padStart(2, '0')
             + ":" + dt.getUTCMinutes().toString().padStart(2, '0')
    }

    // While the date is automatic, set it so that the takeoff is the most
    // recent time that matches the takeoff field: today (UTC), or yesterday
    // if today's takeoff would still be in the future. A flight that is
    // logged within a day after takeoff thus needs no date input, also when
    // midnight UTC lies in between.
    function followTakeoff() {
        if (!autoDate || !FlightLog.isValidTime(startField.text))
            return
        const hoursMinutes = FlightLog.normalizedTime(startField.text).split(":")
        const now = new Date()
        let takeoff = Date.UTC(now.getUTCFullYear(), now.getUTCMonth(), now.getUTCDate(),
                               Number(hoursMinutes[0]), Number(hoursMinutes[1]))
        if (takeoff > now.getTime())
            takeoff -= 24 * 60 * 60 * 1000
        const day = new Date(takeoff)
        dateField.date = dateField.isoDate(day.getUTCFullYear(), day.getUTCMonth(), day.getUTCDate())
    }

    // The flight as it will be saved: editFlight with the content of the
    // fields applied
    function resultFlight(): flight {
        const edited = FlightLog.createFlight(
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
        // Set autoDate first: setting the takeoff field calls followTakeoff()
        const start = editFlight.startTime
        autoDate = isNaN(start.getTime())
        if (autoDate)
            dateField.date = dateField.todayIsoDate()
        else
            dateField.date = dateField.isoDate(start.getUTCFullYear(), start.getUTCMonth(), start.getUTCDate())

        offBlockField.text = utcTime(editFlight.offBlockTime)
        startField.text = utcTime(start)
        landingField.text = utcTime(editFlight.landingTime)
        onBlockField.text = utcTime(editFlight.onBlockTime)

        depField.text = editFlight.departureICAO
        arrField.text = editFlight.arrivalICAO
        pilotField.text = editFlight.pilotName
        callsignField.text = editFlight.aircraftCallsign
        commentsField.text = editFlight.comments
        landingsField.text = editFlight.landingCount > 0 ? editFlight.landingCount.toString() : "1"

        // A new flight usually has its departure filled in already. The
        // takeoff time is then the first mandatory field that is empty.
        defaultFocusItem = autoDate && depField.text.trim() !== "" ? startField : depField

        // The dialog is reused. Start at the top, not where the last flight
        // was left.
        const flickable = scrollView.contentItem as Flickable
        if (flickable !== null)
            flickable.contentY = 0
    }

    // Field for a time of day. Accepts the formats of FlightLog.isValidTime()
    // and shows the time as "HH:mm" once the input is finished. Typing a
    // complete time, or pressing Return, moves the focus to nextField.
    component TimeField: TextInputField {
        // Field that gets the focus next, or null
        property Item nextField: null

        // True if the text is empty or a valid time
        readonly property bool acceptableTime: text.trim() === "" || FlightLog.isValidTime(text)

        function advance(): bool {
            if (nextField === null)
                return false
            nextField.forceActiveFocus()
            return true
        }

        Layout.preferredWidth: 5 * font.pixelSize
        Layout.alignment: Qt.AlignBaseline

        hasClearButton: false
        placeholderText: qsTr("hh:mm")
        inputMethodHints: Qt.ImhTime
        EnterKey.type: nextField === null ? Qt.EnterKeyDefault : Qt.EnterKeyNext
        color: acceptableTime ? Material.foreground : "red"

        // Advance only if the user typed at the end of the text, so that
        // editing in the middle of a time never moves the focus away
        onTextEdited: {
            if (cursorPosition === text.length
                    && /^(\d{4}|\d{1,2}:\d{2})$/.test(text.trim())
                    && FlightLog.isValidTime(text))
                advance()
        }
        onEditingFinished: {
            const normalized = FlightLog.normalizedTime(text)
            if (normalized !== text)
                text = normalized
        }
        Keys.onReturnPressed: event => event.accepted = advance()
        Keys.onEnterPressed: event => event.accepted = advance()
    }

    // Shows the UTC day of a time, for example "+1 Sat 22 Nov", if it is not
    // the day of the takeoff. Empty otherwise.
    component DayMarker: Label {
        required property date time
        required property date takeoff

        // Difference in UTC calendar days from takeoff to time. Zero if one
        // of the two is invalid.
        readonly property int offset: {
            if (isNaN(time.getTime()) || isNaN(takeoff.getTime()))
                return 0
            const day = Date.UTC(time.getUTCFullYear(), time.getUTCMonth(), time.getUTCDate())
            const takeoffDay = Date.UTC(takeoff.getUTCFullYear(), takeoff.getUTCMonth(), takeoff.getUTCDate())
            return Math.round((day - takeoffDay) / (24 * 60 * 60 * 1000))
        }

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        color: Material.accentColor
        elide: Text.ElideRight
        text: {
            if (offset === 0)
                return ""
            // U+2212 is the minus sign
            const days = offset > 0 ? "+" + offset : "−" + (-offset)
            // The UTC day, as a local date object for formatting
            const day = new Date(time.getUTCFullYear(), time.getUTCMonth(), time.getUTCDate())
            //: Format of the date in the marker next to a time that falls on
            //: another day than the takeoff, as in "+1 Sat 22 Nov". See
            //: QDate::toString() for the format characters.
            const dayString = day.toLocaleDateString(Qt.locale(), qsTr("ddd d MMM"))
            //: Marker next to a time that falls on another day than the
            //: takeoff. %1 is the difference in days ("+1"), %2 the date.
            return qsTr("%1 %2").arg(days).arg(dayString)
        }
    }

    DecoratedScrollView {
        id: scrollView

        // Scroll no further than necessary to show the focused field, with a
        // margin, so that the field stays clear of the "more" indicators. The
        // fields are children of the grid, so their y is a position in the
        // scrolled content. The height handler runs before the contentItem is
        // resized, hence availableHeight rather than the height of the
        // contentItem.
        function ensureFocusVisible() {
            const control = ApplicationWindow.activeFocusControl
            const flickable = contentItem as Flickable
            if (control === null || flickable === null || control.parent !== grid)
                return
            const margin = font.pixelSize
            let y = flickable.contentY
            if (control.y + control.height + margin > y + availableHeight)
                y = control.y + control.height + margin - availableHeight
            if (control.y - margin < y)
                y = control.y - margin
            flickable.contentY = Math.max(0, Math.min(y, contentHeight - availableHeight))
        }

        anchors.fill: parent
        contentWidth: availableWidth

        clip: true

        // The virtual keyboard came up, or the focus moved to another field
        // (by auto-advance or the Next key)
        onHeightChanged: ensureFocusVisible()
        ApplicationWindow.onActiveFocusControlChanged: ensureFocusVisible()

        GridLayout {
            id: grid

            width: flightEditorDialog.availableWidth
            columns: 3

            // The placeholder text of a Material text field moves onto the
            // upper border of the field once the field has focus or content,
            // and sticks out above the field by half its height. Leave enough
            // room that it stays clear of the field above.
            rowSpacing: 0.85 * flightEditorDialog.font.pixelSize

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Departure")
            }
            TextInputField {
                id: depField
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize * 5
                placeholderText: qsTr("e.g. EDTF")
                inputMethodHints: Qt.ImhUppercaseOnly | Qt.ImhNoPredictiveText
                EnterKey.type: Qt.EnterKeyNext
                Keys.onReturnPressed: arrField.forceActiveFocus()
                Keys.onEnterPressed: arrField.forceActiveFocus()
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Arrival")
            }
            TextInputField {
                id: arrField
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize * 5
                placeholderText: qsTr("e.g. EDFE")
                inputMethodHints: Qt.ImhUppercaseOnly | Qt.ImhNoPredictiveText
                // Skip the date: it is read-only, and the keyboard would go
                // away
                EnterKey.type: Qt.EnterKeyNext
                Keys.onReturnPressed: offBlockField.forceActiveFocus()
                Keys.onEnterPressed: offBlockField.forceActiveFocus()
            }

            // Section caption. It also keeps the groups two row spacings
            // apart.
            Label {
                Layout.columnSpan: 3
                Layout.fillWidth: true
                font.pixelSize: 0.9 * flightEditorDialog.font.pixelSize
                opacity: 0.7
                wrapMode: Text.Wrap
                text: qsTr("Date and times in UTC")
            }

            Label {
                id: dateLabel
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Takeoff date")
            }
            DateInput {
                id: dateField
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                utc: true
                //: Format of the takeoff date in the flight editor. See
                //: QDate::toString() for the format characters.
                displayFormat: qsTr("ddd, d MMM yyyy")
                color: FlightLog.isValidDate(date) ? dateLabel.color : "red"
                onDateEdited: flightEditorDialog.autoDate = false
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Off-Block")
            }
            TimeField {
                id: offBlockField
                nextField: startField
            }
            DayMarker {
                font.pixelSize: 0.9 * flightEditorDialog.font.pixelSize
                time: flightEditorDialog.preview.offBlockTime
                takeoff: flightEditorDialog.preview.startTime
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Takeoff")
            }
            TimeField {
                id: startField
                nextField: landingField
                onTextChanged: flightEditorDialog.followTakeoff()
            }
            // The takeoff is on the takeoff date, so there is no marker
            Item {
                Layout.fillWidth: true
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Landing")
            }
            TimeField {
                id: landingField
                nextField: onBlockField
            }
            DayMarker {
                font.pixelSize: 0.9 * flightEditorDialog.font.pixelSize
                time: flightEditorDialog.preview.landingTime
                takeoff: flightEditorDialog.preview.startTime
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("On-Block")
            }
            TimeField {
                id: onBlockField
            }
            DayMarker {
                font.pixelSize: 0.9 * flightEditorDialog.font.pixelSize
                time: flightEditorDialog.preview.onBlockTime
                takeoff: flightEditorDialog.preview.startTime
            }

            // Separator: an empty row, so that the groups are two row
            // spacings apart
            Item {
                Layout.columnSpan: 3
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Pilot")
            }
            TextInputField {
                id: pilotField
                Layout.columnSpan: 2
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
                Layout.columnSpan: 2
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
                Layout.columnSpan: 2
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
                Layout.columnSpan: 2
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                placeholderText: "1"
                inputMethodHints: Global.digitsInputHint
                validator: IntValidator { bottom: 1; top: 99 }
                color: text === "" || landingsField.acceptableInput ? dateLabel.color : "red"
            }

            // Separator: an empty row, so that the groups are two row
            // spacings apart
            Item {
                Layout.columnSpan: 3
            }

            // Entries that cannot be saved, or a takeoff that has not
            // happened yet. The takeoff is only a warning: the flight can
            // still be saved.
            Label {
                Layout.columnSpan: 3
                Layout.fillWidth: true
                visible: text !== ""
                color: "red"
                wrapMode: Text.Wrap
                text: {
                    if (flightEditorDialog.hasInvalidEntries)
                        return qsTr("Correct the entries marked in red.")
                    if (flightEditorDialog.preview.startTime.getTime() > Clock.time.getTime())
                        return qsTr("The takeoff time is in the future.")
                    return ""
                }
            }

            // What is missing before the flight can be saved, or the
            // calculated values (read-only)
            Label {
                Layout.columnSpan: 3
                Layout.fillWidth: true
                font.italic: true
                opacity: 0.7
                wrapMode: Text.Wrap
                text: {
                    if (depField.displayText.trim() === "")
                        return qsTr("Enter the departure airfield.")
                    if (startField.text.trim() === "")
                        return qsTr("Enter the takeoff time.")
                    const f = flightEditorDialog.preview
                    let parts = []
                    const ft = f.flightTime()
                    if (ft !== "")
                        parts.push(qsTr("Duration: %1").arg(ft))
                    const bt = f.blockTime()
                    if (bt !== "")
                        parts.push(qsTr("Block: %1").arg(bt))
                    const dist = f.distance()
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
