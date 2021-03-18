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

import QtGraphicalEffects 1.15
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15
import QtQml 2.15


Rectangle {
    id: flarmWarning
    
    color: "#FF000000"

    height: gl.implicitHeight

    property var radius: 30
    property var ledRadius: 16

    RowLayout {
        id: gl
        
        anchors.fill: parent

        Item { // Spacer
            Layout.fillWidth: true
            height: 2.0*flarmWarning.radius + 1.5*flarmWarning.ledRadius
        }

        Item { // Traffic height indicator
            id: heightIndicator
            width: 2.0*flarmWarning.radius
            height: 2.0*flarmWarning.radius

            LED {
                width: flarmWarning.ledRadius
                height: flarmWarning.ledRadius
                x: flarmWarning.radius - width/2.0
                y: 0.5*flarmWarning.radius - width/2.0
            }

            LED {
                width: flarmWarning.ledRadius
                height: flarmWarning.ledRadius
                x: flarmWarning.radius - width/2.0
                y: 1.5*flarmWarning.radius - width/2.0
            }

            Rectangle {
                width: 2.0*flarmWarning.radius
                height: 4
                x: 0
                y: flarmWarning.radius - height/2.0

                color: "gray"
            }

        }

        Item { // Spacer
            Layout.fillWidth: true
        }

        Label { // Text label
            Layout.maximumWidth: implicitWidth + 24
            Layout.fillWidth: true

            text: "Obstacle at 2 o'clock, in 1.200 m, 300 ft below."
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap

            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
            color: "red"
        }

        Item { // Spacer
            Layout.fillWidth: true
        }

        Item { // Traffic direction indicator
            id: flarmClock
            width: 2.0*flarmWarning.radius
            height: 2.0*flarmWarning.radius

            Rectangle { // Gray clockface
                width: 2.0*flarmWarning.radius
                height: 2.0*flarmWarning.radius
                x: flarmWarning.radius - width/2.0
                y: flarmWarning.radius - height/2.0

                radius: width/2.0

                color: "gray"
            }

            Rectangle { // LED
                width: flarmWarning.ledRadius
                height: flarmWarning.ledRadius
                x: flarmWarning.radius - width/2.0
                y: flarmWarning.radius - width/2.0 - flarmWarning.radius

                radius: width/2.0

                SequentialAnimation on color {
                       loops: Animation.Infinite
                       ColorAnimation {
                           to: "black"
                           duration: 200
                       }
                       ColorAnimation {
                           to: "red"
                           duration: 200
                       }
                   }

            }

            Image {
                id: flarmClockCenter

                width: 24
                height: 24

                x: flarmWarning.radius - width/2.0
                y: flarmWarning.radius - height/2.0

                source: satNav.icon
            }

        }

        Item { // Spacer
            Layout.fillWidth: true
        }

    }

} // Rectangle
