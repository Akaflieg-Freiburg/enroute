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
import "../items"

Page {
    id: page
    title: qsTr("Flight Route Library")
    focus: true

    property bool isIos: Qt.platform.os == "ios"
    property bool isAndroid: Qt.platform.os === "android"
    property bool isAndroidOrIos: isAndroid || isIos

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
                PlatformAdaptor.vibrateBrief()
                headerMenuX.open()
            }

            AutoSizingMenu{
                id: headerMenuX

                MenuItem {
                    text: qsTr("Info…")
                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        infoDialog.open()
                    }

                } // ToolButton

                MenuItem {
                    text: qsTr("Import…")
                    enabled: !isAndroidOrIos
                    visible: !isAndroidOrIos
                    height: isAndroidOrIos ? 0 : undefined

                    onTriggered: {
                        PlatformAdaptor.vibrateBrief()
                        highlighted = false
                        FileExchange.importContent()
                    }
                }

            }

        }

    }

    RowLayout {
        id: filterRow

        anchors.left: parent.left
        anchors.leftMargin: SafeInsets.left+font.pixelSize
        anchors.right: parent.right
        anchors.rightMargin: SafeInsets.right+font.pixelSize
        anchors.top: parent.top
        anchors.topMargin: page.font.pixelSize

        Label {
            Layout.alignment: Qt.AlignBaseline

            text: qsTr("Filter")
        }

        MyTextField {
            id: textInput

            Layout.alignment: Qt.AlignBaseline
            Layout.fillWidth: true
        }
    }

    Pane {

        anchors.top: filterRow.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        bottomPadding: SafeInsets.bottom
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: font.pixelSize

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
                        PlatformAdaptor.vibrateBrief()
                        finalFileName = modelData
                        if (Navigator.flightRoute.size > 0)
                            overwriteDialog.open()
                        else
                            openFromLibrary()
                    }

                    swipe.onCompleted: {
                        PlatformAdaptor.vibrateBrief()
                        finalFileName = modelData
                        removeDialog.open()
                    }

                }

                ToolButton {
                    id: cptMenuButton

                    icon.source: "/icons/material/ic_more_horiz.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        cptMenu.open()
                    }

                    AutoSizingMenu {
                        id: cptMenu

                        AutoSizingMenu {
                            title: isAndroidOrIos ? qsTr("Share…") : qsTr("Export…")

                            MenuItem {
                                text: qsTr("… to GeoJSON file")
                                onTriggered: {
                                    cptMenu.close()
                                    PlatformAdaptor.vibrateBrief()
                                    highlighted = false
                                    parent.highlighted = false

                                    var errorString = FileExchange.shareContent(Librarian.get(Librarian.Routes, modelData).toGeoJSON(), "application/geo+json", Librarian.get(Librarian.Routes, modelData).suggestedFilename())
                                    if (errorString === "abort") {
                                        toast.doToast(qsTr("Aborted"))
                                        return
                                    }
                                    if (errorString !== "") {
                                        shareErrorDialog.text = errorString
                                        shareErrorDialog.open()
                                        return
                                    }
                                    if (isAndroid)
                                        toast.doToast(qsTr("Flight route shared"))
                                    else if(!isIos)
                                        toast.doToast(qsTr("Flight route exported"))
                                }
                            }

                            MenuItem {
                                text: qsTr("… to GPX file")
                                onTriggered: {
                                    cptMenu.close()
                                    PlatformAdaptor.vibrateBrief()
                                    highlighted = false
                                    parent.highlighted = false

                                    var errorString = FileExchange.shareContent(Librarian.get(Librarian.Routes, modelData).toGpx(), "application/gpx+xml", Librarian.get(Librarian.Routes, modelData).suggestedFilename())
                                    if (errorString === "abort") {
                                        toast.doToast(qsTr("Aborted"))
                                        return
                                    }
                                    if (errorString !== "") {
                                        shareErrorDialog.text = errorString
                                        shareErrorDialog.open()
                                        return
                                    }
                                    if (isAndroid)
                                        toast.doToast(qsTr("Flight route shared"))
                                    else if (!isIos)
                                        toast.doToast(qsTr("Flight route exported"))
                                }
                            }
                        }

                        AutoSizingMenu {
                            title: qsTr("Open in Other App…")
                            enabled: Qt.platform.os !== "ios"

                            MenuItem {
                                text: qsTr("… in GeoJSON format")

                                onTriggered: {
                                    PlatformAdaptor.vibrateBrief()
                                    highlighted = false
                                    parent.highlighted = false

                                    var errorString = FileExchange.viewContent(Librarian.get(Librarian.Routes, modelData).toGeoJSON(), "application/geo+json", "FlightRoute-%1.geojson")
                                    if (errorString !== "") {
                                        shareErrorDialog.text = errorString
                                        shareErrorDialog.open()
                                    } else
                                        toast.doToast(qsTr("Flight route opened in other app"))
                                }
                            }

                            MenuItem {
                                text: qsTr("… in GPX format")

                                onTriggered: {
                                    PlatformAdaptor.vibrateBrief()
                                    highlighted = false
                                    parent.highlighted = false

                                    var errorString = FileExchange.viewContent(Librarian.get(Librarian.Routes, modelData).toGpx(), "application/gpx+xml", "FlightRoute-%1.gpx")
                                    if (errorString !== "") {
                                        shareErrorDialog.text = errorString
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
                                PlatformAdaptor.vibrateBrief()
                                finalFileName = modelData
                                renameName.text = modelData
                                renameDialog.open()
                            }

                        } // renameAction

                        Action {
                            id: removeAction
                            text: qsTr("Remove…")
                            onTriggered: {
                                PlatformAdaptor.vibrateBrief()
                                finalFileName = modelData
                                removeDialog.open()
                            }
                        } // removeAction
                    } // AutoSizingMenu

                } // ToolButton

            }

        }

        DecoratedListView {
            id: wpList

            anchors.fill: parent

            clip: true

            model: Librarian.entries(Librarian.Routes, textInput.displayText)
            delegate: flightRouteDelegate
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        Label {
            anchors.fill: parent
            anchors.topMargin: font.pixelSize*2

            visible: (wpList.count === 0)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            leftPadding: font.pixelSize*2
            rightPadding: font.pixelSize*2

            textFormat: Text.StyledText
            wrapMode: Text.Wrap
            text: (textInput.text === "")
                  ? qsTr("<h3>Sorry!</h3><p>No flight routes available. To add a route here, chose 'Flight Route' from the main menu, edit a route and save it to the library.</p>")
                  : qsTr("<h3>Sorry!</h3><p>No flight routes match your filter criteria.</p>")
        }
    }

    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function openFromLibrary() {
        var errorString = Navigator.flightRoute.load(Librarian.fullPath(Librarian.Routes, finalFileName))
        if (errorString !== "") {
            fileError.text = errorString
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

        title: qsTr("An Error Occurred…")
        standardButtons: Dialog.Ok
    }

    LongTextDialog {
        id: infoDialog
        standardButtons: Dialog.Ok

        title: qsTr("Flight Route Library")
        text: Librarian.getStringFromRessource(":text/flightRouteLibraryInfo.html").arg(Librarian.directory(Librarian.Routes))
    }

    LongTextDialog {
        id: overwriteDialog

        title: qsTr("Overwrite Current Flight Route?")
        standardButtons: Dialog.No | Dialog.Yes

        text: qsTr("Loading the route <strong>%1</strong> will overwrite the current route. Once overwritten, the current flight route cannot be restored.").arg(finalFileName)

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            page.openFromLibrary()
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
            close()
        }
    }

    LongTextDialog {
        id: removeDialog

        title: qsTr("Remove from Device?")
        standardButtons: Dialog.No | Dialog.Yes

        text: qsTr("Once the flight route <strong>%1</strong> is removed, it cannot be restored.").arg(page.finalFileName)

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            Librarian.remove(Librarian.Routes, page.finalFileName)
            page.reloadFlightRouteList()
            toast.doToast(qsTr("Flight route removed from device"))
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
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
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }

            MyTextField {
                id: renameName

                Layout.fillWidth: true
                focus: true

                onAccepted: renameDialog.onAccepted()
            }

        }

        footer: DialogButtonBox {
            ToolButton {
                id: renameButton

                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                enabled: (renameName.text !== "") && !(Librarian.exists(Librarian.Routes, renameName.text))
                text: qsTr("Rename")
            }
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            if ((renameName.text !== "") && !Librarian.exists(Librarian.Routes, renameName.text)) {
                Librarian.rename(Librarian.Routes, finalFileName, renameName.text)
                page.reloadFlightRouteList()
                close()
                toast.doToast(qsTr("Flight route renamed"))
            }
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
            close()
        }
    }

    LongTextDialog {
        id: shareErrorDialog

        title: qsTr("Error Exporting Data…")
        standardButtons: Dialog.Ok
    }

} // Page
