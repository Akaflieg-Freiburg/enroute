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
                // check how many files are already downloaded:
                var nFilesTotal;
                var mapTypeString;
                if (sv.currentIndex == 0) {  // swipe view shows aviation maps
                    nFilesTotal = global.dataManager().aviationMaps.numberOfFilesTotal();
                    mapTypeString = qsTr("aviation maps")
                } else {  // swipe view shows base maps
                    nFilesTotal = global.dataManager().baseMaps.numberOfFilesTotal();
                    mapTypeString = qsTr("base maps")
                }

                // if the user downloads more than 8 files, show them a dialog telling them that
                // the bandwidth is sponsored and they shouldn't over-consume.
                if (nFilesTotal > 7) {
                    dialogLoader.active = false;
                    dialogLoader.dialogArgs = {onAcceptedCallback: model.modelData.startFileDownload,
                        nFilesTotal: nFilesTotal,
                        mapTypeString: mapTypeString};
                    dialogLoader.source = "../dialogs/TooManyDownloadsDialog.qml";
                    dialogLoader.active = true;
                } else {
                    model.modelData.startFileDownload();
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

                ItemDelegate {
                    text: model.modelData.objectName + `<br><font color="#606060" size="2">${model.modelData.infoText}</font>`
                    icon.source: model.modelData.updatable ? "/icons/material/ic_new_releases.svg" : "/icons/material/ic_map.svg"
                    Layout.fillWidth: true
                    enabled: !model.modelData.hasFile
                    onClicked: {
                        if (!model.modelData.downloading && (!model.modelData.hasFile || model.modelData.updatable)) {
                            global.mobileAdaptor().vibrateBrief()

                            if (model.modelData.fileName.endsWith("mbtiles") && global.dataManager().baseMapsRaster.hasFile) {
                                uninstallRasterMapDialog.vectorMap = element
                                uninstallRasterMapDialog.open()
                            } else
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
                        model.modelData.startFileDownload()
                    }
                }

                ToolButton {
                    id: updateButton
                    icon.source: "/icons/material/ic_refresh.svg"
                    visible: model.modelData.updatable
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        model.modelData.startFileDownload()
                    }
                }

                ToolButton {
                    id: cancelButton
                    icon.source: "/icons/material/ic_cancel.svg"
                    visible: model.modelData.downloading
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        model.modelData.stopFileDownload()
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
                                infoDialog.text = global.dataManager().describeDataItem(model.modelData.fileName)
                                infoDialog.open()
                            }
                        }
                        Action {
                            id: removeAction

                            text: qsTr("Uninstall")

                            onTriggered: {
                                global.mobileAdaptor().vibrateBrief()
                                model.modelData.deleteFile()
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

    Component {
        id: terrainItem

        Item {
            id: element
            width: pg.width
            height: gridLayout.height

            function startFileDownload() {
                // check how many files are already downloaded:
                var nFilesTotal;
                var mapTypeString;
                if (sv.currentIndex == 0) {  // swipe view shows aviation maps
                    nFilesTotal = global.dataManager().aviationMaps.numberOfFilesTotal();
                    mapTypeString = qsTr("aviation maps")
                } else {  // swipe view shows base maps
                    nFilesTotal = global.dataManager().baseMaps.numberOfFilesTotal();
                    mapTypeString = qsTr("base maps")
                }

                // if the user downloads more than 8 files, show them a dialog telling them that
                // the bandwidth is sponsored and they shouldn't over-consume.
                if (nFilesTotal > 7) {
                    dialogLoader.active = false;
                    dialogLoader.dialogArgs = {onAcceptedCallback: model.modelData.startFileDownload,
                        nFilesTotal: nFilesTotal,
                        mapTypeString: mapTypeString};
                    dialogLoader.source = "../dialogs/TooManyDownloadsDialog.qml";
                    dialogLoader.active = true;
                } else {
                    model.modelData.startFileDownload();
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

                ItemDelegate {
                    text: model.modelData.objectName + `<br><font color="#606060" size="2">${model.modelData.infoText}</font>`
                    icon.source: model.modelData.updatable ? "/icons/material/ic_new_releases.svg" : "/icons/material/ic_terrain.svg"
                    Layout.fillWidth: true
                    enabled: !model.modelData.hasFile
                    onClicked: {
                        if (!model.modelData.downloading && (!model.modelData.hasFile || model.modelData.updatable)) {
                            global.mobileAdaptor().vibrateBrief()

                            if (model.modelData.fileName.endsWith("mbtiles") && global.dataManager().baseMapsRaster.hasFile) {
                                uninstallRasterMapDialog.vectorMap = element
                                uninstallRasterMapDialog.open()
                            } else
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
                        model.modelData.startFileDownload()
                    }
                }

                ToolButton {
                    id: updateButton
                    icon.source: "/icons/material/ic_refresh.svg"
                    visible: model.modelData.updatable
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        model.modelData.startFileDownload()
                    }
                }

                ToolButton {
                    id: cancelButton
                    icon.source: "/icons/material/ic_cancel.svg"
                    visible: model.modelData.downloading
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        model.modelData.stopFileDownload()
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
                                infoDialog.text = global.dataManager().describeDataItem(model.modelData.fileName)
                                infoDialog.open()
                            }
                        }
                        Action {
                            id: removeAction

                            text: qsTr("Uninstall")

                            onTriggered: {
                                global.mobileAdaptor().vibrateBrief()
                                model.modelData.deleteFile()
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

    Component {
        id: databaseItem

        Item {
            id: element
            width: pg.width
            height: gridLayout.height

            GridLayout {
                id: gridLayout

                columnSpacing: 0
                rowSpacing: 0

                anchors.right: parent.right
                anchors.rightMargin: 0
                anchors.left: parent.left
                anchors.leftMargin: 0
                columns: 6

                ItemDelegate {
                    text: model.modelData.objectName + `<br><font color="#606060" size="2">${model.modelData.infoText}</font>`
                    icon.source: model.modelData.updatable ? "/icons/material/ic_new_releases.svg" : "/icons/material/ic_library_books.svg"
                    Layout.fillWidth: true
                    enabled: !model.modelData.hasFile
                    onClicked: {
                        if (!model.modelData.downloading && (!model.modelData.hasFile || model.modelData.updatable)) {
                            global.mobileAdaptor().vibrateBrief()
                            model.modelData.startFileDownload();
                        }
                    }
                }

                ToolButton {
                    id: downloadButton
                    icon.source: "/icons/material/ic_file_download.svg"
                    visible: !model.modelData.hasFile && !model.modelData.downloading
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        model.modelData.startFileDownload()
                    }
                }

                ToolButton {
                    id: updateButton
                    icon.source: "/icons/material/ic_refresh.svg"
                    visible: model.modelData.updatable
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        model.modelData.startFileDownload()
                    }
                }

                ToolButton {
                    id: cancelButton
                    icon.source: "/icons/material/ic_cancel.svg"
                    visible: model.modelData.downloading
                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        model.modelData.stopFileDownload()
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
                                infoDialog.text = global.dataManager().describeDataItem(model.modelData.fileName)
                                infoDialog.open()
                            }
                        }
                        Action {
                            id: removeAction

                            text: qsTr("Uninstall")

                            onTriggered: {
                                global.mobileAdaptor().vibrateBrief()
                                model.modelData.deleteFile()
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
                enabled: global.dataManager().items.updatable

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
            text: qsTr("Aviation maps")
        }
        TabButton {
            text: qsTr("Base maps")
        }
        TabButton {
            text: qsTr("Terrain")
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
                clip: true
                model: global.dataManager().aviationMaps.downloadablesAsObjectList
                delegate: mapItem
                ScrollIndicator.vertical: ScrollIndicator {}

                section.property: "modelData.section"
                section.delegate: sectionHeading

                footer: ColumnLayout {
                    width: parent.width

                    Rectangle {
                        Layout.fillWidth: true
                        height: 1
                        color: "gray"
                    }

                    WordWrappingItemDelegate {
                        Layout.fillWidth: true
                        icon.source: "/icons/material/ic_info_outline.svg"
                        text: qsTr("How to request additional aviation maps…")
                        onClicked: ltd.open()

                        LongTextDialog {
                            id: ltd
                            standardButtons: Dialog.Ok

                            title: qsTr("Request additional aviation maps")
                            text: global.librarian().getStringFromRessource(":text/aviationMapMissing.html")
                        }

                    }

                    Item { // Spacer
                        height: 3
                    }

                }

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
                model: global.dataManager().baseMaps.downloadablesAsObjectList
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
                model: global.dataManager().terrainMaps.downloadablesAsObjectList
                delegate: terrainItem
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
                model: global.dataManager().databases.downloadablesAsObjectList
                delegate: databaseItem
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

        } // SwipeView

    }


    Rectangle {
        id: noMapListWarning

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        color: "white"
        visible: !global.dataManager().downloadingRemoteItemList && !global.dataManager().hasRemoteItemList

        Label {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: view.font.pixelSize*2
            anchors.leftMargin: view.font.pixelSize*2
            anchors.rightMargin: view.font.pixelSize*2

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: qsTr("<h3>Sorry!</h3><p>The list of available maps has not yet been downloaded from the server. You can restart the download manually using button below.</p>")
        }
    }

    Rectangle {
        id: downloadIndicator

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        color: "white"
        visible: global.dataManager().downloadingRemoteItemList

        Label {
            id: downloadIndicatorLabel

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: view.font.pixelSize*2
            anchors.leftMargin: view.font.pixelSize*2
            anchors.rightMargin: view.font.pixelSize*2

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: qsTr("<h3>Download in progress…</h3><p>Please stand by while we download the list of available maps from the server…</p>")
        }

        BusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: downloadIndicatorLabel.bottom
            anchors.topMargin: 10
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
        visible: (!global.dataManager().downloadingRemoteItemList && !global.dataManager().hasRemoteItemList) || (!global.dataManager().items.downloading && global.dataManager().items.updatable)
        contentHeight: Math.max(downloadMapListActionButton.height, downloadUpdatesActionButton.height)

        ToolButton {
            id: downloadMapListActionButton
            anchors.centerIn: parent
            visible: !global.dataManager().downloadingRemoteItemList && !global.dataManager().hasRemoteItemList

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
            visible: !global.dataManager().items.downloading && global.dataManager().items.updatable

            text: qsTr("Update")
            icon.source: "/icons/material/ic_file_download.svg"

            onClicked: {
                global.mobileAdaptor().vibrateBrief()
                global.dataManager().items.updateAll()
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

    Dialog {
        id: uninstallRasterMapDialog

        property var vectorMap

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(view.width-view.font.pixelSize, 40*view.font.pixelSize)
        height: Math.min(view.height-view.font.pixelSize, implicitHeight)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 5.15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Uninstall Raster Maps")

        Label {
            width: uninstallRasterMapDialog.availableWidth
            text: qsTr("To avoid conflicts between raster and vector maps, all raster maps will be uninstalled before new vector maps are downloaded.")

            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true

        onAccepted: {
            global.mobileAdaptor().vibrateBrief()
            global.dataManager().baseMapsRaster.deleteAllFiles()
            vectorMap.startFileDownload()
            toast.doToast( qsTr("Raster maps uninstalled") )
        }

    } // importDialog

} // Page
