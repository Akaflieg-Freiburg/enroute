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
import QtWebView

import akaflieg_freiburg.enroute
import "../items"

Page {
    id: pg
    title: qsTr("Resolve Coordinates")

    required property string mapURL


    header: PageHeader {

        height: 60 + SafeInsets.top
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: SafeInsets.top

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                stackView.pop()
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.pixelSize: 20
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_info_outline.svg"
            onClicked: {
                PlatformAdaptor.vibrateBrief()
                Global.dialogLoader.active = false
                Global.dialogLoader.setSource("../dialogs/LongTextDialog.qml", {
                                                  title: pg.title,
                                                  text: "<p>"
                                                        + qsTr("This page features an embedded web browser, accessing an external web site in order to retrieve geographic coordinates for locations shared with <strong>Enroute Flight Navigation</strong>.")
                                                        + "</p>"
                                                        + "<p>"
                                                        + qsTr("This page should close automatically within a few seconds.")
                                                        + " "
                                                        + qsTr("If it remains open for longer than 30 seconds, then either user interaction is required, or the coordinate lookup has failed.")
                                                        + "</p>",
                                                  standardButtons: Dialog.Ok
                                              })
                Global.dialogLoader.active = true
            }
        }
    }

    WebView {
        id: webView

        anchors.fill: parent
        anchors.bottomMargin: SafeInsets.bottom

        // User Agent Setting is necessary, or else Google will reply
        // with Android intents rather than https urls.
        httpUserAgent: "EnrouteFlightNavigation/1.0"
        url: pg.mapURL

        onUrlChanged: {
            if (FileExchange.processTextQuiet(url)) {
                PlatformAdaptor.vibrateBrief()
                stackView.pop()
            }
        }
    }

}
