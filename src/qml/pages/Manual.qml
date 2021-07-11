/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15
import QtWebView 1.15

Page {
    id: pg
    title: qsTr("Manual")


    header: ToolBar {

        Material.foreground: "white"

        ToolButton {
            id: closeButton

            anchors.left: parent.left
            anchors.leftMargin: drawer.dragMargin

            icon.source: "/icons/material/ic_clear.svg"
            onClicked: {
                global.mobileAdaptor().vibrateBrief()
                if (stackView.depth > 1) {
                    stackView.pop()
                } else {
                    drawer.open()
                }
            }
        }

        Label {
            anchors.left: closeButton.right
            anchors.right: backButton.left
            anchors.bottom: parent.bottom
            anchors.top: parent.top

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.bold: true
            horizontalAlignment: Qt.AlignHCenter
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: backButton

            anchors.right: forwardButton.left
            enabled: webView.canGoBack

            icon.source: "/icons/material/ic_arrow_back.svg"
            onClicked: {
                webView.goBack()
            }

        }

        ToolButton {
            id: forwardButton

            anchors.right: parent.right
            enabled: webView.canGoForward

            icon.source: "/icons/material/ic_arrow_forward.svg"
            onClicked: {
                webView.goForward()
            }

        }

    }


    WebView {
        id: webView

        anchors.fill: parent

        url: "https://akaflieg-freiburg.github.io/enrouteText/experimentalmanual/html/"
    }


    Keys.onReleased: {
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

} // Page
