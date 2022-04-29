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

        currentIndex: bar.currentIndex
        
        ScrollView {
            clip: true

            // The label that we really want to show is wrapped into an Item. This allows
            // to set implicitHeight, and thus compute the implicitHeight of the Dialog
            // without binding loops
            Item {
                implicitHeight: lbl1.implicitHeight
                width: pg.width

                Label {
                    id: lbl1
                    text: "<style>a:link { color: " + Material.accent + "; }</style>"+global.librarian().getStringFromRessource(":text/info_enroute.html")
                    textFormat: Text.RichText
                    linkColor: Material.accent
                    width: pg.width

                    wrapMode: Text.Wrap
                    topPadding: view.font.pixelSize*1
                    leftPadding: view.font.pixelSize*0.5
                    rightPadding: view.font.pixelSize*0.5
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }
        
        ScrollView {
            clip: true

            // The label that we really want to show is wrapped into an Item. This allows
            // to set implicitHeight, and thus compute the implicitHeight of the Dialog
            // without binding loops
            Item {
                implicitHeight: lbl2.implicitHeight
                width: pg.width

                Label {
                    id: lbl2
                    text: "<style>a:link { color: " + Material.accent + "; }</style>"+global.librarian().getStringFromRessource(":text/authors.html")
                    textFormat: Text.RichText // Link OK
                    width: pg.width
                    wrapMode: Text.Wrap
                    topPadding: view.font.pixelSize*1
                    leftPadding: view.font.pixelSize*0.5
                    rightPadding: view.font.pixelSize*0.5
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }

        ScrollView {
            clip: true

            // The label that we really want to show is wrapped into an Item. This allows
            // to set implicitHeight, and thus compute the implicitHeight of the Dialog
            // without binding loops
            Item {
                implicitHeight: lbl3.implicitHeight
                width: pg.width

                Label {
                    id: lbl3
                    text: "<style>a:link { color: " + Material.accent + "; }</style>"+global.librarian().getStringFromRessource(":text/info_license.html")
                    textFormat: Text.RichText
                    linkColor: Material.accent
                    width: pg.width
                    wrapMode: Text.Wrap
                    topPadding: view.font.pixelSize*1
                    leftPadding: view.font.pixelSize*0.5
                    rightPadding: view.font.pixelSize*0.5
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }
        }

    } // StackView
} // Page
