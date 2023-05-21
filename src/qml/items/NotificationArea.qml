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
    id: mainRect

    border.color: "black"
    color: "white"
    radius: 0.5*font.pixelSize

    implicitHeight: notifyCol.implicitHeight+font.pixelSize
    visible: NotificationManager.currentNotification

    Connections {
        target: NotificationManager

        function onCurrentNotificationChanged() { blink.start() }
    }

    SequentialAnimation {
        id: blink

        ColorAnimation {
            target: mainRect
            property: "color"
            duration: 50
            to: "red"
        }
        ColorAnimation {
            target: mainRect
            property: "color"
            duration: 50
            to: "white"
        }

    }


    ColumnLayout {
        id: notifyCol

        width: parent.width

        WordWrappingItemDelegate {
            Layout.fillWidth: true
            text: {
                if (!NotificationManager.currentNotification)
                    return ""

                return NotificationManager.currentNotification.title +
                  `<br><font color="#606060" size="2">` +
                  NotificationManager.currentNotification.text +
                  `</font>`
            }

            icon.source: {
                if (NotificationManager.currentNotification)
                {
                    if (NotificationManager.currentNotification.importance === Notification.Warning)
                        return "/icons/material/ic_warning.svg"
                    if (NotificationManager.currentNotification.importance === Notification.Warning_Navigation)
                        return "/icons/material/ic_warning.svg"
                    if (NotificationManager.currentNotification.importance === Notification.Alert)
                        return "/icons/material/ic_warning.svg"
                }
                return "/icons/material/ic_info.svg"
            }

            onClicked: {
                if (!NotificationManager.currentNotification)
                    return

                if (NotificationManager.currentNotification.textBodyAction === Notification.OpenMapsAndDataPage)
                {
                    PlatformAdaptor.vibrateBrief()
                    stackView.push("../pages/DataManagerPage.qml", {"dialogLoader": dialogLoader, "stackView": stackView})
                }
                if (NotificationManager.currentNotification.textBodyAction === Notification.OpenTrafficReceiverPage)
                {
                    PlatformAdaptor.vibrateBrief()
                    stackView.push("../pages/TrafficReceiver.qml", {"appWindow": view})
                }
            }
        }

        DialogButtonBox {
            Layout.leftMargin: 0.2*font.pixelSize
            Layout.preferredHeight: 1.2*font.pixelSize
            Layout.rightMargin: 0.2*font.pixelSize
            clip: true

            ToolButton {
                text: NotificationManager.currentNotification ? NotificationManager.currentNotification.button1Text : ""
                visible: text !== ""

                onClicked: {
                    if (NotificationManager.currentNotification)
                        NotificationManager.currentNotification.onButton1Clicked()
                }
            }

            ToolButton {
                text: NotificationManager.currentNotification ? NotificationManager.currentNotification.button2Text : ""
                visible: text !== ""

                onClicked:  {
                    if (NotificationManager.currentNotification)
                        NotificationManager.currentNotification.onButton2Clicked()
                }
            }
        }
    }
}
