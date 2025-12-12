/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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

CenteringDialog {
    id: connectionDescription

    required property var connection
    property bool isSerialPort: {
        if (!connectionDescription.connection)
            return false
        return connectionDescription.connection.hasOwnProperty("baudRate")
    }

    Component.onCompleted: {
        if (!connectionDescription.isSerialPort)
            return 0
        switch (connectionDescription.connection.baudRate)  {
        case ConnectionInfo.Baud1200:
            baudRate.currentIndex = 0
            break;
        case ConnectionInfo.Baud2400:
            baudRate.currentIndex = 1
            break;
        case ConnectionInfo.Baud4800:
            baudRate.currentIndex = 2
            break;
        case ConnectionInfo.Baud9600:
            baudRate.currentIndex = 3
            break;
        case ConnectionInfo.Baud19200:
            baudRate.currentIndex = 4
            break;
        case ConnectionInfo.Baud38400:
            baudRate.currentIndex = 5
            break;
        case ConnectionInfo.Baud57600:
            baudRate.currentIndex = 6
            break;
        case ConnectionInfo.Baud115200:
            baudRate.currentIndex = 7
            break;
        }
        switch (connectionDescription.connection.stopBits) {
        case ConnectionInfo.OneStop:
            stopBits.currentIndex = 0
            break;
        case ConnectionInfo.TwoStop:
            stopBits.currentIndex = 1
            break;
        }
        flowControl.currentIndex = connectionDescription.connection.flowControl
    }

    title: qsTr("Connection Info")
    standardButtons: Dialog.Ok

    DecoratedScrollView {
        anchors.fill: parent

        contentHeight: co.height
        contentWidth: availableWidth // Disable horizontal scrolling

        GridLayout {
            id: co
            width: parent.width

            columns: 2

            Label {
                Layout.fillWidth: true
                Layout.columnSpan: 2
                bottomPadding: 0.2*font.pixelSize
                topPadding: 0.2*font.pixelSize
                leftPadding: 0.2*font.pixelSize
                rightPadding: 0.2*font.pixelSize

                text: {
                    if (connectionDescription.connection)
                        return " " + connectionDescription.connection.sourceName
                    return ""
                }

                wrapMode: Text.WordWrap

                background: Rectangle {
                    border.color: "black"
                    color: {
                        if (connectionDescription.connection && connectionDescription.connection.receivingHeartbeat)
                            return "green"
                        if (connectionDescription.connection && connectionDescription.connection.errorString !== "")
                            return "red"
                        return "transparent"
                    }

                    opacity: 0.2
                    radius: 4
                }
            }

            Label {
                Layout.fillWidth: true
                Layout.columnSpan: 2
                Layout.leftMargin: 4
                Layout.rightMargin: 4
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                text: if (connectionDescription.connection) {
                          var sndLine = "<ul style='margin-left:-25px;'>"

                          sndLine += "<li>" + qsTr("Status") +": " + connectionDescription.connection.connectivityStatus + "</li>"
                          if (connectionDescription.connection.errorString !== "")
                              sndLine += "<li>" + qsTr("Error") + ": " + connectionDescription.connection.errorString + "</li>"
                          sndLine += "<li>" + qsTr("Data Format: %1.").arg(connectionDescription.connection.dataFormat) + "</li>"
                          sndLine += "</ul>"
                          if (connectionDescription.connection.canonical)
                              sndLine += "<p>" + qsTr("This is a standard connection that cannot be deleted by the user.") + "</p>"
                          return sndLine
                      } else {
                          return ""
                      }
            }

            Label {
                id: monitor

                Layout.fillWidth: true
                Layout.columnSpan: 2
                bottomPadding: 0.2*font.pixelSize
                topPadding: 0.2*font.pixelSize
                leftPadding: 0.2*font.pixelSize
                rightPadding: 0.2*font.pixelSize

                property string l1: "line 1"
                property string l2: "line 2"
                property string l3: "line 3"
                property string l4: "line 4"
                property string l5: "line 5"

                Connections {
                    target: connectionDescription.connection
                    function onDataReceived(data) {
                        console.log(data)
                        monitor.l1 = monitor.l2
                        monitor.l2 = monitor.l3
                        monitor.l3 = monitor.l4
                        monitor.l4 = monitor.l5
                        monitor.l5 = data
                        monitor.text = monitor.l1 + "<br>" + monitor.l2 + "<br>" + monitor.l3 + "<br>" + monitor.l4 + "<br>" + monitor.l5
                    }
                }

                wrapMode: Text.NoWrap

                background: Rectangle {
                    border.color: "black"
                    color: "transparent"
                    radius: 4
                }
            }

            Label {
                Layout.columnSpan: 2
                Layout.topMargin: font.pixelSize/2
                visible: connectionDescription.isSerialPort
                font.bold: true
                text: qsTr("Configuration")
            }

            Label {
                visible: connectionDescription.isSerialPort
                text: qsTr("Baud Rate")}
            ComboBox {
                id: baudRate

                visible: connectionDescription.isSerialPort
                Layout.fillWidth: true
                onActivated: function (index) {
                    switch(index) {
                    case 0:
                        connectionDescription.connection.baudRate = ConnectionInfo.Baud1200
                        break;
                    case 1:
                        connectionDescription.connection.baudRate = ConnectionInfo.Baud2400
                        break;
                    case 2:
                        connectionDescription.connection.baudRate = ConnectionInfo.Baud4800
                        break;
                    case 3:
                        connectionDescription.connection.baudRate = ConnectionInfo.Baud9600
                        break;
                    case 4:
                        connectionDescription.connection.baudRate = ConnectionInfo.Baud19200
                        break;
                    case 5:
                        connectionDescription.connection.baudRate = ConnectionInfo.Baud38400
                        break;
                    case 6:
                        connectionDescription.connection.baudRate = ConnectionInfo.Baud57600
                        break;
                    case 7:
                        connectionDescription.connection.baudRate = ConnectionInfo.Baud115200
                        break;
                    }
                }

                model: [ 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 ]
            }

            Label {
                visible: connectionDescription.isSerialPort
                text: qsTr("Stop Bits")}
            ComboBox {
                id: stopBits

                Layout.fillWidth: true
                visible: connectionDescription.isSerialPort

                onActivated: function (index) {
                    switch(index) {
                    case 0:
                        connectionDescription.connection.stopBits = ConnectionInfo.OneStop
                        break;
                    case 1:
                        connectionDescription.connection.stopBits = ConnectionInfo.TwoStop
                        break;
                    }
                }

                model: [ 1, 2 ]
            }

            Label {
                visible: connectionDescription.isSerialPort
                text: qsTr("Flow Control")}
            ComboBox {
                id: flowControl

                Layout.fillWidth: true
                visible: connectionDescription.isSerialPort

                onActivated: function (index) {
                    connectionDescription.connection.flowControl = index
                }

                model: [ "None", "RTS/CTS", "XON/XOFF" ]
            }
        }
    }
}
