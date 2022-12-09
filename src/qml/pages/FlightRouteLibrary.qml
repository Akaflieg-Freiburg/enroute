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

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import enroute 1.0

import "../dialogs"
import "../items"

Page {
    id: page
    title: qsTr("Flight Route Library")
    focus: true


    header: ToolBar {

        Material.foreground: "white"
        height: 60 + global.platformAdaptor().safeInsetTop
        leftPadding: global.platformAdaptor().safeInsetLeft
        rightPadding: global.platformAdaptor().safeInsetRight
        topPadding: global.platformAdaptor().safeInsetTop

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                global.platformAdaptor().vibrateBrief()
                stackView.pop()
            }
        }

        Label {
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
                global.platformAdaptor().vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu{
                id: headerMenuX

                MenuItem {
                    text: qsTr("Info…")
                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        infoDialog.open()
                    }

                } // ToolButton

                MenuItem {
                    text: qsTr("Import…")
                    enabled: Qt.platform.os !== "android"
                    visible: Qt.platform.os !== "android"
                    height: Qt.platform.os !== "android" ? undefined : 0

                    onTriggered: {
                        global.platformAdaptor().vibrateBrief()
                        highlighted = false
                        global.fileExchange().importContent()
                    }
                }

            }

        }

    }

    TextField {
        id: textInput

        anchors.right: parent.right
        anchors.rightMargin: view.font.pixelSize*2.0
        anchors.left: parent.left
        anchors.leftMargin: view.font.pixelSize*2.0
        leftPadding: global.platformAdaptor().safeInsetLeft
        rightPadding: global.platformAdaptor().safeInsetRight

        placeholderText: qsTr("Filter Flight Route Names")
        font.pixelSize: view.font.pixelSize*1.5
    }

    Component {
        id: flightRouteDelegate

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            Layout.fillWidth: true
            height: iDel.heigt

            SwipeToDeleteDelegate {
                id: iDel
                Layout.fillWidth: true

                text: modelData
                icon.source: "/icons/material/ic_directions.svg"

                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    finalFileName = modelData
                    if (global.navigator().flightRoute.size > 0)
                        overwriteDialog.open()
                    else
                        openFromLibrary()
                }

                swipe.onCompleted: {
                    global.platformAdaptor().vibrateBrief()
                    finalFileName = modelData
                    removeDialog.open()
                }

            }

            ToolButton {
                id: cptMenuButton

                icon.source: "/icons/material/ic_more_horiz.svg"

                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    cptMenu.popup()
                }

                AutoSizingMenu {
                    id: cptMenu

                    AutoSizingMenu {
                        title: Qt.platform.os === "android" ? qsTr("Share…") : qsTr("Export…")

                        MenuItem {
                            text: qsTr("… to GeoJSON file")
                            onTriggered: {
                                cptMenu.close()
                                global.platformAdaptor().vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = global.fileExchange().shareContent(global.librarian().get(Librarian.Routes, modelData).toGeoJSON(), "application/geo+json", global.librarian().get(Librarian.Routes, modelData).suggestedFilename())
                                if (errorString === "abort") {
                                    toast.doToast(qsTr("Aborted"))
                                    return
                                }
                                if (errorString !== "") {
                                    shareErrorDialogLabel.text = errorString
                                    shareErrorDialog.open()
                                    return
                                }
                                if (Qt.platform.os === "android")
                                    toast.doToast(qsTr("Flight route shared"))
                                else
                                    toast.doToast(qsTr("Flight route exported"))
                            }
                        }

                        MenuItem {
                            text: qsTr("… to GPX file")
                            onTriggered: {
                                cptMenu.close()
                                global.platformAdaptor().vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = global.fileExchange().shareContent(global.librarian().get(Librarian.Routes, modelData).toGpx(), "application/gpx+xml", global.librarian().get(Librarian.Routes, modelData).suggestedFilename())
                                if (errorString === "abort") {
                                    toast.doToast(qsTr("Aborted"))
                                    return
                                }
                                if (errorString !== "") {
                                    shareErrorDialogLabel.text = errorString
                                    shareErrorDialog.open()
                                    return
                                }
                                if (Qt.platform.os === "android")
                                    toast.doToast(qsTr("Flight route shared"))
                                else
                                    toast.doToast(qsTr("Flight route exported"))
                            }
                        }
                    }

                    AutoSizingMenu {
                        title: qsTr("Open in Other App…")

                        MenuItem {
                            text: qsTr("… in GeoJSON format")

                            onTriggered: {
                                global.platformAdaptor().vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = global.fileExchange().viewContent(global.librarian().get(Librarian.Routes, modelData).toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
                                if (errorString !== "") {
                                    shareErrorDialogLabel.text = errorString
                                    shareErrorDialog.open()
                                } else
                                    toast.doToast(qsTr("Flight route opened in other app"))
                            }
                        }

                        MenuItem {
                            text: qsTr("… in GPX format")

                            onTriggered: {
                                global.platformAdaptor().vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = global.fileExchange().viewContent(global.librarian().get(Librarian.Routes, modelData).toGpx(), "application/gpx+xml", "FlightRoute-%1.gpx")
                                if (errorString !== "") {
                                    shareErrorDialogLabel.text = errorString
                                    shareErrorDialog.open()
                                } else
                                    toast.doToast(qsTr("Flight route opened in other app"))
                            }
                        }

                    }

                    MenuSeparator { }

                    Action {
                        id: renameAction
                        text: qsTr("Rename…")
                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            finalFileName = modelData
                            renameName.text = modelData
                            renameDialog.open()
                        }

                    } // renameAction

                    Action {
                        id: removeAction
                        text: qsTr("Remove…")
                        onTriggered: {
                            global.platformAdaptor().vibrateBrief()
                            finalFileName = modelData
                            removeDialog.open()
                        }
                    } // removeAction
                } // AutoSizingMenu

            } // ToolButton

        }

    }

    ListView {
        id: wpList
        anchors.top: textInput.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        leftMargin: global.platformAdaptor().safeInsetLeft
        rightMargin: global.platformAdaptor().safeInsetRight
        bottomMargin: global.platformAdaptor().safeInsetBottom

        clip: true

        model: global.librarian().entries(Librarian.Routes, textInput.displayText)
        delegate: flightRouteDelegate
        ScrollIndicator.vertical: ScrollIndicator {}
    }

    Label {
        anchors.fill: wpList
        anchors.topMargin: view.font.pixelSize*2

        visible: (wpList.count === 0)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        leftPadding: view.font.pixelSize*2
        rightPadding: view.font.pixelSize*2

        textFormat: Text.StyledText
        wrapMode: Text.Wrap
        text: (textInput.text === "")
              ? qsTr("<h3>Sorry!</h3><p>No flight routes available. To add a route here, chose 'Flight Route' from the main menu, edit a route and save it to the library.</p>")
              : qsTr("<h3>Sorry!</h3><p>No flight routes match your filter criteria.</p>")
    }


    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function openFromLibrary() {
        var errorString = global.navigator().flightRoute.load(global.librarian().fullPath(Librarian.Routes, finalFileName))
        if (errorString !== "") {
            lbl.text = errorString
            fileError.open()
            return
        }
        toast.doToast( qsTr("Loading flight route <strong>%1</strong>").arg(finalFileName) )
        stackView.pop()
    }

    function reloadFlightRouteList() {
        var cache = textInput.text
        textInput.text = textInput.text+"XXXXX"
        textInput.text = cache
    }

    CenteringDialog {
        id: fileError

        modal: true
        title: qsTr("An Error Occurred…")
        standardButtons: Dialog.Ok

        ScrollView{
            anchors.fill: parent
            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            Label {
                id: lbl
                width: fileError.availableWidth
                textFormat: Text.StyledText
                linkColor: Material.accent
                wrapMode: Text.Wrap
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
    }

    LongTextDialog {
        id: infoDialog
        standardButtons: Dialog.Ok

        title: qsTr("Flight Route Library")
        text: global.librarian().getStringFromRessource(":text/flightRouteLibraryInfo.html").arg(global.librarian().directory(Librarian.Routes))
    }

    CenteringDialog {
        id: overwriteDialog

        title: qsTr("Overwrite Current Flight Route?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Loading the route <strong>%1</strong> will overwrite the current route. Once overwritten, the current flight route cannot be restored.").arg(finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            page.openFromLibrary()
        }
        onRejected: {
            global.platformAdaptor().vibrateBrief()
            close()
        }
    }

    CenteringDialog {
        id: removeDialog

        title: qsTr("Remove from Device?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            width: removeDialog.availableWidth

            text: qsTr("Once the flight route <strong>%1</strong> is removed, it cannot be restored.").arg(page.finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            global.librarian().remove(Librarian.Routes, page.finalFileName)
            page.reloadFlightRouteList()
            toast.doToast(qsTr("Flight route removed from device"))
        }
        onRejected: {
            global.platformAdaptor().vibrateBrief()
            page.reloadFlightRouteList()
            close()
        }
    }

    CenteringDialog {
        id: renameDialog

        title: qsTr("Rename Flight Route")
        standardButtons: Dialog.Cancel
        modal: true

        ColumnLayout {
            width: renameDialog.availableWidth

            Label {
                Layout.fillWidth: true

                text: qsTr("Enter new name for the route <strong>%1</strong>.").arg(finalFileName)
                color: Material.primary
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }

            TextField {
                id: renameName

                Layout.fillWidth: true
                focus: true

                placeholderText: qsTr("New Flight Route Name")

                onAccepted: renameDialog.onAccepted()
            }

        }

        footer: DialogButtonBox {
            ToolButton {
                id: renameButton

                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                enabled: (renameName.text !== "") && !(global.librarian().exists(Librarian.Routes, renameName.text))
                text: qsTr("Rename")
            }
        }

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            if ((renameName.text !== "") && !global.librarian().exists(Librarian.Routes, renameName.text)) {
                global.librarian().rename(Librarian.Routes, finalFileName, renameName.text)
                page.reloadFlightRouteList()
                close()
                toast.doToast(qsTr("Flight route renamed"))
            }
        }
        onRejected: {
            global.platformAdaptor().vibrateBrief()
            close()
        }
    }

    CenteringDialog {
        id: shareErrorDialog

        title: qsTr("Error Exporting Data…")
        standardButtons: Dialog.Ok
        modal: true

        Component.onCompleted: open()

        Label {
            id: shareErrorDialogLabel
            width: shareErrorDialog.availableWidth
            onLinkActivated: Qt.openUrlExternally(link)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }
    }

} // Page
