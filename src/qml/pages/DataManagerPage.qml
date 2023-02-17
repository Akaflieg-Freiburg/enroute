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
import QtQuick.Controls.Material
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import enroute 1.0
import "../dialogs"
import "../items"

Page {
    id: pg
    objectName: "DataManagerPage"

    required property var dialogLoader
    required property var stackView

    title: qsTr("Map and Data Library")

    Component {
        id: sectionHeading

        Control {
            required property string section

            height: lbl.height

            Label {
                id: lbl

                x: font.pixelSize
                text: parent.section
                font.pixelSize: parent.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }
        }
    }

    Component {
        id: mapItem

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
                        removeMenu.popup()
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

    header: ToolBar {

        Material.foreground: "white"
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
                pg.stackView.pop()
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: pg.stackView.currentItem.title
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
                PlatformAdaptor.vibrateBrief()
                headerMenuX.popup()
            }

        }

        AutoSizingMenu {
            id: headerMenuX

            MenuItem {
                id: updateMenu

                text: qsTr("Update list of maps and data")

                onTriggered: {
                    PlatformAdaptor.vibrateBrief()
                    highlighted = false
                    DataManager.updateRemoteDataItemList()
                }
            }

            MenuItem {
                id: downloadUpdatesMenu

                text: qsTr("Download all updates…")
                enabled: (!DataManager.items.updateSize.isNull())

                onTriggered: {
                    PlatformAdaptor.vibrateBrief()
                    highlighted = false
                    DataManager.items.update()
                }
            }

        }

    }


    TabBar {
        id: bar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right

        currentIndex: sv.currentIndex
        TabButton {
            text: qsTr("Maps")
        }
        TabButton {
            text: qsTr("Data")
        }
        Material.elevation: 3
    }


    SwipeView{
            id: sv

            currentIndex: bar.currentIndex
            anchors.top: bar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            anchors.bottomMargin: pg.footer.visible ? 0 : SafeInsets.bottom
            anchors.leftMargin: SafeInsets.left
            anchors.rightMargin: SafeInsets.right

            clip: true

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                model: DataManager.mapSets.downloadables
                delegate: MapSet {}
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
                        PlatformAdaptor.vibrateBrief()
                        DataManager.updateRemoteDataItemList()
                    }
                }
            }

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                model: DataManager.databases.downloadables
                delegate: MapSet {}
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
                        PlatformAdaptor.vibrateBrief()
                        DataManager.updateRemoteDataItemList()
                    }
                }
            }
        }


    Label {
        id: appUpdateRequiredWarning

        anchors.fill: parent

        leftPadding: font.pixelSize*2
        rightPadding: font.pixelSize*2

        background: Rectangle {color: "white"}
        visible: DataManager.appUpdateRequired

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment : Text.AlignVCenter
        textFormat: Text.RichText
        wrapMode: Text.Wrap

        text: qsTr("<h3>Update required!</h3>")
              + Librarian.getStringFromRessource("appUpdateRequired")
    }

    Label {
        id: noMapListWarning

        anchors.fill: parent
        anchors.bottomMargin: font.pixelSize
        anchors.leftMargin: font.pixelSize
        anchors.rightMargin: font.pixelSize
        anchors.topMargin: font.pixelSize

        background: Rectangle {color: "white"}
        visible: !DataManager.mapList.downloading && !DataManager.mapList.hasFile

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
        visible: DataManager.mapList.downloading

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

                leftPadding: font.pixelSize*2
                rightPadding: font.pixelSize*2

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
            target: DataManager
            function ondownloadingRemoteItemListChanged () {
                if (DataManager.mapList.downloading) {
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
        visible: (!DataManager.mapList.downloading && !DataManager.mapList.hasFile) || ((!DataManager.items.downloading) && (DataManager.mapsAndData.updateSize > 0))
        contentHeight: Math.max(downloadMapListActionButton.height, downloadUpdatesActionButton.height)
        bottomPadding: SafeInsets.bottom

        ToolButton {
            id: downloadMapListActionButton
            anchors.centerIn: parent
            visible: !DataManager.mapList.downloading && !DataManager.mapList.hasFile

            text: qsTr("Download list of maps…")
            icon.source: "/icons/material/ic_file_download.svg"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                DataManager.updateRemoteDataItemList()
            }
        }

        ToolButton {
            id: downloadUpdatesActionButton
            anchors.centerIn: parent
            visible: (!DataManager.items.downloading) && (DataManager.mapsAndData.updateSize > 0)

            text: qsTr("Update")
            icon.source: "/icons/material/ic_file_download.svg"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                DataManager.mapsAndData.update()
            }
        }
    }

    // Show error when list of maps cannot be downloaded
    Connections {
        target: DataManager
        function onError (message) {
            pg.dialogLoader.active = false
            pg.dialogLoader.title = qsTr("Download Error")
            pg.dialogLoader.text = qsTr("<p>Failed to download the list of aviation maps.</p><p>Reason: %1.</p>").arg(message)
            pg.dialogLoader.source = "../dialogs/ErrorDialog.qml"
            pg.dialogLoader.active = true
        }
    }

} // Page

