/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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

Dialog {
    id: dlg
    title: qsTr("Save Flight Route…")

    modal: true
    focus: true

    // Width and height are chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: parent.height-2*Qt.application.font.pixelSize
    implicitHeight: height

    standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Save

    Component {
        id: fileDelegate

        ItemDelegate {
            id: idel
            text: modelData
            icon.source: "/icons/material/ic_directions.svg"
            icon.color: "transparent"

            anchors.left: parent.left
            anchors.right: parent.right

            onClicked: {
                mobileAdaptor.vibrateBrief()
                finalFileName = modelData
                dlg.close()
                overwriteDialog.open()
            }
        }

    } // fileDelegate


    ColumnLayout {
        anchors.fill: parent

        Label {
            Layout.fillWidth: true

            text: qsTr("Enter a name or choose an existing name from the list below.")
            color: Material.primary
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        TextField {
            id: fileName

            Layout.fillWidth: true
            focus: true
            placeholderText: qsTr("Flight Route Name")

            onTextChanged: dlg.standardButton(DialogButtonBox.Save).enabled = (text !== "")

            onAccepted: {
                if (fileName.text === "")
                    return
                dlg.accept()
            }
        }

        ListView {
            id: lView

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            model: librarian.flightRoutes("")
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: fileDelegate
        }

    } // ColumnLayout

    onOpened: {
        dlg.standardButton(DialogButtonBox.Save).enabled = (fileName.text !== "")
        fileName.text = flightRoute.suggestedFilename()
    }

    onRejected: {
        mobileAdaptor.vibrateBrief()
        close()
    }

    onAccepted: {
        mobileAdaptor.vibrateBrief()
        if (fileName.text === "")
            return
        finalFileName = fileName.text
        if (librarian.flightRouteExists(finalFileName))
            overwriteDialog.open()
        else
            saveToLibrary()
    }

    Component.onCompleted: dlg.standardButton(DialogButtonBox.Save).enabled = (text !== "")

    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function saveToLibrary() {
        var errorString = flightRoute.save(librarian.flightRouteFullPath(finalFileName))
        if (errorString !== "") {
            lbl.text = errorString
            fileError.open()
        } else
            toast.doToast(qsTr("Flight route %1 saved").arg(finalFileName))
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

            contentHeight: lbl.height
            contentWidth: fileError.availableWidth

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

            clip: true

            Label {
                id: lbl
                width: dlg.availableWidth
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                onLinkActivated: Qt.openUrlExternally(link)
            } // Label
        } // ScrollView

    }  // Dialog: fileError

    Dialog {
        id: overwriteDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        title: qsTr("Overwrite flight route?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("The route <strong>%1</strong> already exists in the library. Do you wish to overwrite it?").arg(finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            mobileAdaptor.vibrateBrief()
            dlg.saveToLibrary()
        }

        onRejected: {
            mobileAdaptor.vibrateBrief()
            close()
            dlg.open()
        }

    } // Dialog: overwriteDialog

} // Dialog
