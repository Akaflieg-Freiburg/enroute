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

import enroute 1.0

Dialog {
    id: flarmStatusDialog

    title: qsTr("Traffic Receiver")

    // Size is chosen so that the dialog does not cover the parent in full
    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

    standardButtons: Dialog.Ok

    ScrollView {
        id: view
        clip: true
        anchors.fill: parent

        // The visibility behavior of the vertical scroll bar is a little complex.
        // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

        ColumnLayout {
            id: gl

            width: flarmStatusDialog.availableWidth

            Label {

                text:  {
                    if (flarmAdaptor.status == FLARMAdaptor.Disconnected)
                        return qsTr("Not connected")
                    if (flarmAdaptor.status == FLARMAdaptor.Connecting)
                        return qsTr("Trying to connect to traffic receiver at IP address 192.168.1.1, port 2000 …")
                    if (flarmAdaptor.status == FLARMAdaptor.Connected)
                        return qsTr("Connected to traffic receiver at IP address 192.168.1.1, port 2000. Waiting for data …")
                    return qsTr("Connected to traffic receiver at IP address 192.168.1.1, port 2000. Receiving traffic information …")
                }

                Layout.fillWidth: true
                Layout.leftMargin: 4
                Layout.rightMargin: 4
                wrapMode: Text.WordWrap

                bottomPadding: 0.2*Qt.application.font.pixelSize
                topPadding: 0.2*Qt.application.font.pixelSize
                leftPadding: 0.2*Qt.application.font.pixelSize
                rightPadding: 0.2*Qt.application.font.pixelSize

                leftInset: -4
                rightInset: -4

                // Background color according to METAR/FAA flight category
                background: Rectangle {
                    border.color: "black"
                    color: (flarmAdaptor.status === FLARMAdaptor.Receiving) ? "green" : "red"
                    opacity: 0.2
                    radius: 4
                }
            }


            Text {
                Layout.fillWidth: true
                text: flarmAdaptor.lastError
                wrapMode: Text.Wrap
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: {
                    if (flarmAdaptor.status === FLARMAdaptor.Disconnected)
                        return qsTr("Connect")
                    if (flarmAdaptor.status === FLARMAdaptor.Connecting)
                        return qsTr("Abort")
                    qsTr("Disconnect")
                }
                enabled: !timer.running
                onClicked: {
                    if (flarmAdaptor.status == FLARMAdaptor.Disconnected)
                        flarmAdaptor.connectToDevice()
                    else
                        flarmAdaptor.disconnectFromDevice()
                    timer.running = true;
                }

                Timer {
                    id: timer
                    interval: 1000
                }

            }

        } // GridLayout

    } // Scrollview

    onAccepted: {
        // Give feedback
        mobileAdaptor.vibrateBrief()
        close()
    }
} // Dialog
