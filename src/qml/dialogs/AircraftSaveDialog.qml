/***************************************************************************
 *   Copyright (C) 2020-2023 by Stefan Kebekus                             *
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
import "../items"

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
                PlatformAdaptor.vibrateBrief()
                finalFileName = modelData
                dlg.close()
                overwriteDialog.open()
            }
        }

    }

    ColumnLayout {
        width: dlg.availableWidth

        Label {
            Layout.fillWidth: true

            text: qsTr("Enter a name or choose an existing name from the list below.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        MyTextField {
            id: fileName

            Layout.fillWidth: true
            focus: true

            onDisplayTextChanged: dlg.standardButton(DialogButtonBox.Save).enabled = (displayText !== "")

            onAccepted: {
                if (fileName.text === "")
                    return
                dlg.accept()
            }
        }

        DecoratedListView {
            id: lView

            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.preferredHeight: contentHeight

            clip: true
            model: Librarian.entries(Librarian.Aircraft)
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: fileDelegate
        }

    } // ColumnLayout

    onOpened: {
        fileName.text = Navigator.aircraft.name
        dlg.standardButton(DialogButtonBox.Save).enabled = (fileName.text !== "")
    }

    onRejected: {
        PlatformAdaptor.vibrateBrief()
        close()
    }

    onAccepted: {
        PlatformAdaptor.vibrateBrief()
        if (fileName.text === "")
            return
        finalFileName = fileName.text
        if (Librarian.exists(Librarian.Aircraft, finalFileName))
            overwriteDialog.open()
        else
            saveToLibrary()
    }

    Component.onCompleted: dlg.standardButton(DialogButtonBox.Save).enabled = (text !== "")

    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function saveToLibrary() {
        var errorString = Navigator.aircraft.save(Librarian.fullPath(Librarian.Aircraft, finalFileName))
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

        DecoratedScrollView{
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
        } // DecoratedScrollView

    }  // Dialog: fileError

    LongTextDialog {
        id: overwriteDialog

        title: qsTr("Overwrite Aircraft?")
        text: qsTr("The aircraft <strong>%1</strong> already exists in the library. Do you wish to overwrite it?").arg(finalFileName)

        standardButtons: Dialog.No | Dialog.Yes

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            dlg.saveToLibrary()
        }

        onRejected: {
            PlatformAdaptor.vibrateBrief()
            close()
            dlg.open()
        }

    } // Dialog: overwriteDialog

} // Dialog
