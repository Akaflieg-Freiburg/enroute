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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Dialog {
    id: dlg

    property var dialogArgs: undefined // SSID


    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-view.font.pixelSize, 40*view.font.pixelSize)
    height: Math.min(parent.height-view.font.pixelSize, implicitHeight)

    // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
    // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
    parent: Overlay.overlay
    x: (parent.width-width)/2.0
    y: (parent.height-height)/2.0

    modal: true

    title: qsTr("Store password?")

    Label {
        width: dlg.availableWidth

        text: qsTr("<p><strong>Enroute Flight Navigation</strong> is now connected to the traffic data receiver in the WiFi network <strong>%1</strong>.</p>").arg(dialogArgs)
              +qsTr("<p>Would you like to store the password for the traffic data receiver? Note that the stored password will not be encrypted. You can clear the password storage from the setting page.</p>")
        textFormat: Text.RichText
        wrapMode: Text.Wrap
    }

    footer: DialogButtonBox {
        ToolButton {
            text: qsTr("Store")
            DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
        }
        ToolButton {
            text: qsTr("Cancel")
            DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
        }

    } // DialogButtonBox


    onAccepted: global.passwordDB().setPassword(dialogArgs, dialogLoader.text)

} // Dialog
