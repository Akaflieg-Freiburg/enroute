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
    title: qsTr("Barometric Data")

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
            columns: 2

            width: sView.availableWidth

            Label {
                Layout.columnSpan: 2

                text: qsTr("Pressure Altitude")
                font.pixelSize: sView.font.pixelSize*1.2
                font.bold: true
            }

            Label { // Status
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4
                Layout.columnSpan: 2

                text: PositionProvider.pressureAltitude.isFinite() ? qsTr("Receiving data from traffic receiver") : qsTr("Not connected to a traffic receiver that provides pressure altitude")

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
                    color: PositionProvider.pressureAltitude.isFinite() ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }
            }

            Label { text: qsTr("Pressure Altitude (STD)") }
            Label { text: Navigator.aircraft.verticalDistanceToString( PositionProvider.pressureAltitude ) }


            Label { text: qsTr("QNH") }
            Label { text: WeatherDataProvider.QNHInfo }

            Item {
                Layout.preferredHeight: sView.font.pixelSize*0.5
                Layout.columnSpan: 2
            }

            Label {
                Layout.columnSpan: 2

                text: qsTr("Device Sensors")
                font.pixelSize: sView.font.pixelSize*1.2
                font.bold: true
            }

            Label { // Status
                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4
                Layout.columnSpan: 2

                text: Sensors.statusString

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
                    color: Sensors.ambientPressure.isFinite() ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }
            }

            Label { text: qsTr("Cabin Pressure") }
            Label { text: Sensors.ambientPressure.isFinite() ? Math.round(Sensors.ambientPressure.toHPa()*10.0)/10.0 + " hPa (" + Math.round(Sensors.ambientPressure.toInHg()*100.0)/100.0 + " in Hg)"  : "-" }

            Label { text: qsTr("Cabin Temperature") }
            Label { text: Sensors.ambientTemperature.isFinite() ? Math.round(Sensors.ambientTemperature.toDegreeCelsius()) + " °C (" + Math.round(Sensors.ambientTemperature.toDegreeFarenheit()) + " °F)"  : "-" }

            Label { text: qsTr("Cabin Altitude (STD)") }
            Label { text: Navigator.aircraft.verticalDistanceToString( Sensors.pressureAltitude ) }

        } // GridLayout

    }

}
