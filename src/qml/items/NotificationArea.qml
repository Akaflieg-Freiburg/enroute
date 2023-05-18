/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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
import QtQuick.Controls
import QtQuick.Layouts

import akaflieg_freiburg.enroute


Rectangle {
    border.color: "black"
    color: "white"
    radius: 0.5*font.pixelSize

    implicitHeight: notifyCol.implicitHeight+font.pixelSize
    visible: NotificationManager.currentNotification

    ColumnLayout {
        id: notifyCol

        x: 0.5*font.pixelSize
        y: 0.5*font.pixelSize
        width: parent.width-font.pixelSize

        Label {
            Layout.fillWidth: true
            text: NotificationManager.currentNotification ? NotificationManager.currentNotification.title : ""
            font.bold: true
            wrapMode: Text.Wrap
        }
        Label {
            Layout.fillWidth: true
            visible: text !== ""
            text: NotificationManager.currentNotification ? NotificationManager.currentNotification.text : ""
            wrapMode: Text.Wrap
        }

        RowLayout {

            ToolButton {
                text: NotificationManager.currentNotification ? NotificationManager.currentNotification.button1Text : ""
                Layout.preferredHeight: 1.2*font.pixelSize
                onClicked: {
                    if (NotificationManager.currentNotification)
                        NotificationManager.currentNotification.onButton1Clicked()
                }
            }
            ToolButton {
                text: NotificationManager.currentNotification ? NotificationManager.currentNotification.button2Text : ""
                Layout.preferredHeight: 1.2*font.pixelSize

                onClicked:  {
                    if (NotificationManager.currentNotification)
                        NotificationManager.currentNotification.onButton2Clicked()
                }
            }
        }
    }
}
