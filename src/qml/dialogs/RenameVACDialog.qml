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


CenteringDialog {
    id: renameDialog

    required property string oldName

    title: qsTr("Rename Approach Chart")

    modal: true

    ColumnLayout {
        anchors.fill: parent

        Label {
            Layout.fillWidth: true

            text: qsTr("Enter new name for the approach chart <strong>%1</strong>.").arg(renameDialog.oldName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        MyTextField {
            id: newName

            Layout.fillWidth: true
            focus: true

            placeholderText: renameDialog.oldName
            text: renameDialog.oldName

            onAccepted: renameDialog.onAccepted()
        }
    }

    footer: DialogButtonBox {
        standardButtons: DialogButtonBox.Cancel

        ToolButton {
            id: renameButton

            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            enabled: (newName.text !== "") && (newName.text !== renameDialog.oldName)
            text: qsTr("Rename")
        }
    }

    onAccepted: {
        PlatformAdaptor.vibrateBrief()
        var errorMsg = DataManager.renameVAC(renameDialog.oldName, newName.text)

        if (errorMsg === "")
            Global.toast.doToast(qsTr("Flight route renamed"))
        else
            Global.toast.doToast(qsTr("Error: %1").arg(errorMsg))

        close()
    }

    onRejected: {
        PlatformAdaptor.vibrateBrief()
        close()
    }
}
