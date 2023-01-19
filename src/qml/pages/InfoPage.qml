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

import akaflieg_freiburg.enroute
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
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right

        currentIndex: sv.currentIndex

        TabButton { text: "Enroute" }
        TabButton { text: qsTr("Authors") }
        TabButton { text: qsTr("License") }
        TabButton { text: qsTr("System") }
        Material.elevation: 3
    }

    SwipeView {
        id: sv

        anchors.top: bar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.bottomMargin: SafeInsets.bottom
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right

        clip: true
        currentIndex: bar.currentIndex
        
        ScrollView {
            contentWidth: availableWidth // Disable horizontal scrolling
            clip: true

            Label {
                id: lbl1
                text: "<style>a:link { color: " + Material.accent + "; }</style>"+Librarian.getStringFromRessource(":text/info_enroute.html")
                textFormat: Text.RichText
                linkColor: Material.accent
                width: sv.availableWidth

                wrapMode: Text.Wrap
                topPadding: font.pixelSize*1
                leftPadding: font.pixelSize*0.5
                rightPadding: font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }
        
        ScrollView {
            contentWidth: availableWidth // Disable horizontal scrolling
            clip: true

            Label {
                id: lbl2
                text: "<style>a:link { color: " + Material.accent + "; }</style>"+Librarian.getStringFromRessource(":text/authors.html")
                textFormat: Text.RichText // Link OK
                width: sv.availableWidth
                wrapMode: Text.Wrap
                topPadding: font.pixelSize*1
                leftPadding: font.pixelSize*0.5
                rightPadding: font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        ScrollView {
            contentWidth: availableWidth // Disable horizontal scrolling
            clip: true

            Label {
                id: lbl3
                text: "<style>a:link { color: " + Material.accent + "; }</style>"+Librarian.getStringFromRessource(":text/info_license.html")
                textFormat: Text.RichText
                linkColor: Material.accent
                width: sv.availableWidth
                wrapMode: Text.Wrap
                topPadding: font.pixelSize*1
                leftPadding: font.pixelSize*0.5
                rightPadding: font.pixelSize*0.5
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

        ColumnLayout {

            ScrollView {
                Layout.fillHeight: true
                Layout.preferredWidth: sv.availableWidth
                contentWidth: availableWidth // Disable horizontal scrolling
                clip: true

                Label {
                    text: "<style>a:link { color: " + Material.accent + "; }</style>" + Librarian.systemInfo()
                    textFormat: Text.RichText
                    linkColor: Material.accent
                    width: sv.availableWidth
                    wrapMode: Text.Wrap
                    topPadding: font.pixelSize*1
                    leftPadding: font.pixelSize*0.5
                    rightPadding: font.pixelSize*0.5
                    onLinkActivated: Qt.openUrlExternally(link)
                }
            }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Share Info")
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    var errorString = FileExchange.shareContent(Librarian.systemInfo(), "application/text", "EnrouteSystemInformation.txt")
                    if (errorString === "abort") {
                        toast.doToast(qsTr("Aborted"))
                        return
                    }
                    if (errorString !== "") {
                        shareErrorDialogLabel.text = errorString
                        shareErrorDialog.open()
                        return
                    }
                    if (Qt.platform.os === "android")
                        toast.doToast(qsTr("System Info Shared"))
                    else
                        toast.doToast(qsTr("System Info Exported"))

                }
            }
        }


    } // StackView
} // Page
