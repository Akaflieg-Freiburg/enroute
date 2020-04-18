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

Rectangle {
    id: grid
    
    color: "#AA000000"
    
    height: gl.implicitHeight
    
    Grid {
        id: gl
        
        anchors.fill: parent
        rows: 2
        columns: 3
        
        Button {
            Material.foreground: "white"
            flat: true

            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
            width: parent.width/3
            text: satNav.altitudeInFeetAsString

            onClicked: {
                if (!satNav.hasAltitude)
                    return

                MobileAdaptor.vibrateBrief()
                dialogLoader.active = false
                dialogLoader.source = "../dialogs/AltitudeCorrectionDialog.qml"
                dialogLoader.active = true
            }

        } // Button
        
        Button {
            Material.foreground: "white"
            flat: true

            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
            width: parent.width/3
            text: satNav.groundSpeedInKnotsAsString
        } // Button

        Button {
            Material.foreground: "white"
            flat: true

            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
            width: parent.width/3
            text: satNav.trackAsString
        } // Button

        Label {
            color: "white"
            horizontalAlignment: Text.AlignHCenter
            width: parent.width/3
            text: qsTr("Altitude")
        } // Label

        Label {
            color: "white"
            width: parent.width/3

            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Ground Speed")
        } // Label

        Label {
            color: "white"
            width: parent.width/3
            horizontalAlignment: Text.AlignHCenter
            text: qsTr("Track")
        } // Label
    } // Grid
} // Rectangle
