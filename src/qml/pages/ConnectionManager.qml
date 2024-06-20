/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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
    objectName: "TrafficReceiverPage"

    title: qsTr("Data Connections")

    required property var appWindow

    header: PageHeader {

        height: 60 + SafeInsets.top
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: SafeInsets.top

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                stackView.pop()
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.pixelSize: 20
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_info_outline.svg"
            onClicked: {
                PlatformAdaptor.vibrateBrief()
                openManual("05-referencePages/03-settingsDataConnections.html")
            }
        }

    }


    DecoratedListView {
        anchors.fill: parent
        contentWidth: availableWidth // Disable horizontal scrolling

        clip: true

        model: TrafficDataProvider.dataSources

        header: Label {
            height: 2*implicitHeight
            width: parent ? parent.width : 0

            text: qsTr("Traffic Data Receivers")

            leftPadding: trafficReceiverPage.font.pixelSize
            font.pixelSize: trafficReceiverPage.font.pixelSize*1.2
            font.bold: true
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.WordWrap
        }

        delegate: Item {
            width: parent ? parent.width : 0
            height: idel.implicitHeight

            Rectangle {
                anchors.fill: parent
                color: {
                    if (model.modelData.receivingHeartbeat)
                        return "green"
                    if (model.modelData.errorString !== "")
                        return "red"
                    return "transparent"
                }
                opacity: 0.2
            }

            RowLayout {
                width: parent.width

                WordWrappingItemDelegate {
                    id: idel
                    Layout.fillWidth: true

                    //enabled: model.modelData.canConnect
                    icon.source: model.modelData.icon
                    text: {
                        var sndLine = model.modelData.connectivityStatus
                        if (model.modelData.errorString !== "")
                            sndLine += " • " + qsTr("Error") + ": " + model.modelData.errorString
                        model.modelData.sourceName + "<br><font size='2'>%1</font>".arg(sndLine)
                    }

                    onClicked: {
                        connectionDescription.connection = model.modelData
                        connectionDescription.open()
                    }
                }

                ToolButton {
                    id: cptMenuButton

                    icon.source: "/icons/material/ic_more_horiz.svg"
                    enabled: !model.modelData.canonical

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        cptMenu.open()
                    }

                    AutoSizingMenu {
                        id: cptMenu

                        Action {
                            id: removeAction
                            text: qsTr("Remove…")
                            onTriggered: {
                                PlatformAdaptor.vibrateBrief()
                                Global.toast.doToast( qsTr("Removing Connection: %1").arg(model.modelData.sourceName))
                                TrafficDataProvider.removeDataSource(model.modelData)
                                cptMenu.close()
                            }
                        }
                    }
                }
            }
        }
    }


    footer: Footer {
        ColumnLayout {
            width: parent.width

            Button {
                flat: true

                Layout.alignment: Qt.AlignHCenter
                icon.source: "/icons/material/ic_refresh.svg"
                text: {
                    if (disconnectTimer.running)
                        return qsTr("Disconnecting...")
                    if (connectTimer.running)
                        return qsTr("Reconnecting...")
                    return qsTr("Reconnect")
                }
                enabled: !connectTimer.running
                visible: !TrafficDataProvider.receivingHeartbeat
                onClicked: {
                    TrafficDataProvider.disconnectFromTrafficReceiver()
                    disconnectTimer.running = true;
                    connectTimer.running = true;
                }
                Timer {
                    id: disconnectTimer
                    interval: 1000
                    onTriggered: TrafficDataProvider.connectToTrafficReceiver()
                }
                Timer {
                    id: connectTimer
                    interval: 2000
                }
            }

            Button {
                flat: true

                Layout.alignment: Qt.AlignHCenter
                text: qsTr("New Connection")
                icon.source: "/icons/material/ic_add_circle.svg"

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    addMenu.open()
                }

                AutoSizingMenu {
                    id: addMenu

                    Action {
                        text: qsTr("Bluetooth Classic")
                        enabled: (Qt.platform.os !== "ios")
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            Global.dialogLoader.active = false
                            Global.dialogLoader.setSource("../dialogs/AddBTDeviceDialog.qml", {})
                            Global.dialogLoader.active = true
                            addMenu.close()
                        }
                    }

                    Action {
                        text: qsTr("Serial Port Connection")
                        enabled: (Qt.platform.os !== "ios") && (Qt.platform.os !== "android")
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            addSerialPortDialog.open()
                            addMenu.close()
                        }
                    }

                    Action {
                        text: qsTr("TCP Connection")
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            addTCPDialog.open()
                            addMenu.close()
                        }
                    }

                    Action {
                        text: qsTr("UDP Connection")

                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            addUDPDialog.open()
                            addMenu.close()
                        }
                    }

                }

            }
        }
    }


    LongTextDialog {
        id: ltd
        title: qsTr("Error Adding Connection")
        standardButtons: Dialog.Ok
    }

    CenteringDialog {
        id: connectionDescription

        property var connection

        title: qsTr("Connection Info")
        standardButtons: Dialog.Ok

        DecoratedScrollView {
            anchors.fill: parent

            contentHeight: co.height
            contentWidth: availableWidth // Disable horizontal scrolling

            ColumnLayout {
                id: co
                width: parent.width

                Label {
                    Layout.fillWidth: true
                    text: {
                        if (connectionDescription.connection)
                            return connectionDescription.connection.sourceName
                        return ""
                    }
                    wrapMode: Text.WordWrap
                }

                Label {
                    Layout.fillWidth: true
                    text: if (connectionDescription.connection) {
                              var sndLine = "<strong>" + qsTr("Status") +":</strong> " + connectionDescription.connection.connectivityStatus
                              if (connectionDescription.connection.errorString !== "")
                                  sndLine += "<br><strong>" + qsTr("Error") + ":</strong> " + connectionDescription.connection.errorString
                              return sndLine
                          } else {
                              return ""
                          }

                    Layout.leftMargin: 4
                    Layout.rightMargin: 4
                    wrapMode: Text.WordWrap

                    bottomPadding: 0.2*font.pixelSize
                    topPadding: 0.2*font.pixelSize
                    leftPadding: 0.2*font.pixelSize
                    rightPadding: 0.2*font.pixelSize

                    leftInset: -4
                    rightInset: -4

                    // Background color according to METAR/FAA flight category
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
                    text: {
                        if (!connectionDescription.connection)
                            return ""
                        var s = qsTr("Data Format: %1.").arg(connectionDescription.connection.dataFormat)
                        if (connectionDescription.connection.canonical)
                            s += "<br>" + qsTr("This is a standard connection that cannot be deleted by the user.")
                        return s
                    }
                    wrapMode: Text.WordWrap
                }

            }
        }
    }

    CenteringDialog {
        id: addUDPDialog

        modal: true
        title: qsTr("Add UDP Connection")
        standardButtons: Dialog.Cancel|Dialog.Ok

        onAboutToShow: mtf.text = ""

        GridLayout {
            width: addUDPDialog.availableWidth
            columns: 2

            Label {
                text: qsTr("Please enter the port used by your traffic data receiver.")
                      + " "
                      + qsTr("This is a number between 0 and 65535.")
                wrapMode: Text.WordWrap
                Layout.columnSpan: 2
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Port")
            }
            MyTextField {
                id: mtf
                focus: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator {
                    bottom: 0
                    top: 65535
                }
                onAcceptableInputChanged: {
                    addUDPDialog.standardButton(DialogButtonBox.Ok).enabled = acceptableInput
                }
                onAccepted: addUDPDialog.accepted()
            }
        }

        onAccepted: {
            var resultString = TrafficDataProvider.addDataSource_UDP(Number(mtf.text))
            if (resultString !== "")
            {
                ltd.text = resultString
                ltd.open()
                return
            }
            Global.toast.doToast( qsTr("Adding UDP Connection: Port %1").arg(mtf.text) )
            addUDPDialog.close()
        }

    }

    CenteringDialog {
        id: addTCPDialog

        modal: true
        title: qsTr("Add TCP Connection")
        standardButtons: Dialog.Cancel|Dialog.Ok

        onAboutToShow: {
            host.text = ""
            port.text = ""
            binding.target = addTCPDialog.standardButton(DialogButtonBox.Ok)
        }

        GridLayout {
            width: addUDPDialog.availableWidth
            columns: 2

            Label {
                text: qsTr("Please enter the host name and port number used by your traffic data receiver.")
                      + " "
                      + qsTr("The host is typically an IPv4 address of the form '192.168.4.1', but can be any internet address.")
                      + " "
                      + qsTr("The port is a number between 0 and 65535.")
                wrapMode: Text.WordWrap
                Layout.columnSpan: 2
                Layout.fillWidth: true
            }

            Label {
                text: qsTr("Host")
            }
            MyTextField {
                id: host
                focus: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
            }

            Label {
                text: qsTr("Port")
            }
            MyTextField {
                id: port
                focus: true
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignBaseline
                inputMethodHints: Qt.ImhDigitsOnly
                validator: IntValidator {
                    bottom: 0
                    top: 65535
                }
                onAccepted: addTCPDialog.accepted()
            }
        }

        Binding {
            id: binding
            property: "enabled"
            value: (host.text !== "") && port.acceptableInput
        }

        onAccepted: {
            var resultString = TrafficDataProvider.addDataSource_TCP(host.text, Number(port.text))
            if (resultString !== "")
            {
                ltd.text = resultString
                ltd.open()
                return
            }
            Global.toast.doToast( qsTr("Adding TCP Connection to %1, Port %2").arg(host.text).arg(port.text) )
            addTCPDialog.close()
        }

    }

    CenteringDialog {
        id: addSerialPortDialog

        modal: true
        title: qsTr("Add Serial Port Connection")
        standardButtons: Dialog.Cancel

        Component.onCompleted: ConnectionScanner_SerialPort.start()

        ColumnLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true

                text: qsTr("No Device Found")
                visible: (ConnectionScanner_SerialPort.connectionInfos.length === 0)
                wrapMode: Text.Wrap
            }

            DecoratedListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: contentHeight

                clip: true

                model: ConnectionScanner_SerialPort.connectionInfos

                delegate: WordWrappingItemDelegate {
                    width: addSerialPortDialog.availableWidth

                    enabled: model.modelData.canConnect
                    icon.source: model.modelData.icon
                    text: model.modelData.description

                    onClicked: {
                        var resultString = TrafficDataProvider.addDataSource(model.modelData)
                        if (resultString !== "")
                        {
                            ltd.text = resultString
                            ltd.open()
                            return
                        }
                        Global.toast.doToast( qsTr("Adding Connection: %1").arg(model.modelData.name) )
                        addSerialPortDialog.close()
                    }
                }
            }

            Button {
                Layout.alignment: Qt.AlignHCenter

                text: qsTr("Scan for Devices")

                icon.source: "/icons/material/ic_settings_ethernet.svg"
                onClicked: ConnectionScanner_SerialPort.start()
            }
        }

    }
}
