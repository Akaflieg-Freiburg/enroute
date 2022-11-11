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

import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import "../items"

Page {
    id: pg
    title: qsTr("About Enroute Flight Navigation")

    header: StandardHeader {}

    TabBar {
        id: bar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        leftPadding: view.leftScreenMargin
        rightPadding: view.rightScreenMargin

        currentIndex: sv.currentIndex

        TabButton { text: "Enroute" }
        TabButton { text: qsTr("Authors") }
        TabButton { text: qsTr("License") }
        Material.elevation: 3
    }

    SwipeView {
        id: sv

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: view.bottomScreenMargin
        anchors.leftMargin: view.leftScreenMargin
        anchors.rightMargin: view.rightScreenMargin

        clip: true
        currentIndex: bar.currentIndex
        
        ScrollView {
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

            Label {
                id: lbl1
                text: "<style>a:link { color: " + Material.accent + "; }</style>"+global.librarian().getStringFromRessource(":text/info_enroute.html")
                textFormat: Text.RichText
                linkColor: Material.accent
                width: sv.availableWidth

                wrapMode: Text.Wrap
                topPadding: view.font.pixelSize*1
                leftPadding: view.font.pixelSize*0.5
                rightPadding: view.font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
        
        ScrollView {
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

            Label {
                id: lbl2
                text: "<style>a:link { color: " + Material.accent + "; }</style>"+global.librarian().getStringFromRessource(":text/authors.html")
                textFormat: Text.RichText // Link OK
                width: sv.availableWidth
                wrapMode: Text.Wrap
                topPadding: view.font.pixelSize*1
                leftPadding: view.font.pixelSize*0.5
                rightPadding: view.font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        ScrollView {
            clip: true
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

            Label {
                id: lbl3
                text: "<style>a:link { color: " + Material.accent + "; }</style>"+global.librarian().getStringFromRessource(":text/info_license.html")
                textFormat: Text.RichText
                linkColor: Material.accent
                width: sv.availableWidth
                wrapMode: Text.Wrap
                topPadding: view.font.pixelSize*1
                leftPadding: view.font.pixelSize*0.5
                rightPadding: view.font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

    } // StackView
} // Page
