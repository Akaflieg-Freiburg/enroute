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

    border.color: GlobalSettings.nightMode ? "white" : "black"
    color: GlobalSettings.nightMode ? "black" : "white"
    radius: 0.5*font.pixelSize
    clip: true

    implicitHeight: notifyCol.implicitHeight+font.pixelSize
    visible: NotificationManager.currentVisualNotification

    Connections {
        target: NotificationManager

        function onCurrentVisualNotificationChanged() { blink.start() }
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
            to: GlobalSettings.nightMode ? "black" : "white"
        }

    }

    ColumnLayout {
        id: notifyCol

        width: parent.width

        WordWrappingItemDelegate {
            Layout.fillWidth: true
            text: {
                if (!NotificationManager.currentVisualNotification)
                    return ""

                return NotificationManager.currentVisualNotification.title +
                  (GlobalSettings.nightMode ? `<br><font color="#FFFFFF" size="2">` : `<br><font color="#606060" size="2">`) +
                  NotificationManager.currentVisualNotification.text +
                  `</font>`
            }

            icon.source: {
                if (NotificationManager.currentVisualNotification)
                {
                    if (NotificationManager.currentVisualNotification.importance === Notification.Warning)
                        return "/icons/material/ic_warning.svg"
                    if (NotificationManager.currentVisualNotification.importance === Notification.Warning_Navigation)
                        return "/icons/material/ic_warning.svg"
                    if (NotificationManager.currentVisualNotification.importance === Notification.Alert)
                        return "/icons/material/ic_warning.svg"
                }
                return "/icons/material/ic_info.svg"
            }

            onClicked: {
                if (!NotificationManager.currentVisualNotification)
                    return

                if (NotificationManager.currentVisualNotification.textBodyAction === Notification.OpenMapsAndDataPage)
                {
                    PlatformAdaptor.vibrateBrief()
                    stackView.push("../pages/DataManagerPage.qml", {"dialogLoader": dialogLoader, "stackView": stackView})
                }
                if (NotificationManager.currentVisualNotification.textBodyAction === Notification.OpenTrafficReceiverPage)
                {
                    PlatformAdaptor.vibrateBrief()
                    stackView.push("../pages/TrafficReceiver.qml", {"appWindow": view})
                }
            }
        }

        DialogButtonBox {
            Layout.leftMargin: 0.2*font.pixelSize
            Layout.rightMargin: 0.2*font.pixelSize
            Layout.fillWidth: true

            clip: true

            background: Item {}

            NotificationButton {
                text: NotificationManager.currentVisualNotification ? NotificationManager.currentVisualNotification.button2Text : ""
                visible: text !== ""
                flat: true

                onClicked:  {
                    PlatformAdaptor.vibrateBrief()
                    if (NotificationManager.currentVisualNotification)
                        NotificationManager.currentVisualNotification.onButton2Clicked()
                }
            }

            NotificationButton {
                text: NotificationManager.currentVisualNotification ? NotificationManager.currentVisualNotification.button1Text : ""
                visible: text !== ""
                flat: true

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    if (NotificationManager.currentVisualNotification)
                        NotificationManager.currentVisualNotification.onButton1Clicked()
                }
            }

        }

    }
}
