/***************************************************************************
 *   Copyright (C) 2020-2022 by Stefan Kebekus                             *
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

import akaflieg_freiburg.enroute
import enroute 1.0

CenteringDialog {
    id: dlg
    title: qsTr("Save Aircraft…")

    modal: true

    standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Save

    Component {
        id: fileDelegate

        ItemDelegate {
            id: idel
            text: modelData
            icon.source: "/icons/material/ic_airplanemode_active.svg"

            anchors.left: parent.left
            anchors.right: parent.right

            onClicked: {
                global.platformAdaptor().vibrateBrief()
                finalFileName = modelData
                dlg.close()
                overwriteDialog.open()
            }
        }

    }

    ColumnLayout {
        anchors.fill: parent

        Label {
            Layout.fillWidth: true

            text: qsTr("Enter a name or choose an existing name from the list below.")
            color: Material.accent
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        TextField {
            id: fileName

            Layout.fillWidth: true
            focus: true
            placeholderText: qsTr("Aircraft Name")

            onDisplayTextChanged: dlg.standardButton(DialogButtonBox.Save).enabled = (displayText !== "")

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
            Layout.preferredHeight: contentHeight

            clip: true
            model: global.librarian().entries(Librarian.Aircraft)
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: fileDelegate
        }

    } // ColumnLayout

    onOpened: {
        fileName.text = global.navigator().aircraft.name
        dlg.standardButton(DialogButtonBox.Save).enabled = (fileName.text !== "")
    }

    onRejected: {
        global.platformAdaptor().vibrateBrief()
        close()
    }

    onAccepted: {
        global.platformAdaptor().vibrateBrief()
        if (fileName.text === "")
            return
        finalFileName = fileName.text
        if (global.librarian().exists(Librarian.Aircraft, finalFileName))
            overwriteDialog.open()
        else
            saveToLibrary()
    }

    Component.onCompleted: dlg.standardButton(DialogButtonBox.Save).enabled = (text !== "")

    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function saveToLibrary() {
        var errorString = global.navigator().aircraft.save(global.librarian().fullPath(Librarian.Aircraft, finalFileName))
        if (errorString !== "") {
            lbl.text = errorString
            fileError.open()
        } else
            toast.doToast(qsTr("Aircraft %1 saved").arg(finalFileName))
    }

    CenteringDialog {
        id: fileError

        modal: true
        title: qsTr("An Error Occurred…")
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

    CenteringDialog {
        id: overwriteDialog

        title: qsTr("Overwrite Aircraft?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("The aircraft <strong>%1</strong> already exists in the library. Do you wish to overwrite it?").arg(finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            dlg.saveToLibrary()
        }

        onRejected: {
            global.platformAdaptor().vibrateBrief()
            close()
            dlg.open()
        }

    } // Dialog: overwriteDialog

} // Dialog
