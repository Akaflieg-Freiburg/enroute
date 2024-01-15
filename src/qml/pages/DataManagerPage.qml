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
import QtQuick.Dialogs
import QtQuick.Layouts

import akaflieg_freiburg.enroute
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
            }
        }
    }

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
                headerMenuX.open()
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

                MenuSeparator { }

                MenuItem {
                    id: menuImport

                    text: qsTr("Import…")

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        importFileDialog.open()
                    }

                    FileDialog {
                        id: importFileDialog

                        acceptLabel: qsTr("Import")
                        rejectLabel: qsTr("Cancel")

                        fileMode: FileDialog.OpenFile

                        // Setting a non-trivial name filter on Android means we cannot select any
                        // files at all.
                        nameFilters: Qt.platform.os === "android" ? undefined : [qsTr("OpenAir Airspace Data (*.txt)"),
                                                                                 qsTr("Raster and Vector Maps (*.mbtiles)"),
                                                                                 qsTr("Trip Kits (*.zip)"),
                                                                                 qsTr("Visual Approach Charts (*.tif *.tiff)")]

                        onAccepted: {
                            PlatformAdaptor.vibrateBrief()
                            close()
                            FileExchange.processFileOpenRequest(importFileDialog.selectedFile)
                        }
                        onRejected: {
                            PlatformAdaptor.vibrateBrief()
                            close()
                        }
                    }
                }

                MenuSeparator { }

                MenuItem {
                    text: qsTr("Clear VAC library…")
                    enabled: DataManager.VAC.downloadables.length > 0

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        clearVACDialog.open()
                    }

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
            text: "VAC"
        }
        TabButton {
            text: qsTr("Data")
        }
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

        DecoratedListView {
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

        DecoratedListView {
            Layout.fillHeight: true
            Layout.fillWidth: true
            clip: true
            model: DataManager.VAC.downloadables
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

            Label {
                anchors.fill: parent
                anchors.bottomMargin: font.pixelSize
                anchors.leftMargin: font.pixelSize
                anchors.rightMargin: font.pixelSize
                anchors.topMargin: font.pixelSize

                background: Rectangle {color: "white"}
                visible: !DataManager.VAC.hasFile

                horizontalAlignment: Text.AlignHCenter
                verticalAlignment : Text.AlignVCenter
                textFormat: Text.RichText
                wrapMode: Text.Wrap

                text: "<p>" + qsTr("There are no approach charts installed. The <a href='x'>manual</a> explains how to install and use them.") + "</p>"
                onLinkActivated: openManual("03-tutorialAdvanced/04-vac.html")

            }
        }

        DecoratedListView {
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
            target: DataManager.mapList
            function onDownloadingChanged () {
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


    footer: Footer {
        width: parent.width

        visible: (!DataManager.mapList.downloading && !DataManager.mapList.hasFile) || ((!DataManager.items.downloading) && !DataManager.mapsAndData.updateSize.isNull())
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
            visible: !DataManager.items.downloading && !DataManager.mapsAndData.updateSize.isNull()

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
            pg.dialogLoader.source = "dialogs/ErrorDialog.qml"
            pg.dialogLoader.active = true
        }
    }

    LongTextDialog {
        id: clearVACDialog

        title: qsTr("Clear approach chart Library?")
        standardButtons: Dialog.No | Dialog.Yes

        text: qsTr("Once cleared, the approach charts cannot be restored.")

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            DataManager.VAC.deleteFiles()
            toast.doToast(qsTr("Approach chart library cleared"))
        }
    }

} // Page

