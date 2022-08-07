/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15

import enroute 1.0

import "../items"

/* Waypoint dialog
 *
 * Short description: set property waypoint, then open. Re-implement onAccepted and read the new values from newName, newLatitude
 * and newLongitude.
 */

Dialog {
    id: waypointEditorDialog

    // Property waypoint, and code to handle waypoint changes
    property var waypoint: global.geoMapProvider().createWaypoint()

    readonly property string newName: wpNameField.text
    readonly property double newLatitude: latInput.value
    readonly property double newLongitude: longInput.value
    readonly property double newAltitudeMeter: eleField.valueMeter

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(view.width-view.font.pixelSize, 40*view.font.pixelSize)
    height: Math.min(view.height-view.font.pixelSize, implicitHeight)

    // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
    // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
    parent: Overlay.overlay
    x: parent == null ? 0 : (parent.width-width)/2.0
    y: parent == null ? 0 : (parent.height-height)/2.0

    modal: true
    title: qsTr("Edit Waypoint")

    standardButtons: Dialog.Cancel|Dialog.Ok
    focus: true

    GridLayout {
        width: waypointEditorDialog.availableWidth
        columns: 2

        Label {
            Layout.alignment: Qt.AlignBaseline
            text: qsTr("Name")
        }

        TextField {
            id: wpNameField

            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline
            Layout.minimumWidth: view.font.pixelSize*5

            text: waypoint.extendedName
            focus: true

            placeholderText: qsTr("undefined")
        }

        Label {
            Layout.alignment: Qt.AlignBaseline
            Layout.columnSpan: 2
        }

        Label {
            Layout.alignment: Qt.AlignBaseline
            text: qsTr("Latitude")
        }

        DegreeInput {
            id: latInput

            Layout.fillWidth: true
            currentIndex: formatChoice.currentIndex
            value: waypoint.coordinate.latitude
            minValue: -90.0
            maxValue: 90.0

            onAcceptableInputChanged: enableOk()
        }

        Label {
            Layout.alignment: Qt.AlignBaseline
            text: qsTr("Longitude")
        }

        DegreeInput {
            id: longInput

            Layout.fillWidth: true
            currentIndex: formatChoice.currentIndex
            value: waypoint.coordinate.longitude
            minValue: -180.0
            maxValue: 180.0

            onAcceptableInputChanged: enableOk()
        }

        Label {
            Layout.alignment: Qt.AlignBaseline
            text: qsTr("Format")
        }

        ComboBox {
            id: formatChoice
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline

            model: [ qsTr("Degrees"), qsTr("Degrees and Minutes"), qsTr("Degrees, Minutes and Seconds") ]
        }

        Label {
            Layout.alignment: Qt.AlignBaseline
            Layout.columnSpan: 2
        }

        Label {
            Layout.alignment: Qt.AlignBaseline
            text: qsTr("Elevation")
        }

        ElevationInput {
            id: eleField

            Layout.fillWidth: true
            currentIndex: eleFormatChoice.currentIndex
            valueMeter: waypoint.coordinate.altitude
        }

        Label {
            Layout.alignment: Qt.AlignBaseline
            text: qsTr("Unit")
        }

        ComboBox {
            id: eleFormatChoice
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignBaseline

            model: [ qsTr("Feet"), qsTr("Meter") ]
        }


    }

    function enableOk() {
        waypointEditorDialog.standardButton(DialogButtonBox.Ok).enabled = latInput.acceptableInput && longInput.acceptableInput
    }

} // Dialog
