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
import QtQuick.Shapes

import akaflieg_freiburg.enroute
import "../items"

/* Waypoint dialog
 *
 * Short description: set property waypoint, then open. Re-implement onAccepted and read the new values from newName, newLatitude
 * and newLongitude.
 */

CenteringDialog {
    id: waypointEditorDialog

    // Property waypoint, and code to handle waypoint changes
    property waypoint waypoint: GeoMapProvider.createWaypoint()

    readonly property string newName: wpNameField.text
    readonly property string newNotes: wpNotesField.text
    readonly property double newLatitude: latInput.value
    readonly property double newLongitude: longInput.value
    readonly property double newAltitudeMeter: eleField.valueMeter

    modal: true
    title: qsTr("Edit Waypoint")

    standardButtons: Dialog.Cancel|Dialog.Ok

    onAboutToShow: {
        // This is necessary, because the initial binding "latInput.value: waypoint.coordinate.latitude"
        // breaks as soon as the user edits the coordinates manually.
        latInput.value = waypoint.coordinate.latitude
        longInput.value = waypoint.coordinate.longitude
        wpNameField.text = waypoint.extendedName
        wpNotesField.text = waypoint.notes
    }

    DecoratedScrollView {
        anchors.fill: parent
        contentWidth: availableWidth // Disable horizontal scrolling

        clip: true

        // If virtual keyboard come up, make sure that the focused element is visible
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
                text: qsTr("Name")
            }

            MyTextField {
                id: wpNameField

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5

                text: waypoint.extendedName
                focus: true
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                text: qsTr("Notes")
            }

            TextArea {
                id: wpNotesField

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.minimumWidth: font.pixelSize*5

                focus: true

                wrapMode: TextEdit.WordWrap

                Component.onCompleted: PlatformAdaptor.setupInputMethodEventFilter(wpNotesField)
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


            WordWrappingItemDelegate {
                id: ios_verticalUOM
                Layout.columnSpan: 2
                Layout.fillWidth: true
                visible: (Qt.platform.os === "ios")

                text: {
                    var unitString = qsTr("Degrees")
                    if (formatChoice.currentIndex === 1)
                        unitString = qsTr("Degrees and Minutes")
                    if (formatChoice.currentIndex === 2)
                        unitString = qsTr("Degrees, Minutes and Seconds")

                    return qsTr("Coordinate Format") +
                            '<br><font color="#606060" size="2">' +
                            qsTr("Currently using: %1").arg(unitString) +
                            '</font>'
                }
                icon.source: "/icons/material/ic_my_location.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    degreeFormatDialog.open()
                }
                CenteringDialog {
                    id: degreeFormatDialog

                    title: qsTr("Coordinate Format")
                    standardButtons: Dialog.Ok|Dialog.Cancel

                    DecoratedScrollView{
                        anchors.fill: parent
                        contentWidth: availableWidth // Disable horizontal scrolling

                        // Delays evaluation and prevents binding loops
                        Binding on implicitHeight {
                            value: cl.implicitHeight
                            delayed: true    // Prevent intermediary values from being assigned
                        }

                        clip: true

                        ColumnLayout {
                            id: cl
                            width: degreeFormatDialog.availableWidth

                            WordWrappingRadioDelegate {
                                id: a
                                Layout.fillWidth: true
                                text: qsTr("Degrees")
                            }
                            WordWrappingRadioDelegate {
                                id: b
                                Layout.fillWidth: true
                                text: qsTr("Degrees and Minutes")
                            }
                            WordWrappingRadioDelegate {
                                id: c
                                Layout.fillWidth: true
                                text: qsTr("Degrees, Minutes and Seconds")
                            }
                        }
                    }

                    onAboutToShow: {
                        a.checked = formatChoice.currentIndex === 0
                        b.checked = formatChoice.currentIndex === 1
                        c.checked = formatChoice.currentIndex === 2
                    }

                    onAccepted: {
                        if (a.checked)
                            formatChoice.currentIndex = 0
                        if (b.checked)
                            formatChoice.currentIndex = 1
                        if (c.checked)
                            formatChoice.currentIndex = 2
                    }
                }
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                visible: (Qt.platform.os !== "ios")

                text: qsTr("Format")
            }

            ComboBox {
                id: formatChoice
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.rightMargin: 3
                visible: (Qt.platform.os !== "ios")

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

            WordWrappingItemDelegate {
                id: ios_coordinateFormat
                Layout.columnSpan: 2
                Layout.fillWidth: true
                visible: (Qt.platform.os === "ios")

                text: {
                    var unitString = qsTr("Feet")
                    if (eleFormatChoice.currentIndex === 1) {
                        unitString = qsTr("Meter")
                    }
                    return qsTr("Elevation Unit") +
                            '<br><font color="#606060" size="2">' +
                            qsTr("Currently using: %1").arg(unitString) +
                            '</font>'
                }
                icon.source: "/icons/material/ic_arrow_upward.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    verticalUOMDialog.open()
                }
                CenteringDialog {
                    id: verticalUOMDialog

                    title: qsTr("Elevation Unit")
                    standardButtons: Dialog.Ok|Dialog.Cancel

                    DecoratedScrollView{
                        anchors.fill: parent
                        contentWidth: availableWidth // Disable horizontal scrolling


                        // Delays evaluation and prevents binding loops
                        Binding on implicitHeight {
                            value: cl1.implicitHeight
                            delayed: true    // Prevent intermediary values from being assigned
                        }

                        clip: true

                        ColumnLayout {
                            id: cl1
                            width: verticalUOMDialog.availableWidth

                            WordWrappingRadioDelegate {
                                id: a1
                                Layout.fillWidth: true
                                text: qsTr("Feet")
                            }
                            WordWrappingRadioDelegate {
                                id: b1
                                Layout.fillWidth: true
                                text: qsTr("Meter")
                            }
                        }
                    }

                    onAboutToShow: {
                        a1.checked = eleFormatChoice.currentIndex === 0
                        b1.checked = eleFormatChoice.currentIndex === 1
                    }

                    onAccepted: {
                        if (a1.checked)
                            eleFormatChoice.currentIndex = 0
                        if (b1.checked)
                            eleFormatChoice.currentIndex = 1
                    }

                }
            }

            Label {
                Layout.alignment: Qt.AlignBaseline
                visible: (Qt.platform.os !== "ios")

                text: qsTr("Unit")
            }

            ComboBox {
                id: eleFormatChoice
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                Layout.rightMargin: 3
                visible: (Qt.platform.os !== "ios")

                model: [ qsTr("Feet"), qsTr("Meter") ]
            }


        }
    }

    function enableOk() {
        waypointEditorDialog.standardButton(DialogButtonBox.Ok).enabled = latInput.acceptableInput && longInput.acceptableInput
    }

} // Dialog
