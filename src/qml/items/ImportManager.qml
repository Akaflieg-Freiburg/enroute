/***************************************************************************
 *   Copyright (C) 2020 by Stefan Kebekus                                  *
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

import enroute 1.0

Item {
    id: importManager

    property string filePath: ""
    property int fileFunction: MobileAdaptor.UnknownFunction

    Connections {
        target: mobileAdaptor
        onOpenFileRequest: {

            if (fileFunction === MobileAdaptor.UnknownFunction) {
                errLbl.text = qsTr("The file type of the file <strong>%1</strong> could not be recognized.").arg(fileName)
                errorDialog.open()
                return
            }

            importManager.filePath = fileName
            importManager.fileFunction = fileFunction
            importDialog.open()
      }
    } // Connections

    Dialog {
        id: importDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Import Flight Route?")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            id: lbl

            width: importDialog.availableWidth

            text: qsTr("This will overwrite the current route. Once overwritten, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

    } // importDialog

    Dialog {
        id: errorDialog
        anchors.centerIn: parent
        parent: Overlay.overlay
        standardButtons: Dialog.Cancel
        modal: true

        title: qsTr("Error importing flight route")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            id: errLbl

            width: importDialog.availableWidth

            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

    } // errorDialog

}
