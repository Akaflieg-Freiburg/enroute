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
            text: element.model.modelData.objectName + `<br><font color="#606060" size="2">${element.model.modelData.infoText}</font>`
            icon.source: {
                if (element.model.modelData.updatable)
                    return "/icons/material/ic_new_releases.svg";
                if (element.model.modelData.contentType === Downloadable_Abstract.TerrainMap)
                    return "/icons/material/ic_terrain.svg";
                if (element.model.modelData.contentType === Downloadable_Abstract.Data)
                    return "/icons/material/ic_library_books.svg"
                return "/icons/material/ic_map.svg";
            }
            Layout.fillWidth: true
            enabled: !element.model.modelData.hasFile
            onClicked: {
                if (!element.model.modelData.downloading
                        && (!element.model.modelData.hasFile || element.model.modelData.updatable)) {
                    PlatformAdaptor.vibrateBrief()
                    element.startFileDownload()
                }
            }
        }

        ToolButton {
            id: downloadButton
            icon.source: "/icons/material/ic_file_download.svg"
            visible: !element.model.modelData.hasFile && !element.model.modelData.downloading
            onClicked: {
                PlatformAdaptor.vibrateBrief()
                element.startFileDownload()
            }
        }

        ToolButton {
            id: updateButton
            icon.source: "/icons/material/ic_refresh.svg"
            visible: (!element.model.modelData.updateSize.isNull()) && !element.model.modelData.downloading
            onClicked: {
                PlatformAdaptor.vibrateBrief()
                element.model.modelData.update()
            }
        }

        ToolButton {
            id: cancelButton
            icon.source: "/icons/material/ic_cancel.svg"
            visible: element.model.modelData.downloading
            onClicked: {
                PlatformAdaptor.vibrateBrief()
                element.model.modelData.stopDownload()
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
                    id: renameAction

                    text: qsTr("Rename")
                    enabled: false // (element.model.modelData.contentType === Downloadable_Abstract.VAC)

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
