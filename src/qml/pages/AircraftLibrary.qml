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
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import enroute 1.0

import "../dialogs"
import "../items"

Page {
    id: page
    title: qsTr("Aircraft Library")
    focus: true


    header: StandardHeader {}

    TextField {
        id: textInput

        anchors.right: parent.right
        anchors.rightMargin: font.pixelSize*2.0
        anchors.left: parent.left
        anchors.leftMargin: font.pixelSize*2.0
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right

        placeholderText: qsTr("Filter Aircraft Names")
        font.pixelSize: page.font.pixelSize*1.5
    }

    Component {
        id: entryDelegate

        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            Layout.fillWidth: true
            height: iDel.height

            SwipeToDeleteDelegate {
                id: iDel
                Layout.fillWidth: true

                text: modelData
                icon.source: "/icons/material/ic_airplanemode_active.svg"

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    finalFileName = modelData
                    if (Navigator.flightRoute.size > 0)
                        overwriteDialog.open()
                    else
                        openFromLibrary()
                }

                swipe.onCompleted: {
                    PlatformAdaptor.vibrateBrief()
                    finalFileName = modelData
                    removeDialog.open()
                }

            }

            ToolButton {
                id: cptMenuButton

                icon.source: "/icons/material/ic_more_horiz.svg"

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    cptMenu.popup()
                }

                AutoSizingMenu {
                    id: cptMenu

                    Action {
                        id: renameAction
                        text: qsTr("Rename…")
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            finalFileName = modelData
                            renameName.text = modelData
                            renameDialog.open()
                        }

                    } // renameAction

                    Action {
                        id: removeAction
                        text: qsTr("Remove…")
                        onTriggered: {
                            PlatformAdaptor.vibrateBrief()
                            finalFileName = modelData
                            removeDialog.open()
                        }
                    } // removeAction
                } // AutoSizingMenu

            } // ToolButton

        }

    }

    DecoratedListView {
        id: wpList
        anchors.top: textInput.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        leftMargin: SafeInsets.left
        rightMargin: SafeInsets.right
        bottomMargin: SafeInsets.bottom

        clip: true

        model: Librarian.entries(Librarian.Aircraft, textInput.displayText)
        delegate: entryDelegate
        ScrollIndicator.vertical: ScrollIndicator {}
    }

    Label {
        anchors.fill: wpList
        anchors.topMargin: font.pixelSize*2

        visible: (wpList.count === 0)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        leftPadding: font.pixelSize*2
        rightPadding: font.pixelSize*2

        textFormat: Text.StyledText
        wrapMode: Text.Wrap
        text: (textInput.text === "")
              ? qsTr("<h3>Sorry!</h3><p>No aircraft available. To add a route here, chose 'Aircraft' from the main menu, and save the current aircraft to the library.</p>")
              : qsTr("<h3>Sorry!</h3><p>No aircraft match your filter criteria.</p>")
    }


    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function openFromLibrary() {
        var acft = Navigator.aircraft.clone() // Get a copy of the current aircraft that we can modify and write back
        var errorString = acft.loadFromJSON(Librarian.fullPath(Librarian.Aircraft, finalFileName))
        if (errorString !== "") {
            lbl.text = errorString
            fileError.open()
            return
        }
        Navigator.aircraft = acft
        toast.doToast( qsTr("Loading aircraft <strong>%1</strong>").arg(finalFileName) )
        stackView.pop()
    }

    function reloadFlightRouteList() {
        var cache = textInput.text
        textInput.text = textInput.text+"XXXXX"
        textInput.text = cache
    }

    CenteringDialog {
        id: fileError

        modal: true
        title: qsTr("An Error Occurred…")
        standardButtons: Dialog.Ok

        ScrollView{
            id: sv

            anchors.fill: parent
            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            Label {
                id: lbl
                width: fileError.availableWidth
                textFormat: Text.StyledText
                linkColor: Material.accent
                wrapMode: Text.Wrap
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

    }

    CenteringDialog {
        id: overwriteDialog

        title: qsTr("Overwrite Current Aircraft?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Loading the aircraft <strong>%1</strong> will overwrite the current aircraft. Once overwritten, the current aircraft cannot be restored.").arg(finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            page.openFromLibrary()
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
            close()
        }
    }

    CenteringDialog {
        id: removeDialog

        title: qsTr("Remove from Device?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            width: removeDialog.availableWidth

            text: qsTr("Once the aircraft <strong>%1</strong> is removed, it cannot be restored.").arg(page.finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            Librarian.remove(Librarian.Aircraft, page.finalFileName)
            page.reloadFlightRouteList()
            toast.doToast(qsTr("Aircraft removed from device"))
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
            page.reloadFlightRouteList() // Re-display aircraft that have been swiped out
            close()
        }

    }

    CenteringDialog {
        id: renameDialog

        title: qsTr("Rename Aircraft")
        standardButtons: Dialog.Cancel
        modal: true

        ColumnLayout {
            width: renameDialog.availableWidth

            Label {
                Layout.preferredWidth: overwriteDialog.availableWidth

                text: qsTr("Enter new name for the aircraft <strong>%1</strong>.").arg(finalFileName)
                color: Material.primary
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }

            TextField {
                id: renameName

                Layout.fillWidth: true
                focus: true

                placeholderText: qsTr("New Aircraft Name")

                onAccepted: renameDialog.onAccepted()
            }

        }

        footer: DialogButtonBox {
            ToolButton {
                id: renameButton

                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                enabled: (renameName.text !== "") && !(Librarian.exists(Librarian.Aircraft, renameName.text))
                text: qsTr("Rename")
            }
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            if ((renameName.text !== "") && !Librarian.exists(Librarian.Aircraft, renameName.text)) {
                Librarian.rename(Librarian.Aircraft, finalFileName, renameName.text)
                page.reloadFlightRouteList()
                close()
                toast.doToast(qsTr("Aircraft renamed"))
            }
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
            close()
        }
    }

} // Page
