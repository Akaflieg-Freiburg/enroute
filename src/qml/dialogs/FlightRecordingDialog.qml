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
import "../items"

// Starts or stops flight recording, that is, automatic flight detection.
// Starting asks the user to confirm the aircraft, because the flight entries
// carry its callsign and detection uses its minimum speed.
CenteringDialog {
    id: dlg

    readonly property bool starting: !GlobalSettings.autoFlightDetection
    readonly property bool inFlight: (FlightLog.detectionState === FlightDetector.InFlight)
                                     || (FlightLog.detectionState === FlightDetector.LandingPhase)

    title: dlg.starting ? qsTr("Start Flight Recording") : qsTr("Stop Flight Recording?")
    modal: true

    ColumnLayout {
        anchors.fill: parent

        Label {
            Layout.fillWidth: true

            wrapMode: Text.Wrap
            textFormat: Text.StyledText
            text: {
                if (dlg.starting)
                    return qsTr("Takeoffs and landings will be detected and entered into the flight log until you stop recording.")
                if (dlg.inFlight)
                    return qsTr("A flight is in progress. Stopping now ends the flight, with the current UTC time as landing time.")
                return qsTr("Takeoffs and landings will no longer be detected.")
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.topMargin: dlg.font.pixelSize

            visible: dlg.starting

            Label {
                Layout.fillWidth: true

                wrapMode: Text.Wrap
                textFormat: Text.StyledText
                text: (Navigator.aircraftName === "")
                      ? qsTr("No aircraft selected.")
                      : qsTr("Aircraft: <strong>%1</strong>").arg(Navigator.aircraftName)
            }

            Button {
                flat: true
                text: qsTr("Change…")

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    aircraftSelection.open()
                }
            }
        }
    }

    // Explicit Cancel button: combining standardButtons with a custom
    // button may drop the standard button
    footer: DialogButtonBox {
        Button {
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            flat: true
            text: qsTr("Cancel")
        }

        Button {
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            enabled: !dlg.starting || (Navigator.aircraftName !== "")
            flat: true
            text: dlg.starting ? qsTr("Start") : qsTr("Stop")
        }
    }

    onAccepted: {
        PlatformAdaptor.vibrateBrief()
        if (dlg.starting) {
            GlobalSettings.autoFlightDetection = true
            Global.toast.doToast(qsTr("Flight recording started"))
            return
        }
        // End the flight first: switching detection off discards the track
        // of a flight that is still in progress.
        if (dlg.inFlight)
            FlightLog.endFlight()
        GlobalSettings.autoFlightDetection = false
        Global.toast.doToast(qsTr("Flight recording stopped"))
    }

    AircraftSelectionDialog {
        id: aircraftSelection
    }
}
