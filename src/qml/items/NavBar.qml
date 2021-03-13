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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQml 2.15

Rectangle {
    id: grid
    
    color: "#AA000000"

    height: gl.implicitHeight
    
    GridLayout {
        id: gl
        
        anchors.fill: parent
        rows: 2
        columns: 4
        rowSpacing: 0

        Label {
            Layout.fillWidth: true

            text: satNav.altitudeInFeetAsString
            horizontalAlignment: Text.AlignHCenter
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
            color: "white"
        } // Label
        
        Label {
            Layout.fillWidth: true

            text: globalSettings.useMetricUnits ? satNav.groundSpeedInKMHAsString : satNav.groundSpeedInKnotsAsString

            horizontalAlignment: Text.AlignHCenter
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
            color: "white"
        } // Button

        Label {
            Layout.fillWidth: true

            text: satNav.trackAsString

            horizontalAlignment: Text.AlignHCenter
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
            color: "white"
        } // Label

        Label {
            Layout.fillWidth: true

            text: clock.timeAsUTCString

            horizontalAlignment: Text.AlignHCenter
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
            color: "white"
        } // Label

        Label {
            Layout.fillWidth: true

            color: "white"
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("T.ALT")
        } // Label

        Label {
            Layout.fillWidth: true

            color: "white"
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("GS")
        } // Label

        Label {
            Layout.fillWidth: true

            color: "white"
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("TT")
        } // Label

        Label {
            Layout.fillWidth: true

            color: "white"
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Time")
        } // Label

        Rectangle {
            height: Qt.application.font.pixelSize*0.2
        }

    } // Grid
} // Rectangle
