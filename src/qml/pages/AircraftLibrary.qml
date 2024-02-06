/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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
    id: page
    title: qsTr("Aircraft Library")
    focus: true


    header: StandardHeader {}

    RowLayout {
        id: filterRow

        anchors.left: parent.left
        anchors.leftMargin: SafeInsets.left+font.pixelSize
        anchors.right: parent.right
        anchors.rightMargin: SafeInsets.right+font.pixelSize
        anchors.top: parent.top
        anchors.topMargin: page.font.pixelSize

        Label {
            Layout.alignment: Qt.AlignBaseline

            text: qsTr("Filter")
        }

        MyTextField {
            id: textInput

            Layout.alignment: Qt.AlignBaseline
            Layout.fillWidth: true
        }
    }

    Pane {

        anchors.top: filterRow.bottom
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        bottomPadding: SafeInsets.bottom
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: font.pixelSize

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
                    cptMenu.open()
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

            }
            }
        }

        DecoratedListView {
            id: wpList

            anchors.fill: parent

            clip: true

            model: Librarian.entries(Librarian.Aircraft, textInput.displayText)
            delegate: entryDelegate
            ScrollIndicator.vertical: ScrollIndicator {}
        }

        Label {
            anchors.fill: parent

            visible: (wpList.count === 0)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            leftPadding: font.pixelSize*2
            rightPadding: font.pixelSize*2

            textFormat: Text.StyledText
            wrapMode: Text.Wrap
            text: (textInput.text === "")
                  ? qsTr("<h3>Sorry!</h3><p>No aircraft available. To add a route here, chose 'Aircraft' from the main menu, and save the current aircraft to the library.</p>") //TODO: Fix text (route)
                  : qsTr("<h3>Sorry!</h3><p>No aircraft match your filter criteria.</p>")
        }

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

        DecoratedScrollView{
            id: sv

            anchors.fill: parent
            contentWidth: availableWidth // Disable horizontal scrolling

            clip: true

            Label {
                id: lbl
                width: fileError.availableWidth
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                onLinkActivated: Qt.openUrlExternally(link)
            }
        }

    }

    LongTextDialog {
        id: overwriteDialog

        title: qsTr("Overwrite Current Aircraft?")
        standardButtons: Dialog.No | Dialog.Yes

        text: qsTr("Loading the aircraft <strong>%1</strong> will overwrite the current aircraft. Once overwritten, the current aircraft cannot be restored.").arg(finalFileName)

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            page.openFromLibrary()
        }
        onRejected: {
            PlatformAdaptor.vibrateBrief()
            close()
        }
    }

    LongTextDialog {
        id: removeDialog

        title: qsTr("Remove from Device?")
        standardButtons: Dialog.No | Dialog.Yes

        text: qsTr("Once the aircraft <strong>%1</strong> is removed, it cannot be restored.").arg(page.finalFileName)

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
                Layout.fillWidth: true
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }

            MyTextField {
                id: renameName

                Layout.fillWidth: true
                focus: true

                onAccepted: renameDialog.onAccepted()
            }

        }

        footer: DialogButtonBox {
            Button {
                id: renameButton

                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
                enabled: (renameName.text !== "") && !(Librarian.exists(Librarian.Aircraft, renameName.text))
                flat: true
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
