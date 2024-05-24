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
                text: qsTr("Add Bluetooth Device")
                icon.source: "/icons/material/ic_add_circle.svg"

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    Global.dialogLoader.active = false
                    Global.dialogLoader.setSource("../dialogs/AddBTDeviceDialog.qml", {})
                    Global.dialogLoader.active = true
                }
            }
        }
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
}
