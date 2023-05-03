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
                                            (value >= minValue) &&
                                            (value <= maxValue)

    function setTexts() {
        var minutes = 60.0*(Math.abs(value) - Math.floor(Math.abs(value)))
        var seconds = 60.0*(minutes - Math.floor(minutes))

        d_d.text = value.toLocaleString(Qt.locale(), 'f', 7)

        dm_d.text = Math.trunc(value)
        dm_m.text = minutes.toLocaleString(Qt.locale(), 'f', 5)

        dms_d.text = Math.trunc(value)
        dms_m.text = Math.floor(minutes)
        dms_s.text = seconds.toLocaleString(Qt.locale(), 'f', 3)
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
            value = dVal
        }

        TextField {
            id: d_d

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: DoubleValidator {
                bottom: stackLayout.minValue
                top: stackLayout.maxValue
                notation: DoubleValidator.StandardNotation
            }
            color: (acceptableInput ? "" : "red")

            onEditingFinished: {
                d.setValue()
            }
        }
        Label { text: "°" }
    }

    RowLayout { // Degree and Minute
        id: dm
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        function setValue() {
            if (!dm_d.acceptableInput || !dm_m.acceptableInput)
                return

            var dVal = Number.fromLocaleString(Qt.locale(), dm_d.text)
            var mVal = Number.fromLocaleString(Qt.locale(), dm_m.text)
            if (dVal >= 0)
                value = dVal + mVal/60.0
            else
                value = dVal - mVal/60.0
        }

        TextField {
            id: dm_d

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: IntValidator {
                bottom: stackLayout.minValue
                top: stackLayout.maxValue
            }
            color: (acceptableInput ? "" : "red")

            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dm.setValue()
            }
        }
        Label { text: "°" }

        TextField {
            id: dm_m

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: DoubleValidator {
                bottom: 0.0
                top: 59.9999999999999
                notation: DoubleValidator.StandardNotation
            }
            color: (acceptableInput ? "" : "red")
            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dm.setValue()
            }
        }
        Label { text: "'" }

    }

    RowLayout { // Degree, Minutes and Seconds
        id: dms

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignBaseline

        function setValue() {
            if (!dms_d.acceptableInput || !dms_m.acceptableInput || !dms_s.acceptableInput)
                return

            var dVal = Number.fromLocaleString(Qt.locale(), dms_d.text)
            var mVal = Number.fromLocaleString(Qt.locale(), dms_m.text)
            var sVal = Number.fromLocaleString(Qt.locale(), dms_s.text)
            if (dVal >= 0)
                value = dVal + mVal/60.0 + sVal/3600.0
            else
                value = dVal - mVal/60.0 - sVal/3600.0
        }

        TextField {
            id: dms_d

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: IntValidator {
                bottom: stackLayout.minValue
                top: stackLayout.maxValue
            }
            color: (acceptableInput ? "" : "red")
            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dms.setValue()
            }
        }
        Label { text: "°" }

        TextField {
            id: dms_m

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: IntValidator {
                bottom: 0
                top: 59
            }
            color: (acceptableInput ? "" : "red")
            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dms.setValue()
            }
        }
        Label { text: "'" }

        TextField {
            id: dms_s

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            inputMethodHints: Qt.ImhFormattedNumbersOnly
            validator: DoubleValidator {
                bottom: 0.0
                top: 59.9999999999999
                notation: DoubleValidator.StandardNotation
            }
            color: (acceptableInput ? "" : "red")
            readonly property double numValue: Number.fromLocaleString(Qt.locale(), text)
            onEditingFinished: {
                dms.setValue()
            }
        }
        Label { text: "''" }
    }
}
