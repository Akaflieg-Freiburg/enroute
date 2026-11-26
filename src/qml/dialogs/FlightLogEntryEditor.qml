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
 * Usage: set property editFlight, then open(). On accepted, call
 * resultFlight() to get the edited flight: editFlight with the user's
 * changes applied, see FlightLog.applyEdits(). It has the UUID of
 * editFlight.
 *
 * All dates and times are UTC. The date is the date of the takeoff. The other
 * times are placed around the takeoff by FlightLog.createFlight(), and a
 * marker such as "+1 Sat 22 Nov" next to a time shows when it falls on
 * another day.
 */

CenteringDialog {
    id: flightEditorDialog

    property flight editFlight

    // The flight as it will be saved. The day markers and the lines at the
    // bottom of the dialog show this flight.
    readonly property flight preview: resultFlight()

    // Problems that keep the flight from being saved, in the order of the
    // fields. Date and takeoff time are mandatory. The other times may be
    // left empty but must be valid otherwise: a time that FlightLog cannot
    // interpret would be dropped from the saved flight.
    readonly property list<string> errors: {
        const result = []
        if (!FlightLog.isValidDate(dateField.date))
            result.push(qsTr("Invalid takeoff date"))
        if (!offBlockField.acceptableTime)
            result.push(qsTr("Invalid off-block time"))
        if (startField.missing)
            result.push(qsTr("No takeoff time"))
        else if (!startField.acceptableTime)
            result.push(qsTr("Invalid takeoff time"))
        if (!landingField.acceptableTime)
            result.push(qsTr("Invalid landing time"))
        if (!onBlockField.acceptableTime)
            result.push(qsTr("Invalid on-block time"))
        // FlightLog.createFlight() orders the times: if one is in the future,
        // all later ones are as well. Name the first one only; the fields
        // show all of them in red.
        if (offBlockField.inFuture)
            result.push(qsTr("Off-block time in the future"))
        else if (startField.inFuture)
            result.push(qsTr("Takeoff time in the future"))
        else if (landingField.inFuture)
            result.push(qsTr("Landing time in the future"))
        else if (onBlockField.inFuture)
            result.push(qsTr("On-block time in the future"))
        if (landingsField.text !== "" && !landingsField.acceptableInput)
            result.push(qsTr("Invalid number of landings"))
        return result
    }

    // Entries that can be saved, but are probably incomplete or mistyped.
    // A landing time that is mistakenly earlier than the takeoff time is
    // placed on the next day by FlightLog.createFlight(), and caught by the
    // flight time.
    readonly property list<string> warnings: {
        const result = []
        const hour = 60 * 60 * 1000
        const offBlock = preview.offBlockTime.getTime()
        const takeoff = preview.startTime.getTime()
        const landing = preview.landingTime.getTime()
        const onBlock = preview.onBlockTime.getTime()
        if (depField.displayText.trim() === "")
            result.push(qsTr("No departure location"))
        // A flight without landing time may still be in progress
        if (arrField.displayText.trim() === "" && !isNaN(landing))
            result.push(qsTr("No arrival location"))
        if (takeoff - offBlock > hour)
            result.push(qsTr("More than 1 h between off-block and takeoff"))
        if (landing - takeoff > 12 * hour)
            result.push(qsTr("Flight time more than 12 h"))
        if (onBlock - landing > hour)
            result.push(qsTr("More than 1 h between landing and on-block"))
        return result
    }

    // True if the input can be saved. The OK button is disabled exactly when
    // the dialog shows an error.
    readonly property bool acceptableInput: errors.length === 0

    // If true, the date follows the takeoff time, see followTakeoff(). This
    // is the case when the dialog opens for a flight without a takeoff time
    // (a new flight), until the user picks a date.
    property bool autoDate: false

    modal: true

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
            codeOrName(depField.text),
            codeOrName(arrField.text),
            dateField.date,
            offBlockField.text,
            startField.text,
            landingField.text,
            onBlockField.text,
            pilotField.text,
            callsignField.text,
            commentsField.text
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
    // complete time, or pressing Return, moves the focus to nextField. The
    // text is red if it is not a valid time, or if the time lies in the
    // future. If the time is not on the day of the takeoff, a marker such as
    // "+1 Sat 22 Nov" appears at the right end of the field, left of the
    // clear button.
    component TimeField: TextInputField {
        id: timeField

        // The time as it will be saved, with its date
        required property date time

        // The takeoff as it will be saved, for the day marker
        required property date takeoff

        // Field that gets the focus next, or null
        property Item nextField: null

        // If true, the time must be given
        property bool mandatory: false

        // True if a mandatory time is not given. The placeholder is then
        // red, like the text of a time that is not valid.
        readonly property bool missing: mandatory && text.trim() === ""

        // True if the text is empty or a valid time
        readonly property bool acceptableTime: text.trim() === "" || FlightLog.isValidTime(text)

        // True if the time lies in the future
        readonly property bool inFuture: time.getTime() > Clock.time.getTime()

        // Difference in UTC calendar days from takeoff to time. Zero if one
        // of the two is invalid.
        readonly property int dayOffset: {
            if (isNaN(time.getTime()) || isNaN(takeoff.getTime()))
                return 0
            const day = Date.UTC(time.getUTCFullYear(), time.getUTCMonth(), time.getUTCDate())
            const takeoffDay = Date.UTC(takeoff.getUTCFullYear(), takeoff.getUTCMonth(), takeoff.getUTCDate())
            return Math.round((day - takeoffDay) / (24 * 60 * 60 * 1000))
        }

        function advance(): bool {
            if (nextField === null)
                return false
            nextField.forceActiveFocus()
            return true
        }

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        // Leave room for the day marker
        rightPadding: clearButtonSpace + (dayMarker.text === "" ? 0 : dayMarker.implicitWidth + 0.5 * font.pixelSize)

        placeholderText: qsTr("hh:mm")
        inputMethodHints: Qt.ImhTime

        // Digits and colon only. This filters the input; whether the text is
        // a time is decided by FlightLog.isValidTime(). Do not put the format
        // of a time here: half-typed input would then not be acceptable, and
        // editingFinished() would not be emitted for it.
        validator: RegularExpressionValidator { regularExpression: /[0-9:]{0,5}/ }

        EnterKey.type: nextField === null ? Qt.EnterKeyDefault : Qt.EnterKeyNext
        color: acceptableTime && !inFuture ? Material.foreground : "red"

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

        // Otherwise the colour of the style applies
        Binding on placeholderTextColor {
            when: timeField.missing
            value: "red"
        }

        // Width of a time, to decide whether the long day marker fits
        TextMetrics {
            id: timeMetrics
            font: timeField.font
            text: "00:00"
        }

        // The day marker: "+1 Sat 22 Nov" if that leaves room for the time,
        // otherwise only "+1". Empty if the time is on the day of the
        // takeoff.
        Label {
            id: dayMarker

            // U+2212 is the minus sign
            readonly property string days: timeField.dayOffset > 0 ? "+" + timeField.dayOffset
                                                                   : "\u2212" + (-timeField.dayOffset)
            readonly property string longText: {
                // The UTC day, as a local date object for formatting
                const time = timeField.time
                const day = new Date(time.getUTCFullYear(), time.getUTCMonth(), time.getUTCDate())
                //: Format of the date in the marker next to a time that falls
                //: on another day than the takeoff, as in "+1 Sat 22 Nov".
                //: See QDate::toString() for the format characters.
                const dayString = day.toLocaleDateString(Qt.locale(), qsTr("ddd d MMM"))
                //: Marker next to a time that falls on another day than the
                //: takeoff. %1 is the difference in days ("+1"), %2 the date.
                return qsTr("%1 %2").arg(days).arg(dayString)
            }

            anchors.right: parent.right
            anchors.rightMargin: timeField.clearButtonSpace
            anchors.baseline: parent.baseline

            color: Material.accentColor
            font.pixelSize: 0.9 * timeField.font.pixelSize
            text: {
                if (timeField.dayOffset === 0)
                    return ""
                const room = timeField.width - timeField.leftPadding - timeField.clearButtonSpace
                           - 0.5 * timeField.font.pixelSize - timeMetrics.advanceWidth
                return longMetrics.advanceWidth <= room ? longText : days
            }

            TextMetrics {
                id: longMetrics
                font: dayMarker.font
                text: dayMarker.longText
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0.5 * flightEditorDialog.font.pixelSize

        DecoratedScrollView {
            id: scrollView

            // Scroll no further than necessary to show the focused field, with
            // a margin, so that the field stays clear of the "more"
            // indicators. The fields are children of the grid, so their y is a
            // position in the scrolled content. The height handler runs before
            // the contentItem is resized, hence availableHeight rather than
            // the height of the contentItem.
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

            Layout.fillWidth: true
            Layout.fillHeight: true
            contentWidth: availableWidth

            clip: true

            // The virtual keyboard came up, the status lines below changed
            // size, or the focus moved to another field (by auto-advance or
            // the Next key)
            onHeightChanged: ensureFocusVisible()
            ApplicationWindow.onActiveFocusControlChanged: ensureFocusVisible()

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
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    text: qsTr("Location")
                    font.pixelSize: 1.2 * flightEditorDialog.font.pixelSize
                    font.bold: true
                }

                Label {
                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("Departure")
                }
                TextInputField {
                    id: depField
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

                Label {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    Layout.topMargin: font.pixelSize / 2
                    text: qsTr("Date and Time (UTC)")
                    font.pixelSize: 1.2 * flightEditorDialog.font.pixelSize
                    font.bold: true
                }

                Label {
                    id: dateLabel
                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("Takeoff date")
                }
                DateInput {
                    id: dateField
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
                    time: flightEditorDialog.preview.offBlockTime
                    takeoff: flightEditorDialog.preview.startTime
                    nextField: startField
                }

                Label {
                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("Takeoff")
                }
                TimeField {
                    id: startField
                    time: flightEditorDialog.preview.startTime
                    takeoff: flightEditorDialog.preview.startTime
                    nextField: landingField
                    mandatory: true
                    onTextChanged: flightEditorDialog.followTakeoff()
                }

                Label {
                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("Landing")
                }
                TimeField {
                    id: landingField
                    time: flightEditorDialog.preview.landingTime
                    takeoff: flightEditorDialog.preview.startTime
                    nextField: onBlockField
                }

                Label {
                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("On-Block")
                }
                TimeField {
                    id: onBlockField
                    time: flightEditorDialog.preview.onBlockTime
                    takeoff: flightEditorDialog.preview.startTime
                }

                Label {
                    Layout.columnSpan: 2
                    Layout.fillWidth: true
                    Layout.topMargin: font.pixelSize / 2
                    text: qsTr("Details")
                    font.pixelSize: 1.2 * flightEditorDialog.font.pixelSize
                    font.bold: true
                }

                Label {
                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("Pilot")
                }
                TextInputField {
                    id: pilotField
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    placeholderText: qsTr("Name")
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
                }

                Label {
                    Layout.alignment: Qt.AlignBaseline
                    text: qsTr("Landings")
                }
                TextInputField {
                    id: landingsField
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignBaseline
                    placeholderText: qsTr("Number of landings")
                    inputMethodHints: Global.digitsInputHint
                    validator: IntValidator { bottom: 1; top: 99 }
                    color: text === "" || landingsField.acceptableInput ? dateLabel.color : "red"
                }
            }
        }

        // The two status lines are not part of the scrolled content: they
        // stay in view next to the OK button, and the "more" indicator of the
        // scroll view never covers them. If the dialog is too short, only the
        // scroll view shrinks.

        // Errors, or else warnings
        Label {
            Layout.fillWidth: true
            Layout.minimumHeight: implicitHeight
            visible: text !== ""
            color: "red"
            wrapMode: Text.Wrap
            text: {
                const errors = flightEditorDialog.errors
                const warnings = flightEditorDialog.warnings
                //: Prefix of the list of problems in the flight editor. The
                //: list is a heading-like enumeration, so only singular and
                //: plural are distinguished.
                if (errors.length === 1)
                    return qsTr("Error: %1").arg(errors[0])
                if (errors.length > 1)
                    return qsTr("Errors: %1").arg(errors.join(" • "))
                if (warnings.length === 1)
                    return qsTr("Warning: %1").arg(warnings[0])
                if (warnings.length > 1)
                    return qsTr("Warnings: %1").arg(warnings.join(" • "))
                return ""
            }
        }

        // Calculated values (read-only), once the flight can be saved
        Label {
            Layout.fillWidth: true
            Layout.minimumHeight: implicitHeight
            visible: text !== ""
            wrapMode: Text.Wrap
            text: {
                if (flightEditorDialog.errors.length > 0)
                    return ""
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
                // Separated by " • " as elsewhere in the app. As in the list
                // of flights, the space after the "•" is the only place for a
                // line break.
                return parts.map(part => part.replace(/ /g, " ")).join(" • ")
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
