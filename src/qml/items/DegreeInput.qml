/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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


StackLayout {
    id: stackLayout

    property double value
    property int minValue
    property int maxValue

    readonly property bool acceptableInput: d_d.acceptableInput &&
                                            dm_d.acceptableInput &&
                                            dm_m.acceptableInput &&
                                            dms_d.acceptableInput &&
                                            dms_m.acceptableInput &&
                                            dms_s.acceptableInput &&
                                            !isNaN(value) &&
                                            (value >= minValue) &&
                                            (value <= maxValue)

    function setTexts() {
        if (isNaN(value)) {
            d_d.text = ""

            dm_d.text = ""
            dm_m.text = ""

            dms_d.text = ""
            dms_m.text = ""
            dms_s.text = ""
            return
        }

        var minutes = 60.0*(Math.abs(value) - Math.floor(Math.abs(value)))
        var seconds = 60.0*(minutes - Math.floor(minutes))

        // The degree field carries the sign of the whole value. Math.trunc()
        // of a value between -1 and 0 is -0, which renders as "0" and would
        // lose the sign, so the sign is written explicitly.
        var degreeText = (value < 0 ? "-" : "") + Math.trunc(Math.abs(value))

        d_d.text = value.toLocaleString(Qt.locale(), 'f', 5)
        d_d.cursorPosition = 0

        dm_d.text = degreeText
        dm_d.cursorPosition = 0
        dm_m.text = minutes.toLocaleString(Qt.locale(), 'f', 3)
        dm_m.cursorPosition = 0

        dms_d.text = degreeText
        dms_d.cursorPosition = 0

        dms_m.text = Math.floor(minutes)
        dms_m.cursorPosition = 0
        dms_s.text = seconds.toLocaleString(Qt.locale(), 'f', 1)
        dms_s.cursorPosition = 0
    }

    // Sign of a degree field. Read from the text, since the parsed number
    // cannot distinguish "-0" from "0".
    function isNegative(text) {
        var trimmed = text.trim()
        return trimmed.startsWith("-") || trimmed.startsWith(Qt.locale().negativeSign)
    }

    Component.onCompleted: setTexts()
    onCurrentIndexChanged: setTexts()
    onValueChanged: setTexts()


    RowLayout { // Degree
        id: d

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        function setValue() {
            if (!d_d.acceptableInput)
                return

            var dVal = Number.fromLocaleString(Qt.locale(), d_d.text)
            stackLayout.value = dVal
        }

        MyTextField {
            id: d_d

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: DoubleValidator {
                bottom: stackLayout.minValue
                top: stackLayout.maxValue
                notation: DoubleValidator.StandardNotation
            }
            color: (acceptableInput ? colorGlean.color : "red")

            onEditingFinished: {
                d.setValue()
            }
        }
        Label {
            id: colorGlean

            text: "°"
        }
    }

    RowLayout { // Degree and Minute
        id: dm
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        function setValue() {
            if (!dm_d.acceptableInput || !dm_m.acceptableInput)
                return

            var dVal = Math.abs(Number.fromLocaleString(Qt.locale(), dm_d.text))
            var mVal = Number.fromLocaleString(Qt.locale(), dm_m.text)
            var magnitude = dVal + mVal/60.0
            stackLayout.value = stackLayout.isNegative(dm_d.text) ? -magnitude : magnitude

        }

        MyTextField {
            id: dm_d

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            hasClearButton: false
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: IntValidator {
                bottom: stackLayout.minValue
                top: stackLayout.maxValue
            }
            color: (acceptableInput ? colorGlean.color : "red")

            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dm.setValue()
            }
        }
        Label { text: "°" }

        MyTextField {
            id: dm_m

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            hasClearButton: false
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: DoubleValidator {
                bottom: 0.0
                top: 59.9999999999999
                notation: DoubleValidator.StandardNotation
            }
            color: (acceptableInput ? colorGlean.color : "red")
            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dm.setValue()
            }
        }
        Label { text: "min" }

    }

    RowLayout { // Degree, Minutes and Seconds
        id: dms

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        function setValue() {
            if (!dms_d.acceptableInput || !dms_m.acceptableInput || !dms_s.acceptableInput)
                return

            var dVal = Math.abs(Number.fromLocaleString(Qt.locale(), dms_d.text))
            var mVal = Number.fromLocaleString(Qt.locale(), dms_m.text)
            var sVal = Number.fromLocaleString(Qt.locale(), dms_s.text)
            var magnitude = dVal + mVal/60.0 + sVal/3600.0
            stackLayout.value = stackLayout.isNegative(dms_d.text) ? -magnitude : magnitude

        }

        MyTextField {
            id: dms_d

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            hasClearButton: false
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: IntValidator {
                bottom: stackLayout.minValue
                top: stackLayout.maxValue
            }
            color: (acceptableInput ? colorGlean.color : "red")
            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dms.setValue()
            }
        }
        Label { text: "°" }

        MyTextField {
            id: dms_m

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            hasClearButton: false
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: IntValidator {
                bottom: 0
                top: 59
            }
            color: (acceptableInput ? colorGlean.color : "red")
            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dms.setValue()
            }
        }
        Label { text: "min" }

        MyTextField {
            id: dms_s

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            hasClearButton: false
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: DoubleValidator {
                bottom: 0.0
                top: 59.9999999999999
                notation: DoubleValidator.StandardNotation
            }
            color: (acceptableInput ? colorGlean.color : "red")
            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dms.setValue()
            }
        }
        Label { text: "sec" }
    }
}
