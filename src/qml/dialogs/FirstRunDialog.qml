/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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

import akaflieg_freiburg.enroute

CenteringDialog {
    id: dialogMain

    Component {
        id: firstStart

        ScrollView{
            id: sv
            required property var dialogMain

            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            Label {
                text: Librarian.getStringFromRessource(":text/firstStart.html")
                width: sv.dialogMain.width
                textFormat: Text.RichText
                linkColor: Material.accent
                wrapMode: Text.Wrap
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }

            function accept() {
                GlobalSettings.acceptedTerms = 1
            }
        }

    }

    Component {
        id: privacy

        ScrollView{
            id: sv
            required property var dialogMain

            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            Label {
                text: Librarian.getStringFromRessource(":text/privacy.html")
                width: sv.dialogMain.availableWidth
                textFormat: Text.RichText
                linkColor: Material.accent
                wrapMode: Text.Wrap
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }

            function accept() {
                GlobalSettings.privacyHash = Librarian.getStringHashFromRessource(":text/privacy.html")
            }
        }
    }

    closePolicy: Popup.NoAutoClose
    modal: true

    StackView {
        id: stack

        implicitHeight: empty ? 0 : currentItem.implicitHeight
        Behavior on implicitHeight {
            NumberAnimation { duration: 200 }
            enabled: dialogMain.opened
        }

        anchors.fill: parent
    }

    function conditionalOpen() {
        if (GlobalSettings.privacyHash !== Librarian.getStringHashFromRessource(":text/privacy.html"))
            stack.push(privacy, {"dialogMain": dialogMain})
        if (GlobalSettings.acceptedTerms === 0)
            stack.push(firstStart, {"dialogMain": dialogMain})
        if (!stack.empty)
            open()
        return !stack.empty
    }

    footer: DialogButtonBox {
        ToolButton {
            text: qsTr("Accept")

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                stack.currentItem.accept()
                if (stack.depth > 1)
                    stack.pop()
                else
                    dialogMain.close()
            }
        }
        ToolButton {
            text: qsTr("Quit App")

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                Qt.quit()
            }
        }
    }
}
