/***************************************************************************
 *   Copyright (C) 2019-2023 by Stefan Kebekus                             *
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
    title: qsTr("Manual")
    property string fileName: ""

    header: PageHeader {
        height: 60 + SafeInsets.top
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: SafeInsets.top

        ToolButton {
            id: closeButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_clear.svg"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                stackView.pop()
            }
        }

        Label {
            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: backButton.left

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.pixelSize: 20
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: backButton

            anchors.verticalCenter: parent.verticalCenter

            anchors.right: forwardButton.left
            visible: webView.canGoBack

            icon.source: "/icons/material/ic_arrow_back.svg"
            onClicked: {
                webView.goBack()
            }

        }

        ToolButton {
            id: forwardButton

            anchors.verticalCenter: parent.verticalCenter

            anchors.right: parent.right
            visible: webView.canGoForward

            icon.source: "/icons/material/ic_arrow_forward.svg"
            onClicked: {
                webView.goForward()
            }

        }

    }

    WebView {
        id: webView

        anchors.fill: parent
        anchors.bottomMargin: SafeInsets.bottom

        url: "file://" + manual_location + "/" + pg.fileName

        settings.allowFileAccess: true
    }


    Keys.onReleased: event => {
        if (event.key === Qt.Key_Back) {
            if (webView.canGoBack) {
                webView.goBack()
            } else {
                if (stackView.depth > 1)
                    stackView.pop()
            }
            event.accepted = true
        }
    }

}
