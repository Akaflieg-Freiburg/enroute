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

import enroute 1.0

import "../dialogs"
import "../items"

Page {
    id: page
    title: qsTr("Flight Route Library")
    focus: true


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

            AutoSizingMenu{
                id: headerMenuX

                MenuItem {
                    text: qsTr("Info …")
                    onTriggered: {
                        global.mobileAdaptor().vibrateBrief()
                        infoDialog.open()
                    }

                } // ToolButton

                MenuItem {
                    text: qsTr("Import …")
                    enabled: Qt.platform.os !== "android"
                    visible: Qt.platform.os !== "android"
                    height: Qt.platform.os !== "android" ? undefined : 0

                    onTriggered: {
                        global.mobileAdaptor().vibrateBrief()
                        highlighted = false
                        global.mobileAdaptor().importContent()
                    }
                }

            }

        }

    }

    TextField {
        id: textInput

        anchors.right: parent.right
        anchors.rightMargin: Qt.application.font.pixelSize*2.0
        anchors.left: parent.left
        anchors.leftMargin: Qt.application.font.pixelSize*2.0

        placeholderText: qsTr("Filter Flight Route Names")
        font.pixelSize: Qt.application.font.pixelSize*1.5
    }

    Component {
        id: flightRouteDelegate

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            Layout.fillWidth: true
            height: iDel.heigt

            ItemDelegate {
                id: iDel
                Layout.fillWidth: true

                text: modelData
                icon.source: "/icons/material/ic_directions.svg"

                onClicked: {
                    global.mobileAdaptor().vibrateBrief()
                    finalFileName = modelData
                    if (global.navigator().flightRoute.size > 0)
                        overwriteDialog.open()
                    else
                        openFromLibrary()
                }

            }

            ToolButton {
                id: cptMenuButton

                icon.source: "/icons/material/ic_more_horiz.svg"

                onClicked: {
                    global.mobileAdaptor().vibrateBrief()
                    cptMenu.popup()
                }

                AutoSizingMenu {
                    id: cptMenu

                    AutoSizingMenu {
                        title: Qt.platform.os === "android" ? qsTr("Share …") : qsTr("Export …")

                        MenuItem {
                            text: qsTr("… to GeoJSON file")
                            onTriggered: {
                                cptMenu.close()
                                global.mobileAdaptor().vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = global.mobileAdaptor().exportContent(global.librarian().get(Librarian.Routes, modelData).toGeoJSON(), "application/geo+json", global.librarian().get(Librarian.Routes, modelData).suggestedFilename())
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
                                global.mobileAdaptor().vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = global.mobileAdaptor().exportContent(global.librarian().get(Librarian.Routes, modelData).toGpx(), "application/gpx+xml", global.librarian().get(Librarian.Routes, modelData).suggestedFilename())
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
                        title: qsTr("Open in other app …")

                        MenuItem {
                            text: qsTr("… in GeoJSON format")

                            onTriggered: {
                                global.mobileAdaptor().vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = global.mobileAdaptor().viewContent(global.librarian().get(Librarian.Routes, modelData).toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
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
                                global.mobileAdaptor().vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = global.mobileAdaptor().viewContent(global.librarian().get(Librarian.Routes, modelData).toGpx(), "application/gpx+xml", "FlightRoute-%1.gpx")
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
                        text: qsTr("Rename …")
                        onTriggered: {
                            global.mobileAdaptor().vibrateBrief()
                            finalFileName = modelData
                            renameName.text = ""
                            renameDialog.open()
                        }

                    } // renameAction

                    Action {
                        id: removeAction
                        text: qsTr("Remove …")
                        onTriggered: {
                            global.mobileAdaptor().vibrateBrief()
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

        clip: true

        model: global.librarian().entries(Librarian.Routes, textInput.displayText)
        delegate: flightRouteDelegate
        ScrollIndicator.vertical: ScrollIndicator {}
    }

    Label {
        anchors.fill: wpList
        anchors.topMargin: Qt.application.font.pixelSize*2

        visible: (wpList.count === 0)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        leftPadding: Qt.application.font.pixelSize*2
        rightPadding: Qt.application.font.pixelSize*2

        textFormat: Text.StyledText
        wrapMode: Text.Wrap
        text: (textInput.text === "")
              ? qsTr("<h3>Sorry!</h3><p>No flight routes available. To add a route here, chose 'Flight Route' from the main menu, edit a route and save it to the library.</p>")
              : qsTr("<h3>Sorry!</h3><p>No flight routes match your filter criteria.</p>")
    }


    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function openFromLibrary() {
        var errorString = global.navigator().flightRoute.loadFromGeoJSON(global.librarian().fullPath(Librarian.Routes, finalFileName))
        if (errorString !== "") {
            lbl.text = errorString
            fileError.open()
            return
        }
        stackView.push("FlightRouteEditor.qml")
    }

    function reloadFlightRouteList() {
        var cache = textInput.text
        textInput.text = textInput.text+"XXXXX"
        textInput.text = cache
    }

    Dialog {
        id: fileError

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (view.width-width)/2.0
        y: (view.height-height)/2.0

        modal: true
        title: qsTr("An error occurred…")
        standardButtons: Dialog.Ok

        ScrollView{
            id: sv
            anchors.fill: parent

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.vertical.policy: (height < lbl.implicitHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            ScrollBar.vertical.interactive: false

            clip: true

            // The Label that we really want to show is wrapped into an Item. This allows
            // to set implicitHeight, and thus compute the implicitHeight of the Dialog
            // without binding loops
            Item {
                implicitHeight: lbl.implicitHeight
                width: fileError.availableWidth

                Label {
                    id: lbl
                    width: fileError.availableWidth
                    textFormat: Text.StyledText
                    linkColor: Material.accent
                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                } // Label
            } // Item
        } // ScrollView

    }

    LongTextDialog {
        id: infoDialog
        standardButtons: Dialog.Ok

        title: qsTr("Flight Route Library")
        text: global.librarian().getStringFromRessource(":text/flightRouteLibraryInfo.html").arg(global.librarian().directory(Librarian.Routes))
    }

    Dialog {
        id: overwriteDialog

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Overwrite current flight route?")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Loading the route <strong>%1</strong> will overwrite the current route. Once overwritten, the current flight route cannot be restored.").arg(finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            global.mobileAdaptor().vibrateBrief()
            page.openFromLibrary()
        }
        onRejected: {
            global.mobileAdaptor().vibrateBrief()
            close()
        }

    }

    Dialog {
        id: removeDialog

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Remove from device?")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Once the flight route <strong>%1</strong> is removed, it cannot be restored.").arg(page.finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            global.mobileAdaptor().vibrateBrief()
            global.librarian().remove(Librarian.Routes, page.finalFileName)
            page.reloadFlightRouteList()
            toast.doToast(qsTr("Flight route removed from device"))
        }
        onRejected: {
            global.mobileAdaptor().vibrateBrief()
            close()
        }

    }

    Dialog {
        id: renameDialog

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Rename Flight Route")

        standardButtons: Dialog.Cancel
        modal: true
        focus: true

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)


        ColumnLayout {
            width: renameDialog.availableWidth

            Label {
                width: overwriteDialog.availableWidth

                text: qsTr("Enter new name for the route <strong>%1</strong>.").arg(finalFileName)
                color: Material.primary
                Layout.fillWidth: true
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

        } // ColumnLayout

        footer: DialogButtonBox {
            ToolButton {
                id: renameButton

                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                enabled: (renameName.text !== "") && !(global.librarian().exists(Librarian.Routes, renameName.text))
                text: qsTr("Rename")
            }
        }

        onAccepted: {
            global.mobileAdaptor().vibrateBrief()
            if ((renameName.text !== "") && !global.librarian().exists(Librarian.Routes, renameName.text)) {
                global.librarian().rename(Librarian.Routes, finalFileName, renameName.text)
                page.reloadFlightRouteList()
                close()
                toast.doToast(qsTr("Flight route renamed"))
            }
        }
        onRejected: {
            global.mobileAdaptor().vibrateBrief()
            close()
        }

    }

    Dialog {
        id: shareErrorDialog

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Error exporting data…")
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)

        Label {
            id: shareErrorDialogLabel
            width: shareErrorDialog.availableWidth
            onLinkActivated: Qt.openUrlExternally(link)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.Ok
        modal: true

    }

} // Page
