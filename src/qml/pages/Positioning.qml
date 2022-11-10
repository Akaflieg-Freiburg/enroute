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

import QtQml
import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import enroute 1.0
import "../dialogs"
import "../items"

Page {
    id: trafficReceiverPage
    title: qsTr("Positioning")

    header: StandardHeader {}

    ScrollView {
        id: sView
        clip: true

        width: parent.width
        height: parent.height
        bottomPadding: view.font.pixelSize + view.bottomScreenMargin
        leftPadding: view.font.pixelSize + view.leftScreenMargin
        rightPadding: view.font.pixelSize + view.rightScreenMargin
        topPadding: view.font.pixelSize

        // The visibility behavior of the vertical scroll bar is a little complex.
        // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

        GridLayout {
            id: gl
            columnSpacing: 30
            columns: 2

            width: sView.availableWidth

            Label {
                Layout.columnSpan: 2

                text: qsTr("Status")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            Label { // Status
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4
                Layout.columnSpan: 2

                text: global.positionProvider().statusString

                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                bottomPadding: 0.6*view.font.pixelSize
                topPadding: 0.6*view.font.pixelSize
                leftPadding: 0.2*view.font.pixelSize
                rightPadding: 0.2*view.font.pixelSize

                leftInset: -4
                rightInset: -4

                background: Rectangle {
                    border.color: "black"
                    color: global.positionProvider().positionInfo.isValid() ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }
            }

            Item {
                height: view.font.pixelSize*0.5
                Layout.columnSpan: 2
            }

            Label {
                Layout.columnSpan: 2

                text: qsTr("Position Data")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            Label { text: qsTr("Latitude") }
            Label {
                Layout.fillWidth: true
                text: {
                    if (!global.positionProvider().positionInfo.isValid())
                        return "-"
                    const lat = global.positionProvider().positionInfo.coordinate().toString().split(",")[0]
                    if (lat === "")
                        return "-"
                    return lat
                }
            }

            Label { text: qsTr("Longitude") }
            Label {
                text: {
                    if (!global.positionProvider().positionInfo.isValid())
                        return "-"
                    const lon = global.positionProvider().positionInfo.coordinate().toString().split(",")[1].trim()
                    if (lon === "")
                        return "-"
                    return lon
                }
            }

            Label { text: qsTr("True Altitude (AMSL)") }
            Label { text: global.navigator().aircraft.verticalDistanceToString( global.positionProvider().positionInfo.trueAltitudeAMSL() ) }

            Label { text: qsTr("True Altitude (AGL)") }
            Label { text: global.navigator().aircraft.verticalDistanceToString( global.positionProvider().positionInfo.trueAltitudeAGL() ) }

            Label { text: qsTr("Error (horizontal)") }
            Label {
                text: {
                    const posError = global.positionProvider().positionInfo.positionErrorEstimate();
                    return posError.isFinite() ? "±" + Math.round(posError.toM()) + " m" : "-"
                }
            }

            Label { text: qsTr("Error (vertical)") }
            Label { text: global.navigator().aircraft.verticalDistanceToString( global.positionProvider().positionInfo.trueAltitudeErrorEstimate() ) }

            Label { text: qsTr("Magnetic Variation") }
            Label { text: {
                    const magVar = global.positionProvider().positionInfo.variation();
                    return magVar.isFinite() ? Math.round(magVar.toDEG()) + "°" : "-"
                }
            }

            Label { text: qsTr("Ground Speed") }
            Label { text: global.navigator().aircraft.horizontalSpeedToString( global.positionProvider().positionInfo.groundSpeed() ) }

            Label { text: qsTr("True Track") }
            Label {
                text: {
                    const tt = global.positionProvider().positionInfo.trueTrack();
                    return tt.isFinite() ? Math.round(tt.toDEG()) + "°" : "-"
                }
            }

            Label { text: qsTr("Vertical Speed") }
            Label { text: global.navigator().aircraft.verticalSpeedToString( global.positionProvider().positionInfo.verticalSpeed() ) }

            Label { text: qsTr("Pressure Altitude") }
            Label { text: global.navigator().aircraft.verticalDistanceToString( global.positionProvider().pressureAltitude ) }

            Label { text: qsTr("Timestamp") }
            Label { text: global.positionProvider().positionInfo.isValid() ? global.positionProvider().positionInfo.timestampString() : "-" }

        } // GridLayout

    }

}
