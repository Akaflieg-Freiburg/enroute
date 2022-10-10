/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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
import "../dialogs"
import "../items"

Page {
    id: pg
    objectName: "DataManagerPage"

    title: qsTr("Map and Data Library")

    Component {
        id: sectionHeading

        Label {
            x: view.font.pixelSize
            text: section
            font.pixelSize: view.font.pixelSize*1.2
            font.bold: true
            color: Material.accent
        }
    }

    Component {
        id: mapItem

        Item {
            id: element
            width: pg.width
            height: gridLayout.height

            function startFileDownload() {
                // if the user downloads too many, show them a dialog telling them that
                // the bandwidth is sponsored and they shouldn't over-consume.
                var nFilesTotal = global.dataManager().items.files.length
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
                    text: model.modelData.objectName + `<br><font color="#606060" size="2">${model.modelData.infoText}</font>`
                    icon.source: {
                        if (model.modelData.updatable)
                            return "/icons/material/ic_new_releases.svg";
                        if (model.modelData.contentType === Downloadable_Abstract.TerrainMap)
                            return "/icons/material/ic_terrain.svg";
                        if (model.modelData.contentType === Downloadable_Abstract.Data)
                            return "/icons/material/ic_library_books.svg"
                        return "/icons/material/ic_map.svg";
                    }
                    Layout.fillWidth: true
                    enabled: !model.modelData.hasFile
                    onClicked: {
                        if (!model.modelData.downloading && (!model.modelData.hasFile || model.modelData.updatable)) {
                            global.mobileAdaptor().vibrateBrief()
                            startFileDownload()
                        }
                    }
                }

                ToolButton {
                    id: downloadButton
                    icon.source: "/icons/material/ic_file_download.svg"
                    visible: !model.modelData.hasFile && !model.modelData.downloading
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        startFileDownload()
                    }
                }

                ToolButton {
                    id: updateButton
                    icon.source: "/icons/material/ic_refresh.svg"
                    visible: (model.modelData.updateSize !== 0) && !model.modelData.downloading
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        model.modelData.update()
                    }
                }

                ToolButton {
                    id: cancelButton
                    icon.source: "/icons/material/ic_cancel.svg"
                    visible: model.modelData.downloading
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        model.modelData.stopDownload()
                    }
                }

                ToolButton {
                    id: removeButton
                    icon.source: "/icons/material/ic_more_horiz.svg"

                    visible: model.modelData.hasFile & !model.modelData.downloading
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        removeMenu.popup()
                    }

                    AutoSizingMenu {
                        id: removeMenu

                        Action {
                            id: infoAction

                            text: qsTr("Info")

                            onTriggered: {
                                global.mobileAdaptor().vibrateBrief()
                                infoDialog.title = model.modelData.objectName
                                infoDialog.text = model.modelData.description
                                infoDialog.open()
                            }
                        }
                        Action {
                            id: removeAction

                            text: qsTr("Uninstall")

                            onTriggered: {
                                global.mobileAdaptor().vibrateBrief()
                                model.modelData.deleteFiles()
                            }
                        }
                    }

                } // ToolButton
            }

            Connections {
                target: model.modelData
                function onError(objectName, message) {
                    dialogLoader.active = false
                    dialogLoader.title = qsTr("Download Error")
                    dialogLoader.text = qsTr("<p>Failed to download <strong>%1</strong>.</p><p>Reason: %2.</p>").arg(objectName).arg(message)
                    dialogLoader.source = "../dialogs/ErrorDialog.qml"
                    dialogLoader.active = true
                }
            }

        }

    }

    header: ToolBar {

        Material.foreground: "white"
        height: 60

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                global.mobileAdaptor().vibrateBrief()
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

            anchors.verticalCenter: parent.verticalCenter

            anchors.right: parent.right

            icon.source: "/icons/material/ic_more_vert.svg"
            icon.color: "white"

            onClicked: {
                global.mobileAdaptor().vibrateBrief()
                headerMenuX.popup()
            }

        }

        AutoSizingMenu {
            id: headerMenuX

            MenuItem {
                id: updateMenu

                text: qsTr("Update list of maps and data")

                onTriggered: {
                    global.mobileAdaptor().vibrateBrief()
                    highlighted = false
                    global.dataManager().updateRemoteDataItemList()
                }
            }

            MenuItem {
                id: downloadUpdatesMenu

                text: qsTr("Download all updates…")
                enabled: (global.dataManager().items.updateSize !== 0)

                onTriggered: {
                    global.mobileAdaptor().vibrateBrief()
                    highlighted = false
                    global.dataManager().items.updateAll()
                }
            }

        }

    }


    TabBar {
        id: bar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        currentIndex: sv.currentIndex
        TabButton {
            text: qsTr("Maps")
        }
        TabButton {
            text: qsTr("Data")
        }
        Material.elevation: 3
    }

    ColumnLayout {
        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        anchors.topMargin: view.font.pixelSize*0.2

        SwipeView{
            id: sv

            currentIndex: bar.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                model: global.dataManager().mapSets.downloadables
                delegate: mapItem
                ScrollIndicator.vertical: ScrollIndicator {}

                section.property: "modelData.section"
                section.delegate: sectionHeading

                // Refresh list of maps on overscroll
                property int refreshFlick: 0
                onFlickStarted: {
                    refreshFlick = atYBeginning
                }
                onFlickEnded: {
                    if ( atYBeginning && refreshFlick ) {
                        global.mobileAdaptor().vibrateBrief()
                        global.dataManager().updateRemoteDataItemList()
                    }
                }
            }

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                model: global.dataManager().databases.downloadables
                delegate: mapItem
                ScrollIndicator.vertical: ScrollIndicator {}

                section.property: "modelData.section"
                section.delegate: sectionHeading

                // Refresh list of maps on overscroll
                property int refreshFlick: 0
                onFlickStarted: {
                    refreshFlick = atYBeginning
                }
                onFlickEnded: {
                    if ( atYBeginning && refreshFlick ) {
                        global.mobileAdaptor().vibrateBrief()
                        global.dataManager().updateRemoteDataItemList()
                    }
                }
            }
        }

    }

    Label {
        id: noMapListWarning

        anchors.fill: parent
        anchors.bottomMargin: view.font.pixelSize
        anchors.leftMargin: view.font.pixelSize
        anchors.rightMargin: view.font.pixelSize
        anchors.topMargin: view.font.pixelSize

        background: Rectangle {color: "white"}
        visible: !global.dataManager().mapList.downloading && !global.dataManager().mapList.hasFile

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment : Text.AlignVCenter
        textFormat: Text.RichText
        wrapMode: Text.Wrap

        text: qsTr("<h3>Sorry!</h3><p>The list of available maps has not yet been downloaded from the server. You can restart the download manually using button below.</p>")
    }

    Rectangle {
        id: downloadIndicator

        anchors.fill: parent

        color: "white"
        visible: global.dataManager().mapList.downloading

        ColumnLayout {
            anchors.fill: parent

            Item {
                Layout.fillHeight: true
            }

            Label {
                id: downloadIndicatorLabel

                Layout.fillWidth: true

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment : Text.AlignVCenter
                textFormat: Text.RichText
                wrapMode: Text.Wrap

                leftPadding: view.font.pixelSize*2
                rightPadding: view.font.pixelSize*2

                text: qsTr("<h3>Download in progress…</h3><p>Please stand by while we download the list of available maps from the server…</p>")
            }

            BusyIndicator {
                Layout.fillWidth: true
            }

            Item {
                Layout.fillHeight: true
            }

        }

        // The Connections and the SequentialAnimation here provide a fade-out animation for the downloadindicator.
        // Without this, the downaloadIndication would not be visible on very quick downloads, leaving the user
        // without any feedback if the download did actually take place.
        Connections {
            target: global.dataManager()
            function ondownloadingRemoteItemListChanged () {
                if (global.dataManager().downloadingRemoteItemList) {
                    downloadIndicator.visible = true
                    downloadIndicator.opacity = 1.0
                } else
                    fadeOut.start()
            }
        }
        SequentialAnimation{
            id: fadeOut
            NumberAnimation { target: downloadIndicator; property: "opacity"; to:1.0; duration: 400 }
            NumberAnimation { target: downloadIndicator; property: "opacity"; to:0.0; duration: 400 }
            NumberAnimation { target: downloadIndicator; property: "visible"; to:1.0; duration: 20}
        }
    }


    footer: Pane {
        width: parent.width

        Material.elevation: 3
        visible: downloadMapListActionButton.visible || downloadUpdatesActionButton.visible
        contentHeight: Math.max(downloadMapListActionButton.height, downloadUpdatesActionButton.height)

        ToolButton {
            id: downloadMapListActionButton
            anchors.centerIn: parent
            visible: !global.dataManager().mapList.downloading && !global.dataManager().mapList.hasFile

            text: qsTr("Download list of maps…")
            icon.source: "/icons/material/ic_file_download.svg"

            onClicked: {
                global.mobileAdaptor().vibrateBrief()
                global.dataManager().updateRemoteDataItemList()
            }
        }

        ToolButton {
            id: downloadUpdatesActionButton
            anchors.centerIn: parent
            visible: (!global.dataManager().items.downloading) && (global.dataManager().mapsAndData.updateSize > 0)

            text: qsTr("Update")
            icon.source: "/icons/material/ic_file_download.svg"

            onClicked: {
                global.mobileAdaptor().vibrateBrief()
                global.dataManager().mapsAndData.update()
            }
        }
    }

    // Show error when list of maps cannot be downloaded
    Connections {
        target: global.dataManager()
        function onError (message) {
            dialogLoader.active = false
            dialogLoader.title = qsTr("Download Error")
            dialogLoader.text = qsTr("<p>Failed to download the list of aviation maps.</p><p>Reason: %1.</p>").arg(message)
            dialogLoader.source = "../dialogs/ErrorDialog.qml"
            dialogLoader.active = true
        }
    }

    LongTextDialog {
        id: infoDialog

        text: ""
        standardButtons: Dialog.Ok
    }

} // Page
