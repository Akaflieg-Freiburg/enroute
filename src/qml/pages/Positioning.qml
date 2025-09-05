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

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../dialogs"
import "../items"

Page {
    id: trafficReceiverPage
    title: qsTr("SatNav Positioning")

    header: StandardHeader {}

    DecoratedScrollView {
        id: sView

        anchors.fill: parent
        contentWidth: availableWidth // Disable horizontal scrolling

        clip: true

        bottomPadding: font.pixelSize + SafeInsets.bottom
        leftPadding: font.pixelSize + SafeInsets.left
        rightPadding: font.pixelSize + SafeInsets.right
        topPadding: font.pixelSize

        GridLayout {
            id: gl
            columnSpacing: 30
            columns: 3

            width: sView.availableWidth

            Label {
                Layout.columnSpan: 2
                text: qsTr("Status")
                font.pixelSize: trafficReceiverPage.font.pixelSize*1.2
                font.bold: true
            }
            ToolButton { enabled: false }

            Label { // Status
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4
                Layout.columnSpan: 3

                text: PositionProvider.statusString

                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                bottomPadding: 0.6*font.pixelSize
                topPadding: 0.6*font.pixelSize
                leftPadding: 0.2*font.pixelSize
                rightPadding: 0.2*font.pixelSize

                leftInset: -4
                rightInset: -4

                background: Rectangle {
                    border.color: "black"
                    color: PositionProvider.positionInfo.isValid() ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }
            }

            Label {
                Layout.columnSpan: 2
                text: qsTr("Position")
                font.pixelSize: trafficReceiverPage.font.pixelSize*1.2
                font.bold: true
            }
            ToolButton { enabled: false }

            Label { text: qsTr("Latitude") }
            Label {
                Layout.fillWidth: true
                text: {
                    if (!PositionProvider.positionInfo.isValid())
                        return "-"
                    const lat = PositionProvider.positionInfo.coordinate().toString().split(",")[0]
                    if (lat === "")
                        return "-"
                    return lat
                }
            }
            Item {}

            Label { text: qsTr("Longitude") }
            Label {
                text: {
                    if (!PositionProvider.positionInfo.isValid())
                        return "-"
                    const lon = PositionProvider.positionInfo.coordinate().toString().split(",")[1].trim()
                    if (lon === "")
                        return "-"
                    return lon
                }
            }
            Item {}

            Label { text: qsTr("Error (horizontal)") }
            Label {
                text: {
                    const posError = PositionProvider.positionInfo.positionErrorEstimate();
                    return posError.isFinite() ? "±" + Math.round(posError.toM()) + " m" : "-"
                }
            }
            Item {}

            Label { text: qsTr("Ground Speed") }
            Label { text: Navigator.aircraft.horizontalSpeedToString( PositionProvider.positionInfo.groundSpeed() ) }
            Item {}

            Label { text: qsTr("True Track") }
            Label {
                text: {
                    const tt = PositionProvider.positionInfo.trueTrack();
                    return tt.isFinite() ? Math.round(tt.toDEG()) + "°" : "-"
                }
            }
            Item {}

            Label { text: qsTr("Error (True Track)") }
            Label {
                text: {
                    const tt = PositionProvider.positionInfo.trueTrackErrorEstimate();
                    return tt.isFinite() ? "±" + Math.round(tt.toDEG()) + "°" : "-"
                }
            }
            Item {}

            Label {
                Layout.columnSpan: 2
                text: qsTr("True Altitude")
                font.pixelSize: trafficReceiverPage.font.pixelSize*1.2
                font.bold: true
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    helpDialog.title = qsTr("Altitudes")
                    helpDialog.text = "<p>"+qsTr("True altitude AGL or AMSL is the vertical distance from the aircraft to the terrain or to the main sea level, respectively.")+"</p>"
                            +"<p>"+qsTr("<strong>Warning:</strong> Vertical airspace limits are defined in terms of barometric altitude. Depending on weather, true altitude and barometric altitude may differ substantially. <strong>Never use true altitude to judge the vertical distance from your aircraft to an airspace boundary.</strong>")+"</p>"
                    helpDialog.open()
                }
            }

            Label { text: qsTr("True Altitude") }
            Label { text: PositionProvider.positionInfo.trueAltitudeAMSL().isFinite() ? Navigator.aircraft.verticalDistanceToString( PositionProvider.positionInfo.trueAltitudeAMSL() ) + " AMSL" : "-" }
            Item { }

            Item { }
            Label { text: PositionProvider.positionInfo.trueAltitudeAGL().isFinite() ? Navigator.aircraft.verticalDistanceToString( PositionProvider.positionInfo.trueAltitudeAGL() ) + " AGL" : "-" }
            Item { }

            Label { text: qsTr("Error (vertical)") }
            Label { text: Navigator.aircraft.verticalDistanceToString( PositionProvider.positionInfo.trueAltitudeErrorEstimate() ) }
            Item {}

            Label { text: qsTr("Vertical Speed") }
            Label { text: Navigator.aircraft.verticalSpeedToString( PositionProvider.positionInfo.verticalSpeed() ) }
            Item {}

            Label {
                Layout.columnSpan: 2
                text: qsTr("Other")
                font.pixelSize: trafficReceiverPage.font.pixelSize*1.2
                font.bold: true
            }
            ToolButton { enabled: false }

            Label { text: qsTr("Magnetic Variation") }
            Label { text: {
                    const magVar = PositionProvider.positionInfo.variation();
                    return magVar.isFinite() ? Math.round(magVar.toDEG()) + "°" : "-"
                }
            }
            Item {}

            Label { text: qsTr("Timestamp") }
            Label { text: PositionProvider.positionInfo.isValid() ? PositionProvider.positionInfo.timestampString() : "-" }
            Item {}

        } // GridLayout

    }

    LongTextDialog {
        id: helpDialog

        modal: true

        standardButtons: Dialog.Ok
    }
}
