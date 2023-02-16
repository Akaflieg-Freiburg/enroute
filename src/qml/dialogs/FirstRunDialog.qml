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
                text: "<h3>"+qsTr("Welcome to Enroute Flight Navigation - A project of Akaflieg Freiburg")+"</h3>"
                      + "<p>" + qsTr("Thank you for using this flight navigation app!  Before we get started, we need to point out that <strong>this app and the aviation data come with no guarantees</strong>.")+"</p>"
                      + "<p>" + qsTr("The app is not certified to satisfy aviation standards. It may contain errors and may not work as expected.") + "</p>"
                      + "<p>" + qsTr("The aviation data does not come from official sources. It might be incomplete, outdated or otherwise incorrect.") + "</p>"
                      + "<p>" + qsTr("<strong>This app is no substitute for proper flight preparation or good pilotage.</strong> We hope you enjoy the app and that you do find it useful.") + "</p>"
                      + "<p>" + qsTr("Fly safely and enjoy many happy landings!") + "</p>"
                      + "<p>&#8212; Stefan Kebekus.</p>";

                width: sv.dialogMain.availableWidth
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
                text: "<h3>" + qsTr("Privacy") + "</h3>"
                      + "<p>" + qsTr("Please take a minute to review our privacy policies.") + "</p>"
                      + Librarian.getStringFromRessource(":text/privacy.html")
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

    Component {
        id: permissions

        ScrollView{
            id: sv

            required property var dialogMain
            required property string text

            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            Label {
                text: "<h3>" + qsTr("Privacy-relevant permissions requested by this app") + "</h3>"
                      + "<p>" + qsTr("Please grant the following permissiona when prompted.") + "</p>"
                      + sv.text
                width: sv.dialogMain.availableWidth
                textFormat: Text.RichText
                linkColor: Material.accent
                wrapMode: Text.Wrap
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }

            function accept() {
                PlatformAdaptor.requestPermissionsSync()
                PositionProvider.startUpdates()
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
        var missingPermissionsText = PlatformAdaptor.checkPermissions()
        if (missingPermissionsText === "")
            PositionProvider.startUpdates()
        else
            stack.push(permissions, {"dialogMain": dialogMain, "text": missingPermissionsText})
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
