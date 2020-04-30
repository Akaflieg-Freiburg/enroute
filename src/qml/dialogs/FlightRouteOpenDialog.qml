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

import enroute 1.0

Dialog {
    id: dlg
    title: qsTr("Load Flight Route…")
    modal: true

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

    standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Open

    Component {
        id: fileDelegate

        ItemDelegate {
            id: idel
            text: modelData

            anchors.left: parent.left
            anchors.right: parent.right
            Layout.fillWidth: true

            Component.onCompleted: console.log(modelData)


            onClicked: {
                fileName.text = modelData
            }
        }

    }

    TextField {
        id: fileName

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        focus: true
        placeholderText: "File Name"

        onTextChanged: dlg.standardButton(DialogButtonBox.Open).enabled = (text !== "")

        onAccepted: {
            if (fileName.text !== "")
                dlg.accept()
        }

        Component.onCompleted: fileName.text = flightRoute.suggestedFilename()
    }

    ListView {
        anchors.top: fileName.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        model: library.flightRoutes()
        ScrollIndicator.vertical: ScrollIndicator {}

        Component.onCompleted: console.log(library.flightRoutes())
        delegate: fileDelegate
    }


    Component.onCompleted: dlg.standardButton(DialogButtonBox.Open).enabled = (fileName.text !== "")

    onRejected: {
        MobileAdaptor.vibrateBrief()
        close()
    }

    onAccepted: {
        MobileAdaptor.vibrateBrief()
        if (!flightRoute.isEmpty)
            overwriteDialog.open()
        else
            openFromLibrary()
    }

    Connections {
        target: sensorGesture
        onDetected: close()
    }

    function openFromLibrary() {
        var errorString = flightRoute.loadFromLibrary(fileName.text)
        if (errorString !== "") {
            lbl.text = errorString
            fileError.open()
        }
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
                width: dlg.availableWidth

                Label {
                    id: lbl
                    width: dlg.availableWidth
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
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            MobileAdaptor.vibrateBrief()
            dlg.openFromLibrary()
        }
        onRejected: {
            MobileAdaptor.vibrateBrief()
            close()
            dlg.open()
        }

        Connections {
            target: sensorGesture
            onDetected: close()
        }
    }

} // Dialog
