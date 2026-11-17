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

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute

// A field that shows a date and lets the user pick another one from a
// calendar.
//
// The property "date" holds the date as an ISO string "yyyy-MM-dd", or an
// empty string. It is a calendar day without time zone; the caller decides
// what it means (the flight log uses UTC days). The field shows the date in
// the format of the current locale. Tapping the field, or pressing Return or
// Space while it has focus, opens the calendar. The signal dateEdited() is
// emitted when the user picks a date, but not when "date" is set from outside.

TextField {
    id: dateInput

    property string date: ""

    // If true, "today" is the current day in UTC rather than in local time.
    // This affects the "Today" button and the marker in the calendar.
    property bool utc: false

    // Today as ISO date string. Updated whenever the calendar opens.
    property string todayDate: ""

    signal dateEdited()

    // Split an ISO date string into year, month (0-based, as in JavaScript and
    // MonthGrid) and day. Returns null if the string is not a valid date.
    function dateParts(iso: string): var {
        const match = /^(\d{4})-(\d{2})-(\d{2})$/.exec(iso)
        if (match === null)
            return null
        const year = Number(match[1])
        const month = Number(match[2]) - 1
        const day = Number(match[3])
        const check = new Date(year, month, day)
        if (check.getFullYear() !== year || check.getMonth() !== month || check.getDate() !== day)
            return null
        return { year: year, month: month, day: day }
    }

    // Inverse of dateParts()
    function isoDate(year: int, month: int, day: int): string {
        return year.toString().padStart(4, '0')
                + "-" + String(month + 1).padStart(2, '0')
                + "-" + day.toString().padStart(2, '0')
    }

    // Today as ISO date string, see property utc
    function todayIsoDate(): string {
        const now = new Date()
        if (utc)
            return isoDate(now.getUTCFullYear(), now.getUTCMonth(), now.getUTCDate())
        return isoDate(now.getFullYear(), now.getMonth(), now.getDate())
    }

    function openCalendar() {
        todayDate = todayIsoDate()
        let parts = dateParts(date)
        if (parts === null)
            parts = dateParts(todayDate)
        monthGrid.year = parts.year
        monthGrid.month = parts.month
        calendar.open()
    }

    function pick(iso: string) {
        PlatformAdaptor.vibrateBrief()
        date = iso
        dateEdited()
        calendar.close()
    }

    // Move the calendar by a number of months
    function shiftMonth(delta: int) {
        const months = monthGrid.year * 12 + monthGrid.month + delta
        monthGrid.year = Math.floor(months / 12)
        monthGrid.month = months - 12 * monthGrid.year
    }

    readOnly: true
    selectByMouse: false
    rightPadding: calendarButton.width + calendarButton.anchors.rightMargin
    implicitWidth: 100 // set arbitrary value to avoid binding loop, as in TextInputField

    text: {
        const parts = dateParts(date)
        if (parts === null)
            return date
        return new Date(parts.year, parts.month, parts.day).toLocaleDateString(Qt.locale(), Locale.LongFormat)
    }

    Keys.onReturnPressed: openCalendar()
    Keys.onEnterPressed: openCalendar()
    Keys.onSpacePressed: openCalendar()

    // Decoration only, taps are handled by the MouseArea below. This is the
    // same construction as the clear button of TextInputField, so that the
    // two line up when the fields are stacked in a form. The icon is larger
    // than the clear icon; the margin keeps its center in the same place.
    RoundButton {
        id: calendarButton

        anchors.right: parent.right
        anchors.rightMargin: (dateInput.font.pixelSize - icon.width) / 2
        anchors.verticalCenter: parent.verticalCenter

        focusPolicy: Qt.NoFocus
        background: Item {}

        icon.source: "/icons/material/ic_event.svg"
        icon.width: dateInput.font.pixelSize * 1.3
        icon.height: dateInput.font.pixelSize * 1.3
    }

    // Catch taps before the TextField sees them: the field never gets a
    // cursor, and on mobile no keyboard comes up.
    MouseArea {
        anchors.fill: parent
        onClicked: {
            dateInput.forceActiveFocus()
            dateInput.openCalendar()
        }
    }

    Dialog {
        id: calendar

        parent: Overlay.overlay
        anchors.centerIn: parent

        modal: true
        focus: true
        // Same policy as CenteringDialog, see there
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnReleaseOutside

        width: Math.min(parent.width - 2 * font.pixelSize, 22 * font.pixelSize)

        ColumnLayout {
            anchors.fill: parent

            RowLayout {
                Layout.fillWidth: true

                ToolButton {
                    icon.source: "/icons/material/ic_chevron_left.svg"
                    onClicked: dateInput.shiftMonth(-1)
                }
                Label {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    font.bold: true
                    text: monthGrid.title
                }
                ToolButton {
                    icon.source: "/icons/material/ic_chevron_right.svg"
                    onClicked: dateInput.shiftMonth(1)
                }
            }

            DayOfWeekRow {
                Layout.fillWidth: true
                locale: monthGrid.locale
                opacity: 0.7
            }

            MonthGrid {
                id: monthGrid

                Layout.fillWidth: true
                locale: Qt.locale()

                // Swipe horizontally to change the month
                DragHandler {
                    target: null
                    xAxis.enabled: true
                    yAxis.enabled: false
                    onActiveChanged: {
                        if (active)
                            return
                        const dx = centroid.position.x - centroid.pressPosition.x
                        if (Math.abs(dx) > 3 * monthGrid.font.pixelSize)
                            dateInput.shiftMonth(dx < 0 ? 1 : -1)
                    }
                }

                delegate: AbstractButton {
                    id: dayButton

                    required property int day
                    required property int month
                    required property int year

                    readonly property string iso: dateInput.isoDate(year, month, day)
                    readonly property bool selected: dateInput.date === iso
                    readonly property bool today: dateInput.todayDate === iso
                    readonly property bool inMonth: month === monthGrid.month

                    implicitWidth: 2.5 * monthGrid.font.pixelSize
                    implicitHeight: 2.5 * monthGrid.font.pixelSize

                    text: monthGrid.locale.toString(new Date(year, month, day), "d")

                    contentItem: Label {
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        text: dayButton.text
                        font.bold: dayButton.today || dayButton.selected
                        color: dayButton.selected ? "white" : dayButton.Material.foreground
                        opacity: dayButton.inMonth || dayButton.selected ? 1 : 0.4
                    }

                    background: Rectangle {
                        anchors.centerIn: parent
                        width: Math.min(dayButton.width, dayButton.height)
                        height: width
                        radius: width / 2

                        color: dayButton.selected ? dayButton.Material.accentColor
                                                  : dayButton.down ? dayButton.Material.rippleColor : "transparent"
                        border.width: dayButton.today && !dayButton.selected ? 1 : 0
                        border.color: dayButton.Material.accentColor
                    }

                    onClicked: dateInput.pick(iso)
                }
            }
        }

        footer: DialogButtonBox {
            Button {
                text: qsTr("Today")
                flat: true
                onClicked: dateInput.pick(dateInput.todayIsoDate())
            }
            Button {
                text: qsTr("Cancel")
                flat: true
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
        }
    }
}
