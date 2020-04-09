/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
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

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import "../items"

Page {
    id: pg
    title: qsTr("Download Maps")

    Component {
        id: sectionHeading

        Label {
            x: Qt.application.font.pixelSize
            text: section
            font.pixelSize: Qt.application.font.pixelSize*1.2
            font.bold: true
            color: Material.primary
        }
    }

    Component {
        id: mapItem

        Item {
            id: element
            width: parent.width
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
                    icon.source: model.modelData.updatable ? "/icons/material/ic_new_releases.svg" : "/icons/material/ic_map.svg"
                    icon.color: model.modelData.hasLocalFile ? Material.primary : "#9E9E9E"
                    Layout.fillWidth: true
                    onClicked: {
                        if (!model.modelData.downloading && (!model.modelData.hasLocalFile || model.modelData.updatable)) {
                            MobileAdaptor.vibrateBrief()
                            model.modelData.startFileDownload()
                        }
                    }
                }

                ToolButton {
                    id: downloadButton
                    icon.source: "/icons/material/ic_file_download.svg"
                    visible: !model.modelData.hasLocalFile && !model.modelData.downloading
                    onClicked: {
                        MobileAdaptor.vibrateBrief()
                        model.modelData.startFileDownload()
                    }
                }

                ToolButton {
                    id: updateButton
                    icon.source: "/icons/material/ic_refresh.svg"
                    visible: model.modelData.updatable
                    onClicked: {
                        MobileAdaptor.vibrateBrief()
                        model.modelData.startFileDownload()
                    }
                }

                ToolButton {
                    id: cancelButton
                    icon.source: "/icons/material/ic_cancel.svg"
                    visible: model.modelData.downloading
                    onClicked: {
                        MobileAdaptor.vibrateBrief()
                        model.modelData.stopFileDownload()
                    }
                }

                ToolButton {
                    id: removeButton
                    icon.source: "/icons/material/ic_more_horiz.svg"

                    visible: model.modelData.hasLocalFile & !model.modelData.downloading
                    onClicked: {
                        MobileAdaptor.vibrateBrief()
                        removeMenu.popup()
                    }

                    AutoSizingMenu {
                        id: removeMenu

                        Action {
                            id: updateAction

                            text: qsTr("Remove from device")
                            icon.source: "/icons/material/ic_delete.svg"

                            onTriggered: {
                                MobileAdaptor.vibrateBrief()
                                model.modelData.deleteLocalFile()
                            }
                        }
                    }

                }
            }

            Connections {
                target: model.modelData
                onError: {
                    dialogLoader.active = false
                    dialogLoader.title = qsTr("Download Error")
                    dialogLoader.text = qsTr(`<p>Failed to download <strong>${objectName}</strong>.</p><p>Reason: ${message}.</p>`)
                    dialogLoader.source = "../dialogs/ErrorDialog.qml"
                    dialogLoader.active = true
                }
            }

        }

    }

    header:  TabBar {
        id: bar
        width: parent.width
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
        anchors.fill: parent
        anchors.topMargin: Qt.application.font.pixelSize*0.2

        SwipeView{
            id: sv

            currentIndex: bar.currentIndex
            Layout.fillWidth: true
            Layout.fillHeight: true
            //            visible: !mapManager.downloadingGeoMapList && mapManager.hasGeoMapList

            ListView {
                clip: true
                model: mapManager.aviationMapsAsObjectList
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
                        MobileAdaptor.vibrateBrief()
                        mapManager.updateGeoMapList()
                    }
                }
            } // ListView

            ListView {
                Layout.fillHeight: true
                Layout.fillWidth: true
                clip: true
                model: mapManager.baseMapsAsObjectList
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
                        MobileAdaptor.vibrateBrief()
                        mapManager.updateGeoMapList()
                    }
                }
            } // ListView

        } // SwipeView

    } // ColumnLayout


    Rectangle {
        id: noMapListWarning

        anchors.fill: parent
        color: "white"
        visible: !mapManager.downloadingGeoMapList && !mapManager.hasGeoMapList

        Label {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Qt.application.font.pixelSize*2

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: qsTr("<h3>Sorry!</h3><p>The list of available maps has not yet been downloaded from the server. You can restart the download manually using the item 'Update' from the menu.  To find the menu, look for the symbol '&#8942;' at the top right corner of the screen.</p>")
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }

    Rectangle {
        id: downloadIndicator

        anchors.fill: parent
        color: "white"
        visible: mapManager.downloadingGeoMapList

        Label {
            id: downloadIndicatorLabel

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Qt.application.font.pixelSize*2

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: qsTr("<h3>Download in progress…</h3><p>Please stand by while we download the list of available maps from the server…</p>")
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
            onDownloadingGeoMapListChanged: {
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
        Label{
            id: lbl
            width: parent.width
            height: implicitHeight
            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: qsTr("<p>Aeronautical data is kindly provided by the <a href=\"https://www.openaip.net\">openAIP</a> and <a href=\"https://www.openflightmaps.org\">open flightmaps</a> projects. Base maps are kindly provided by <a href=\"https://openmaptiles.org\">OpenMapTiles</a>.</p>")
            onLinkActivated: Qt.openUrlExternally(link)
        }
        Material.elevation: 3
    } // Label

    // Add ToolButton to central application header when this page is shown
    Component.onCompleted: {
        headerMenuToolButton.visible = true
        headerMenu.insertAction(0, downloadUpdatesAction)
        headerMenu.insertAction(0, updateAction)
        headerMenu.insertAction(0, stressTestAction)
    }
    Component.onDestruction: {
        headerMenuToolButton.visible = false
        headerMenu.removeAction(downloadUpdatesAction)
        headerMenu.removeAction(updateAction)
        headerMenu.removeAction(stressTestAction)
    }

    // Show error when list of maps cannot be downloaded
    Connections {
        target: mapManager
        onError: {
            dialogLoader.active = false
            dialogLoader.title = qsTr("Download Error")
            dialogLoader.text = qsTr(`<p>Failed to download the list of aviation maps.</p><p>Reason: ${message}.</p>`)
            dialogLoader.source = "../dialogs/ErrorDialog.qml"
            dialogLoader.active = true
        }
    }

    Action {
        id: updateAction

        text: qsTr("Update list of maps")
        icon.source: "/icons/material/ic_refresh.svg"

        onTriggered: {
            MobileAdaptor.vibrateBrief()
            mapManager.updateGeoMapList()
        }
    }

    Action {
        id: stressTestAction

        text: qsTr("Stress test")
        icon.source: "/icons/material/ic_refresh.svg"

        onTriggered: {
            MobileAdaptor.vibrateBrief()
            mapManager.stressTest()
        }
    }

    Action {
        id: downloadUpdatesAction

        text: qsTr("Download all updates…")
        icon.source: "/icons/material/ic_file_download.svg"
        enabled: mapManager.geoMapUpdatesAvailable
        onTriggered: {
            MobileAdaptor.vibrateBrief()
            mapManager.updateGeoMaps()
        }
    }

} // Page
