/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14

import enroute 1.0

Dialog {
    title: qsTr("Satellite Status")

    // Size is chosen so that the dialog does not cover the parent in full
    contentWidth: gl.implicitWidth
    contentHeight: gl.implicitHeight

    standardButtons: Dialog.Ok

    ScrollView {
        id: view
        clip: true
        anchors.fill: parent

        ScrollBar.horizontal.policy: ScrollBar.AsNeeded
        ScrollBar.vertical.policy: ScrollBar.AsNeeded


        GridLayout {
            id: gl
            columnSpacing: 30
            columns: 2

            Text { text: qsTr("Satellite Status") }
            Text {
                font.weight: Font.Bold
                text: satNav.statusAsString
                color: (satNav.status === SatNav.OK) ? "green" : "red"
            }

            Text { text: qsTr("Last Fix") }
            Text { text: satNav.timestampAsString }

            Text { text: qsTr("Mode") }
            Text { text: satNav.isInFlight ? qsTr("Flight") : qsTr("Ground") }

            Text {
                font.pixelSize: Qt.application.font.pixelSize*0.5
                Layout.columnSpan: 2
            }

            Text {
                text: qsTr("Horizontal")
                font.weight: Font.Bold
                Layout.columnSpan: 2
            }

            Text { text: qsTr("Latitude") }
            Text { text: satNav.latitudeAsString }

            Text { text: qsTr("Longitude") }
            Text { text: satNav.longitudeAsString }

            Text { text: qsTr("Error") }
            Text { text: satNav.horizontalPrecisionInMetersAsString }

            Text { text: qsTr("Ground Speed"); textFormat: Text.RichText }
            Text { text: globalSettings.useMetricUnits ? satNav.groundSpeedInKMHAsString : satNav.groundSpeedInKnotsAsString }

            Text { text: qsTr("TT") }
            Text { text: satNav.trackAsString }

            Text {
                font.pixelSize: Qt.application.font.pixelSize*0.5
                Layout.columnSpan: 2
            }

            Text {
                text: qsTr("Vertical")
                font.weight: Font.Bold
                Layout.columnSpan: 2
            }

            Text { text: qsTr("ALT") }
            Text { text: satNav.altitudeInFeetAsString }

        } // GridLayout

    } // Scrollview

    Connections {
        target: sensorGesture
        function onDetected () {
            close()
        }
    }

    onAccepted: {
        // Give feedback
        mobileAdaptor.vibrateBrief()
        close()
    }
} // Dialog
