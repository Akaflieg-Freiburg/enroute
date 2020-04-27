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
    title: qsTr("Save Flight Route…")
    modal: true

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

    standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Save

    TextField {
        id: fileName

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        focus: true
        placeholderText: "File Name"

        onTextChanged: dlg.standardButton(DialogButtonBox.Save).enabled = (text !== "")

        onAccepted: {
            if (fileName.text !== "")
                dlg.accept()
        }

        Component.onCompleted: fileName.text = flightRoute.suggestedFilename()
    }

    Component.onCompleted: dlg.standardButton(DialogButtonBox.Save).enabled = (fileName.text !== "")

    onRejected: {
        MobileAdaptor.vibrateBrief()
        close()
    }

    onAccepted: {
        MobileAdaptor.vibrateBrief()
        if (flightRoute.fileExists(fileName.text))
            overwriteDialog.open()
        else
            saveToLibrary()
    }

    function saveToLibrary() {
        console.log("STL")
        var errorString = flightRoute.saveToLibrary(fileName.text)
        if (errorString !== "") {
            console.log("Error")
        }
    }

    Dialog {
        id: overwriteDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Overwrite file '%1'?").arg(fileName.text)
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            MobileAdaptor.vibrateBrief()
            dlg.saveToLibrary()
        }
        onRejected: {
            MobileAdaptor.vibrateBrief()
            close()
            dlg.open()
        }

    }

} // Dialog
