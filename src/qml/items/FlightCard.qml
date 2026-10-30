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

// Top of the main menu: everything that is set up before a flight. The active
// aircraft and, in builds with the flight log, the flight recording control,
// grouped in one card.
Pane {
    id: card

    readonly property bool recording: GlobalSettings.autoFlightDetection

    function elapsed(startTime, now) {
        const minutes = Math.max(0, Math.floor((now - startTime) / 60000))
        return Math.floor(minutes / 60) + ":" + String(minutes % 60).padStart(2, "0")
    }

    function openDialog(source) {
        PlatformAdaptor.vibrateBrief()
        Global.drawer.close()
        Global.dialogLoader.active = false
        Global.dialogLoader.setSource(source, {})
        Global.dialogLoader.active = true
    }

    Layout.leftMargin: 8+SafeInsets.left
    Layout.rightMargin: 8
    Layout.topMargin: 8
    Layout.bottomMargin: 8

    padding: 12
    background: Rectangle {
        radius: 16
        color: Qt.alpha("teal", 0.12)
        border.width: card.recording ? 1.5 : 0
        border.color: Qt.alpha("red", 0.6)
    }

    RowLayout {
        anchors.fill: parent
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            TapHandler {
                onTapped: card.openDialog("../dialogs/AircraftSelectionDialog.qml")
            }

            Rectangle {
                implicitWidth: 40
                implicitHeight: 40
                radius: 20
                color: "teal"

                Icon {
                    anchors.centerIn: parent
                    Material.foreground: "white"
                    source: "/icons/material/ic_airplanemode_active.svg"
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 2

                Label {
                    Layout.fillWidth: true

                    elide: Text.ElideRight
                    font.pixelSize: card.font.pixelSize*1.15
                    font.weight: Font.Medium
                    text: (Navigator.aircraftName === "") ? qsTr("Select Aircraft…") : Navigator.aircraftName
                }

                Label {
                    Layout.fillWidth: true

                    elide: Text.ElideRight
                    textFormat: Text.StyledText
                    color: card.Material.secondaryTextColor
                    font.pixelSize: card.font.pixelSize*0.9
                    text: {
                        if (!GlobalSettings.flightLogEnabled)
                            return qsTr("Active aircraft")
                        if (!card.recording)
                            return qsTr("Flight recording off")
                        const dot = "<font color='red'>●</font> "
                        switch (FlightLog.detectionState) {
                        case FlightDetector.TakeoffPhase:
                            return dot + qsTr("Recording · takeoff detected")
                        case FlightDetector.InFlight:
                        case FlightDetector.LandingPhase:
                            if (!isNaN(FlightLog.currentFlightStartTime))
                                return dot + qsTr("Recording · in flight %1").arg(card.elapsed(FlightLog.currentFlightStartTime, Clock.time))
                            return dot + qsTr("Recording · in flight")
                        default:
                            return dot + qsTr("Recording · on ground")
                        }
                    }
                }
            }
        }

        Button {
            visible: GlobalSettings.flightLogEnabled

            Material.roundedScale: Material.FullScale
            Material.foreground: card.recording ? "red" : undefined
            flat: card.recording
            highlighted: !card.recording
            icon.source: card.recording ? "" : "/icons/material/ic_fiber_manual_record.svg"
            icon.color: "red"
            text: card.recording ? qsTr("Stop") : qsTr("Start")

            onClicked: card.openDialog("../dialogs/FlightRecordingDialog.qml")
        }
    }
}
