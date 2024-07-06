/***************************************************************************
 *   Copyright (C) 2024 by Stefan Kebekus                                  *
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
import "../items"
import "../pages"

CenteringDialog {
    id: privacyWarning

    required property bool openExternally
    required property string text
    required property string url

    modal: true

    title: qsTr("Privacy warning")

    ColumnLayout {
        anchors.fill: parent

        DecoratedScrollView{
            Layout.fillHeight: true
            Layout.fillWidth: true

            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            Label {
                id: lbl
                text: "<p>"
                      + privacyWarning.text
                      + " " + qsTr("The authors of <strong>Enroute Flight Navigation</strong> do not control the external website.")
                      + " " + qsTr("They do not know what data it collects or how that data is processed.")
                      + "</p>"
                      + "<p>"
                      + " " + qsTr("With the click on OK, you consent to Enroute accessing the external site from your device.")
                      + " " + qsTr("Click OK only if you agree with the terms and privacy policies of that site.")
                      + "</p>"

                width: privacyWarning.availableWidth
                textFormat: Text.RichText
                wrapMode: Text.Wrap
            }
        }

        Item {
            Layout.preferredHeight: lbl.font.pixelSize
        }

        WordWrappingCheckDelegate {
            id: alwaysOpen

            Layout.fillWidth: true

            text: qsTr("Always open external web sites, do not ask again")
            checked: GlobalSettings.alwaysOpenExternalWebsites
        }
    }

    standardButtons: Dialog.Cancel|Dialog.Ok

    onAccepted: {
        PlatformAdaptor.vibrateBrief()
        GlobalSettings.alwaysOpenExternalWebsites = alwaysOpen.checked
        if (openExternally)
            Qt.openUrlExternally(url)
        else
            stackView.push("../pages/URLResolver.qml", {mapURL: url})
    }
}
