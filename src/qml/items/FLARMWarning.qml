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
    id: flarmWarningIndicator
    
    color: "#FF000000"

    implicitHeight: gl.implicitHeight+1.2*ledRadius

    property var radius: 30
    property var ledRadius: 16

    visible: flarmAdaptor.flarmWarning.alarmLevel > 0

    RowLayout {
        id: gl
        
        anchors.bottomMargin: 0.6*ledRadius
        anchors.topMargin: 0.6*ledRadius
        anchors.fill: parent

        Item { // Spacer
            Layout.fillWidth: true
        }

        Item { // Traffic height indicator
            id: heightIndicator
            width: 2.0*flarmWarningIndicator.radius
            height: 2.0*flarmWarningIndicator.radius
            visible: flarmAdaptor.flarmWarning.alarmLevel > 0

            Rectangle {
                width: 2.0*flarmWarningIndicator.radius
                height: 4
                x: 0
                y: flarmWarningIndicator.radius - height/2.0

                color: "gray"
            }

            LED {
                visible: flarmAdaptor.flarmWarning.vDist.toM >= 100
                blinking: visible
                width: flarmWarningIndicator.ledRadius
                height: flarmWarningIndicator.ledRadius
                x: flarmWarningIndicator.radius - width/2.0
                y: 0.5*flarmWarningIndicator.radius - width/2.0
            }

            LED {
                visible: (flarmAdaptor.flarmWarning.vDist.toM > -100) && (flarmAdaptor.flarmWarning.vDist.toM < 100)
                blinking: visible
                width: 1.5*flarmWarningIndicator.ledRadius
                height: 1.5*flarmWarningIndicator.ledRadius
                x: flarmWarningIndicator.radius - width/2.0
                y: flarmWarningIndicator.radius - width/2.0
            }

            LED {
                visible: flarmAdaptor.flarmWarning.vDist.toM < -100
                blinking: visible
                width: flarmWarningIndicator.ledRadius
                height: flarmWarningIndicator.ledRadius
                x: flarmWarningIndicator.radius - width/2.0
                y: 1.5*flarmWarningIndicator.radius - width/2.0
            }

        }

        Item { // Spacer
            Layout.fillWidth: true
        }

        Label { // Text label
            Layout.maximumWidth: implicitWidth + 24
            Layout.fillWidth: true

            text: flarmAdaptor.flarmWarning.description
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            color: {
                if (flarmAdaptor.flarmWarning.alarmLevel === 0) {
                    return "white"
                }
                if (flarmAdaptor.flarmWarning.alarmLevel === 1) {
                    return "yellow"
                }
                return "red"
            }
            Behavior on color { ColorAnimation { duration: 100 } }

            font.weight: flarmAdaptor.flarmWarning.alarmLevel > 0 ? Font.Bold : Font.Normal
            font.pixelSize: flarmAdaptor.flarmWarning.alarmLevel > 0 ? Qt.application.font.pixelSize*1.3 : Qt.application.font.pixelSize
        }

        Item { // Spacer
            Layout.fillWidth: true
        }

        Item { // Traffic direction indicator
            id: flarmClock
            width: 2.0*flarmWarningIndicator.radius
            height: 2.0*flarmWarningIndicator.radius
            visible: flarmAdaptor.flarmWarning.alarmLevel > 0

            Rectangle { // Gray clockface
                width: 2.0*flarmWarningIndicator.radius
                height: 2.0*flarmWarningIndicator.radius
                x: flarmWarningIndicator.radius - width/2.0
                y: flarmWarningIndicator.radius - height/2.0

                radius: width/2.0

                color: "gray"
            }

            LED { // LED
                width: flarmWarningIndicator.ledRadius
                height: flarmWarningIndicator.ledRadius
                x: flarmWarningIndicator.radius*(1 + Math.sin(2*Math.PI*(flarmAdaptor.flarmWarning.relativeBearing)/360.0)) - width/2.0
                y: flarmWarningIndicator.radius*(1 - Math.cos(2*Math.PI*(flarmAdaptor.flarmWarning.relativeBearing)/360.0)) - width/2.0
                visible: !isNaN(flarmAdaptor.flarmWarning.relativeBearing)
                blinking: visible
            }

            Image {
                id: flarmClockCenter

                width: 24
                height: 24

                x: flarmWarningIndicator.radius - width/2.0
                y: flarmWarningIndicator.radius - height/2.0

                source: satNav.icon
            }

        }

        Item { // Spacer
            Layout.fillWidth: true
        }

    }

} // Rectangle
