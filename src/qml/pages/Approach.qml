/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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
import "../dialogs"
import "../items"

Page {
    id: pg

    required property var dialogLoader
    required property var stackView

    title: qsTr("Approach Charts")

    Component {
        id: approachChartItem

        Item {
            id: element

            width: parent ? parent.width : undefined
            height: gridLayout.height

            required property var model

            function startFileDownload() {
                // if the user downloads too many, show them a dialog telling them that
                // the bandwidth is sponsored and they shouldn't over-consume.
                var nFilesTotal = DataManager.items.files.length
                if (nFilesTotal > 15) {
                    dialogLoader.active = false;
                    dialogLoader.dialogArgs = {onAcceptedCallback: model.modelData.startDownload};
                    dialogLoader.source = "../dialogs/TooManyDownloadsDialog.qml";
                    dialogLoader.active = true;
                } else {
                    model.modelData.startDownload();
                }
            }

            GridLayout {
                id: gridLayout

                columnSpacing: 0
                rowSpacing: 0

                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0
                columns: 6

                WordWrappingItemDelegate {
                    text: element.model.modelData.objectName
                    icon.source: "/icons/material/ic_map.svg"
                    Layout.fillWidth: true
                    enabled: element.model.modelData.hasFile
                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        GeoMapProvider.approachChart = element.model.modelData.fileName
                        stackView.pop()
                    }
                }

                ToolButton {
                    id: removeButton
                    icon.source: "/icons/material/ic_more_horiz.svg"

                    visible: element.model.modelData.hasFile & !element.model.modelData.downloading
                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        removeMenu.open()
                    }

                    AutoSizingMenu {
                        id: removeMenu

                        Action {
                            id: infoAction

                            text: qsTr("Info")

                            onTriggered: {
                                PlatformAdaptor.vibrateBrief()
                                dialogLoader.active = false
                                dialogLoader.title = element.model.modelData.objectName
                                dialogLoader.text = element.model.modelData.description
                                dialogLoader.source = "../dialogs/ErrorDialog.qml"
                                dialogLoader.active = true
                            }
                        }
                        Action {
                            id: removeAction

                            text: qsTr("Uninstall")

                            onTriggered: {
                                PlatformAdaptor.vibrateBrief()
                                element.model.modelData.deleteFiles()
                            }
                        }
                    }

                } // ToolButton
            }

            Connections {
                target: element.model.modelData
                function onError(objectName, message) {
                    dialogLoader.active = false
                    dialogLoader.title = qsTr("Download Error")
                    dialogLoader.text = qsTr("<p>Failed to download <strong>%1</strong>.</p><p>Reason: %2.</p>").arg(objectName).arg(message)
                    dialogLoader.source = "../dialogs/ErrorDialog.qml"
                    dialogLoader.active = true
                }
            }

            Loader {
                id: dialogLoader

                property string title
                property string text
                property var dialogArgs: undefined

                onLoaded: {
                    item.modal = true
                    if (dialogArgs && item.hasOwnProperty('dialogArgs')) {
                        item.dialogArgs = dialogArgs
                    }
                    item.open()
                }

            }


        }

    }

    header: StandardHeader {}

    DecoratedListView {
        anchors.fill: parent

        clip: true
        model: DataManager.approachCharts.downloadables
        delegate: approachChartItem
        ScrollIndicator.vertical: ScrollIndicator {}

        section.property: "modelData.section"

    }

    Label {
        id: noMapListWarning

        anchors.fill: parent
        anchors.bottomMargin: font.pixelSize
        anchors.leftMargin: font.pixelSize
        anchors.rightMargin: font.pixelSize
        anchors.topMargin: font.pixelSize

        background: Rectangle {color: "white"}
        visible: !DataManager.approachCharts.hasFile

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment : Text.AlignVCenter
        textFormat: Text.RichText
        wrapMode: Text.Wrap

        text: qsTr("<h3>Sorry!</h3><p>There are no approach charts available. Read the manual to learn how to install them.</p>")
    }

} // Page

