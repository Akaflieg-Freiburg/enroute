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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14

import "../items"

Page {
    id: page
    title: qsTr("Flight Route Library")
    focus: true

    header: StandardHeader {}

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
                icon.color: "transparent"
            }

            ToolButton {
                id: cptMenuButton

                icon.source: "/icons/material/ic_more_horiz.svg"

                onClicked: {
                    MobileAdaptor.vibrateBrief()
                    cptMenu.popup()
                }

                AutoSizingMenu {
                    id: cptMenu

                    Action {
                        id: openAction

                        text: qsTr("Open")
                        icon.source: "/icons/material/ic_open_in_new.svg"

                        onTriggered: {
                                MobileAdaptor.vibrateBrief()
                                finalFileName = modelData
                                if (flightRoute.routeObjects.length > 0)
                                    overwriteDialog.open()
                                else
                                    openFromLibrary()
                        }

                    } // openAction

                    Action {
                        id: removeAction

                        text: qsTr("Remove from device")
                        icon.source: "/icons/material/ic_delete.svg"

                        onTriggered: {
                            MobileAdaptor.vibrateBrief()
                            finalFileName = modelData
                            removeDialog.open()
                        }
                    } // removeAction
                }

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

        model: librarian.flightRoutes(textInput.text)
        delegate: flightRouteDelegate
        ScrollIndicator.vertical: ScrollIndicator {}
    }

    Rectangle {
        anchors.top: textInput.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        color: "white"
        visible: wpList.count === 0

        Label {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Qt.application.font.pixelSize*2

            horizontalAlignment: Text.AlignHCenter
            textFormat: Text.RichText
            wrapMode: Text.Wrap
            text: qsTr("<h3>Sorry!</h3><p>No flight routes available. To add a route here, chose 'Flight Route' from the main menu, edit a route and save it to the library.</p>")
            onLinkActivated: Qt.openUrlExternally(link)
        }
    }


    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function openFromLibrary() {
        var errorString = flightRoute.load(librarian.flightRouteFullPath(finalFileName))
        if (errorString !== "") {
            lbl.text = errorString
            fileError.open()
            return
        }
        stackView.push("FlightRoutePage.qml")
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
                    horizontalAlignment: Text.AlignJustify
                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                } // Label
            } // Item
        } // ScrollView

        Connections {
            target: sensorGesture
            onDetected: close()
        }
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

            text: qsTr("Once overwritten, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            MobileAdaptor.vibrateBrief()
            page.openFromLibrary()
        }
        onRejected: {
            MobileAdaptor.vibrateBrief()
            close()
        }

        Connections {
            target: sensorGesture
            onDetected: close()
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
            MobileAdaptor.vibrateBrief()
            librarian.flightRouteRemove(page.finalFileName)
            var cache = textInput.text
            textInput.text = textInput.text+"XXXXX"
            textInput.text = cache
        }
        onRejected: {
            MobileAdaptor.vibrateBrief()
            close()
        }

        Connections {
            target: sensorGesture
            onDetected: close()
        }
    }

} // Page
