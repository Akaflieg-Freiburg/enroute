/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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
    id: page
    title: qsTr("Flight Route Library")
    focus: true

    header: ToolBar {

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.leftMargin: drawer.dragMargin

            icon.source: "/icons/material/ic_arrow_back.svg"
            onClicked: {
                mobileAdaptor.vibrateBrief()
                if (stackView.depth > 1) {
                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        } // ToolButton

        Label {
            anchors.left: backButton.right
            anchors.right: headerMenuToolButton.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.bold: true
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            icon.source: "/icons/material/ic_more_vert.svg"
            onClicked: {
                mobileAdaptor.vibrateBrief()
                headerMenuX.popup()
            }

            AutoSizingMenu{
                id: headerMenuX

                MenuItem {
                    text: qsTr("Info …")
                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        infoDialog.open()
                    }

                } // ToolButton

                MenuItem {
                    text: qsTr("Import …")
                    enabled: Qt.platform.os !== "android"

                    onTriggered: {
                        mobileAdaptor.vibrateBrief()
                        highlighted = false
                        mobileAdaptor.importContent()
                    }
                }

            }

        } // ToolButton

    } // ToolBar

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
                icon.color: Material.primary

            }

            ToolButton {
                id: cptMenuButton

                icon.source: "/icons/material/ic_more_horiz.svg"

                onClicked: {
                    mobileAdaptor.vibrateBrief()
                    cptMenu.popup()
                }

                AutoSizingMenu {
                    id: cptMenu

                    Action {
                        id: openAction
                        text: qsTr("Open …")
                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
                            finalFileName = modelData
                            if (flightRoute.routeObjects.length > 0)
                                overwriteDialog.open()
                            else
                                openFromLibrary()
                        }

                    } // openAction

                    MenuSeparator { }

                    AutoSizingMenu {
                        title: Qt.platform.os === "android" ? qsTr("Share …") : qsTr("Export …")

                        MenuItem {
                            text: qsTr("… to GeoJSON file")
                            onTriggered: {
                                cptMenu.close()
                                mobileAdaptor.vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = mobileAdaptor.exportContent(librarian.flightRouteGet(modelData).toGeoJSON(), "application/geo+json", librarian.flightRouteGet(modelData).suggestedFilename())
                                if (errorString !== "") {
                                    shareErrorDialogLabel.text = errorString
                                    shareErrorDialog.open()
                                }

                            }
                        }

                        MenuItem {
                            text: qsTr("… to GPX file")
                            onTriggered: {
                                cptMenu.close()
                                mobileAdaptor.vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = mobileAdaptor.exportContent(librarian.flightRouteGet(modelData).toGpx(), "application/gpx+xml", librarian.flightRouteGet(modelData).suggestedFilename())
                                if (errorString !== "") {
                                    shareErrorDialogLabel.text = errorString
                                    shareErrorDialog.open()
                                }
                            }
                        }
                    }

                    AutoSizingMenu {
                        title: qsTr("Open in other app …")

                        MenuItem {
                            text: qsTr("… in GeoJSON format")

                            onTriggered: {
                                mobileAdaptor.vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = mobileAdaptor.viewContent(librarian.flightRouteGet(modelData).toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
                                if (errorString !== "") {
                                    shareErrorDialogLabel.text = errorString
                                    shareErrorDialog.open()
                                }

                            }
                        }

                        MenuItem {
                            text: qsTr("… in GPX format")

                            onTriggered: {
                                mobileAdaptor.vibrateBrief()
                                highlighted = false
                                parent.highlighted = false

                                var errorString = mobileAdaptor.viewContent(librarian.flightRouteGet(modelData).toGpx(), "application/gpx+xml", "FlightRoute-%1.gpx")
                                if (errorString !== "") {
                                    shareErrorDialogLabel.text = errorString
                                    shareErrorDialog.open()
                                }

                            }
                        }

                    }

                    MenuSeparator { }

                    Action {
                        id: renameAction
                        text: qsTr("Rename …")
                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
                            finalFileName = modelData
                            renameName.text = ""
                            renameDialog.open()
                        }

                    } // renameAction

                    Action {
                        id: removeAction
                        text: qsTr("Remove …")
                        onTriggered: {
                            mobileAdaptor.vibrateBrief()
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

        model: librarian.flightRoutes(textInput.displayText)
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

        textFormat: Text.RichText
        wrapMode: Text.Wrap
        text: (textInput.text === "")
              ? qsTr("<h3>Sorry!</h3><p>No flight routes available. To add a route here, chose 'Flight Route' from the main menu, edit a route and save it to the library.</p>")
              : qsTr("<h3>Sorry!</h3><p>No flight routes match your filter criteria.</p>")
        onLinkActivated: Qt.openUrlExternally(link)
    }


    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function openFromLibrary() {
        var errorString = flightRoute.loadFromGeoJSON(librarian.flightRouteFullPath(finalFileName))
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

        anchors.centerIn: parent
        parent: Overlay.overlay

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
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                } // Label
            } // Item
        } // ScrollView

    }

    LongTextDialog {
        id: infoDialog
        standardButtons: Dialog.Ok
        anchors.centerIn: parent

        title: qsTr("Flight Route Library")
        text: librarian.getStringFromRessource(":text/flightRouteLibraryInfo.html").arg(librarian.flightRouteDirectory())
    }

    Dialog {
        id: overwriteDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Overwrite current flight route?")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Loading the route <strong>%1</strong> will overwrite the current route. Once overwritten, the current flight route cannot be restored.").arg(finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            mobileAdaptor.vibrateBrief()
            page.openFromLibrary()
        }
        onRejected: {
            mobileAdaptor.vibrateBrief()
            close()
        }

    }

    Dialog {
        id: removeDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Remove from device?")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Once the flight route <strong>%1</strong> is removed, it cannot be restored.").arg(page.finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            mobileAdaptor.vibrateBrief()
            librarian.flightRouteRemove(page.finalFileName)
            page.reloadFlightRouteList()
        }
        onRejected: {
            mobileAdaptor.vibrateBrief()
            close()
        }

    }

    Dialog {
        id: renameDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

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
                textFormat: Text.RichText
            }

            TextField {
                id: renameName

                Layout.fillWidth: true
                focus: true

                placeholderText: "New Flight Route Name"

                onAccepted: renameDialog.onAccepted()
            }

        } // ColumnLayout

        footer: DialogButtonBox {
            ToolButton {
                id: renameButton

                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                enabled: (renameName.displayText !== "") && !librarian.flightRouteExists(renameName.displayText)
                text: qsTr("Rename")
            }
        }

        onAccepted: {
            mobileAdaptor.vibrateBrief()
            if ((renameName.text !== "") && !librarian.flightRouteExists(renameName.text)) {
                librarian.flightRouteRename(finalFileName, renameName.text)
                page.reloadFlightRouteList()
                close()
            }
        }
        onRejected: {
            mobileAdaptor.vibrateBrief()
            close()
        }

    } // renameDialog

    Dialog {
        id: shareErrorDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Error exporting data…")
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)

        Label {
            id: shareErrorDialogLabel
            width: shareErrorDialog.availableWidth
            onLinkActivated: Qt.openUrlExternally(link)
            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

        standardButtons: Dialog.Ok
        modal: true

    }

} // Page
