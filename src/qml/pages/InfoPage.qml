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

import akaflieg_freiburg.enroute
import "../dialogs"
import "../items"

Page {
    id: pg
    title: qsTr("About EFN")

    required property var stackView
    required property var toast

    header: ColoredToolBar {

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
                pg.stackView.pop()
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: pg.title
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
                helpDialog.open()
            }
        }

    }


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
        
        DecoratedScrollView {
            contentWidth: availableWidth // Disable horizontal scrolling
            clip: true

            Label {
                id: lbl1
                text: Librarian.getStringFromRessource(":text/info_enroute.html")
                textFormat: Text.RichText
                width: sv.availableWidth

                wrapMode: Text.Wrap
                topPadding: font.pixelSize*1
                leftPadding: font.pixelSize*0.5
                rightPadding: font.pixelSize*0.5
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }
        }
        
        DecoratedScrollView {
            contentWidth: availableWidth // Disable horizontal scrolling
            clip: true

            Label {
                id: lbl2
                text: Librarian.getStringFromRessource(":text/authors.html")
                textFormat: Text.RichText // Link OK
                width: sv.availableWidth
                wrapMode: Text.Wrap
                topPadding: font.pixelSize*1
                leftPadding: font.pixelSize*0.5
                rightPadding: font.pixelSize*0.5
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }
        }

        DecoratedScrollView {
            contentWidth: availableWidth // Disable horizontal scrolling
            clip: true

            Label {
                id: lbl3
                text: Librarian.getStringFromRessource(":text/info_license.html")
                textFormat: Text.RichText
                width: sv.availableWidth
                wrapMode: Text.Wrap
                topPadding: font.pixelSize*1
                leftPadding: font.pixelSize*0.5
                rightPadding: font.pixelSize*0.5
                onLinkActivated: (link) => Qt.openUrlExternally(link)
            }
        }

        ColumnLayout {

            DecoratedScrollView {
                Layout.fillHeight: true
                Layout.preferredWidth: sv.availableWidth
                contentWidth: availableWidth // Disable horizontal scrolling
                clip: true

                Label {
                    id: sysInfoLabel
                    text: {
                        // Mention time, so this property get updated every minute
                        Clock.time
                        return PlatformAdaptor.systemInfo()
                    }
                    textFormat: Text.RichText
                    width: sv.availableWidth
                    wrapMode: Text.Wrap
                    topPadding: font.pixelSize*1
                    leftPadding: font.pixelSize*0.5
                    rightPadding: font.pixelSize*0.5
                    onLinkActivated: (link) => Qt.openUrlExternally(link)
                }
            }

            Rectangle { Layout.preferredHeight: sv.font.pixelSize/2 }

            Button {
                Layout.alignment: Qt.AlignHCenter
                text: qsTr("Share Info")
                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    var errorString = FileExchange.shareContent(sysInfoLabel.text, "application/text", "EnrouteSystemInformation.txt")
                    if (errorString === "abort") {
                        pg.toast.doToast(qsTr("Aborted"))
                        return
                    }
                    if (errorString !== "") {
                        shareErrorDialog.text = errorString
                        shareErrorDialog.open()
                        return
                    }
                    if (Qt.platform.os === "android")
                        pg.toast.doToast(qsTr("System Info Shared"))
                    else
                        pg.toast.doToast(qsTr("System Info Exported"))

                }
            }

            Rectangle { Layout.preferredHeight: sv.font.pixelSize/2 }
        }

    }

    LongTextDialog {
        id: shareErrorDialog

        title: qsTr("Error Exporting Data…")

        standardButtons: Dialog.Ok
    }
    LongTextDialog {
        id: helpDialog

        title: qsTr("About EFN")
        text: "<p>"+qsTr("This page presents four tabs with information about the app, its authors, the software license, and the current system.")+"</p>"
              +"<p>"+qsTr("System information can be helpful to the developers when you report a bug. The button 'Share Info' at the bottom of the 'System' tab can be used to forward this information to the developers.")+"</p>"

        standardButtons: Dialog.Ok
    }
}
