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
import "../dialogs"
import "../pages"


CenteringDialog {
    id: renameDialog

    required property string oldName

    title: qsTr("Rename")

    modal: true

    ColumnLayout {
        anchors.fill: parent

        Label {
            Layout.fillWidth: true

            text: qsTr("Enter new name for the VAC <strong>%1</strong>.").arg(finalFileName)
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
