/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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

import QtQml 2.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import enroute 1.0
import "../dialogs"
import "../items"

Page {
    id: trafficReceiverPage
    title: qsTr("Positioning")

    header: StandardHeader {}

    ScrollView {
        id: view
        clip: true
        anchors.fill: parent
        anchors.topMargin: Qt.application.font.pixelSize
        anchors.bottomMargin: Qt.application.font.pixelSize
        anchors.leftMargin: Qt.application.font.pixelSize
        anchors.rightMargin: Qt.application.font.pixelSize

        // The visibility behavior of the vertical scroll bar is a little complex.
        // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

        GridLayout {
            id: gl
            columnSpacing: 30
            columns: 2

            width: view.width

            Label {
                text: qsTr("<h3>Status</h3>")
                Layout.columnSpan: 2
            }

            Label { // Status
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4
                Layout.columnSpan: 2

                text: positionProvider.statusString

                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                bottomPadding: 0.6*Qt.application.font.pixelSize
                topPadding: 0.6*Qt.application.font.pixelSize
                leftPadding: 0.2*Qt.application.font.pixelSize
                rightPadding: 0.2*Qt.application.font.pixelSize

                leftInset: -4
                rightInset: -4

                background: Rectangle {
                    border.color: "black"
                    color: positionProvider.positionInfo.isValid() ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }
            }

            Item {
                height: Qt.application.font.pixelSize*0.5
                Layout.columnSpan: 2
            }

            Label { text: qsTr("Latitude") }
            Label {
                Layout.fillWidth: true
                text: {
                    if (!positionProvider.positionInfo.isValid())
                        return "-"
                    const lat = positionProvider.positionInfo.coordinate().toString().split(",")[0]
                    if (lat === "")
                        return "-"
                    return lat
                }
            }

            Label { text: qsTr("Longitude") }
            Label {
                text: {
                    if (!positionProvider.positionInfo.isValid())
                        return "-"
                    const lon = positionProvider.positionInfo.coordinate().toString().split(",")[1]
                    if (lon === "")
                        return "-"
                    return lon
                }
            }

            Label { text: qsTr("True Altitude") }
            Label {
                text: {
                    const talt = positionProvider.positionInfo.trueAltitude();
                    return talt.isFinite() ? Math.round(talt.toFeet()) + " ft" : "-"
                }
            }

            Label { text: qsTr("Error (horizontal)") }
            Label {
                text: {
                    const posError = positionProvider.positionInfo.positionErrorEstimate();
                    return posError.isFinite() ? "±" + Math.round(posError.toM()) + " m" : "-"
                }
            }

            Label { text: qsTr("Error (vertical)") }
            Label {
                text: {
                    const taltError = positionProvider.positionInfo.trueAltitudeErrorEstimate();
                    return taltError.isFinite() ? "±" + Math.round(taltError.toFeet()) + " ft" : "-"
                }

            }

            Label { text: qsTr("Magnetic Variation") }
            Label { text: {
                    const magVar = positionProvider.positionInfo.variation();
                    return magVar.isFinite() ? Math.round(magVar.toDEG()) + "°" : "-"
                }
            }

            Label { text: qsTr("Ground Speed") }
            Label {
                text: {
                    const gs = positionProvider.positionInfo.groundSpeed();
                    if (!gs.isFinite())
                        return "-"
                    return global.settings().useMetricUnits ? Math.round(gs.toKMH()) + " km/h" : Math.round(gs.toKN()) + " kn"
                }
            }

            Label { text: qsTr("True Track") }
            Label {
                text: {
                    const tt = positionProvider.positionInfo.trueTrack();
                    return tt.isFinite() ? Math.round(tt.toDEG()) + "°" : "-"
                }
            }

            Label { text: qsTr("Vertical Speed") }
            Label {
                text: {
                    const vs = positionProvider.positionInfo.verticalSpeed();
                    return vs.isFinite() ? Math.round(vs.toFPM()) + " ft/min" : "-"
                }
            }

            Label { text: qsTr("Pressure Altitude") }
            Label { text: positionProvider.pressureAltitude.isFinite() ? Math.round(positionProvider.pressureAltitude.toFeet()) + " ft" : "-" }


        } // GridLayout

    } // Scrollview

    LongTextDialog {
        id: trafficHelp
        standardButtons: Dialog.Ok
        anchors.centerIn: parent

        title: qsTr("Connect your traffic receiver")
        text: librarian.getStringFromRessource(":text/flarmSetup.md")
    }

}
