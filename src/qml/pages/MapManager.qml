/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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
    title: qsTr("Map Library")

    Component {
        id: sectionHeading

        NightAndDayLabel {
            x: Qt.application.font.pixelSize
            nightAndDayText: section
            font.pixelSize: Qt.application.font.pixelSize*1.2
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
                    nFilesTotal = mapManager.aviationMaps.numberOfFilesTotal();
                    mapTypeString = qsTr("aviation maps")
                } else {  // swipe view shows base maps
                    nFilesTotal = mapManager.baseMaps.numberOfFilesTotal();
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
                    onClicked: {
                        if (!model.modelData.downloading && (!model.modelData.hasFile || model.modelData.updatable)) {
                            mobileAdaptor.vibrateBrief()
                            startFileDownload()
                        }
                    }
                }

                ToolButton {
                    id: downloadButton
                    icon.source: "/icons/material/ic_file_download.svg"
                    visible: !model.modelData.hasFile && !model.modelData.downloading
                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        startFileDownload()
                    }
                }

                ToolButton {
                    id: updateButton
                    icon.source: "/icons/material/ic_refresh.svg"
                    visible: model.modelData.updatable
                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        startFileDownload()
                    }
                }

                ToolButton {
                    id: cancelButton
                    icon.source: "/icons/material/ic_cancel.svg"
                    visible: model.modelData.downloading
                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        model.modelData.stopFileDownload()
                    }
                }

                ToolButton {
                    id: removeButton
                    icon.source: "/icons/material/ic_more_horiz.svg"

                    visible: model.modelData.hasFile & !model.modelData.downloading
                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        removeMenu.popup()
                    }

                    AutoSizingMenu {
                        id: removeMenu

                        Action {
                            id: infoAction

                            text: qsTr("Map Info")

                            onTriggered: {
                                mobileAdaptor.vibrateBrief()
                                infoDialog.title = qsTr("Map Info: ") + model.modelData.objectName
                                infoDialog.text = geoMapProvider.describeMapFile(model.modelData.fileName)
                                infoDialog.open()
                            }
                        }
                        Action {
                            id: removeAction

                            text: qsTr("Uninstall")

                            onTriggered: {
                                mobileAdaptor.vibrateBrief()
                                model.modelData.deleteFile()
                            }
                        }
                    }

                } // ToolButton
            }

            Connections {
                target: model.modelData
                function onError (message) {
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

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.leftMargin: drawer.dragMargin

            icon.source: "/icons/material/ic_arrow_back.svg"
            onClicked: {
                mobileAdaptor.vibrateBrief()
                stackView.pop()
            }
        } // ToolButton

        NightAndDayLabel {
            anchors.left: backButton.right
            anchors.right: headerMenuToolButton.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top

            nightAndDayText: stackView.currentItem.title
            elide: Label.ElideRight
            font.bold: true
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            anchors.right: parent.right

            id: headerMenuToolButton
            icon.source: "/icons/material/ic_more_vert.svg"
            onClicked: {
                mobileAdaptor.vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu {
                id: headerMenuX

                MenuItem {
                    id: updateMenu

                    text: qsTr("Update list of maps")

                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false
                        mapManager.updateGeoMapList()
                    }
                }

                MenuItem {
                    id: downloadUpdatesMenu

                    text: qsTr("Download all updates…")
                    enabled: mapManager.geoMaps.updatable

                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false
                        mapManager.geoMaps.updateAll()
                    }
                }

            } // AutoSizingMenu
        } // ToolButton

    } // ToolBar


    TabBar {
        id: bar

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        currentIndex: sv.currentIndex
        TabButton {
            text: qsTr("Aviation Maps")
        }
        TabButton {
            text: qsTr("Base Maps")
        }
        Material.elevation: 3
    } // TabBar

    ColumnLayout {
        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        anchors.topMargin: Qt.application.font.pixelSize*0.2

        SwipeView{
            id: sv

            currentIndex: bar.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true

            ListView {
                clip: true
                model: mapManager.aviationMaps.downloadablesAsObjectList
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
                            text: librarian.getStringFromRessource(":text/aviationMapMissing.html")
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
                        mobileAdaptor.vibrateBrief()
                        mapManager.updateGeoMapList()
                    }
                }
            } // ListView

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                model: mapManager.baseMaps.downloadablesAsObjectList
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
                        mobileAdaptor.vibrateBrief()
                        mapManager.updateGeoMapList()
                    }
                }
            } // ListView

        } // SwipeView

    } // ColumnLayout


    Rectangle {
        id: noMapListWarning

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        color: "white"
        visible: !mapManager.downloadingGeoMapList && !mapManager.hasGeoMapList

        NightAndDayLabel {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Qt.application.font.pixelSize*2

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            nightAndDayText: qsTr("<h3>Sorry!</h3><p>The list of available maps has not yet been downloaded from the server. You can restart the download manually using the item 'Update' from the menu.  To find the menu, look for the symbol '&#8942;' at the top right corner of the screen.</p>")
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

    Rectangle {
        id: downloadIndicator

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        color: "white"
        visible: mapManager.downloadingGeoMapList

        NightAndDayLabel {
            id: downloadIndicatorLabel

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Qt.application.font.pixelSize*2

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            nightAndDayText: qsTr("<h3>Download in progress…</h3><p>Please stand by while we download the list of available maps from the server…</p>")
            onLinkActivated: Qt.openUrlExternally(link)
        } // downloadIndicatorLabel

        BusyIndicator {
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.top: downloadIndicatorLabel.bottom
            anchors.topMargin: 10
        }

        // The Connections and the SequentialAnimation here provide a fade-out animation for the downloadindicator.
        // Without this, the downaloadIndication would not be visible on very quick downloads, leaving the user
        // without any feedback if the download did actually take place.
        Connections {
            target: mapManager
            function onDownloadingGeoMapListChanged () {
                if (mapManager.downloadingGeoMapList) {
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
    } // downloadIndicator - Rectangle

    footer: Pane {
        width: parent.width

        Material.elevation: 3
        visible: !mapManager.geoMaps.downloading && mapManager.geoMaps.updatable

        ToolButton {
            id: downloadUpdatesActionButton
            anchors.centerIn: parent
            text: qsTr("Download all updates…")
            icon.source: "/icons/material/ic_file_download.svg"

            onClicked: {
                mobileAdaptor.vibrateBrief()
                mapManager.geoMaps.updateAll()
            }
        }
    } // Pane (footer)

    // Show error when list of maps cannot be downloaded
    Connections {
        target: mapManager
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
