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

import enroute 1.0

Dialog {
    id: dlg
    title: qsTr("Load Flight Route from Library…")
    modal: true
    focus: true

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: parent.height-2*Qt.application.font.pixelSize

    // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
    // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
    parent: Overlay.overlay
    x: (parent.width-width)/2.0
    y: (parent.height-height)/2.0


    implicitHeight: height

    standardButtons: DialogButtonBox.Cancel

    Component {
        id: fileDelegate

        ItemDelegate {
            id: idel
            text: modelData
            icon.source: "/icons/material/ic_directions.svg"

            anchors.left: parent.left
            anchors.right: parent.right

            onClicked: {
                global.mobileAdaptor().vibrateBrief()
                finalFileName = modelData
                dlg.close()
                if (global.navigator().flightRoute.size > 0)
                    overwriteDialog.open()
                else
                    openFromLibrary()
            }
        }

    } // fileDelegate

    ColumnLayout {
        anchors.fill: parent

        Label {
            Layout.fillWidth: true

            text: qsTr("Choose a flight route from the list below.")
            color: Material.accent
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        TextField {
            id: filterName

            Layout.fillWidth: true
            placeholderText: qsTr("Filter Flight Route Names")

            onTextChanged: lView.model = library.flightRoutes(text)

            onAccepted: {
                if (text.length === 0)
                    return
                if (lView.model.length === 0)
                    return

                global.mobileAdaptor().vibrateBrief()
                finalFileName = lView.model[0]
                dlg.close()
                if (global.navigator().flightRoute.size > 0)
                    overwriteDialog.open()
                else
                    openFromLibrary()
            }
        }

        ListView {
            id: lView

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true
            model: global.global.librarian()().flightRoutes(filterName.displayText)
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: fileDelegate
        }

    } // ColumnLayout

    onRejected: {
        global.mobileAdaptor().vibrateBrief()
        close()
    }

    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function openFromLibrary() {
        var errorString = global.navigator().flightRoute.loadFromGeoJSON(global.global.librarian()().flightRouteFullPath(finalFileName))
        if (errorString !== "") {
            lbl.text = errorString
            fileError.open()
        }
    }

    Dialog {
        id: fileError

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        anchors.centerIn: parent
        parent: Overlay.overlay

        modal: true
        title: qsTr("An error occurred…")
        standardButtons: Dialog.Ok

        ScrollView{
            id: sv
            anchors.fill: parent

            contentHeight: lbl.height
            contentWidth: fileError.availableWidth

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
            ScrollBar.vertical.policy: (height < contentHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded

            clip: true

            Label {
                id: lbl
                width: dlg.availableWidth
                textFormat: Text.StyledText
                wrapMode: Text.Wrap
                onLinkActivated: Qt.openUrlExternally(link)
            } // Label
        } // ScrollView

    }

    Dialog {
        id: overwriteDialog
        anchors.centerIn: parent
        parent: Overlay.overlay

        title: qsTr("Overwrite current flight route?")

        // Width is chosen so that the dialog does not cover the parent in full, height is automatic
        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(parent.height-Qt.application.font.pixelSize, implicitHeight)

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Loading the route <strong>%1</strong> will overwrite the current route. Once overwritten, the current flight route cannot be restored.").arg(finalFileName)
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            global.mobileAdaptor().vibrateBrief()
            dlg.openFromLibrary()
        }
        onRejected: {
            global.mobileAdaptor().vibrateBrief()
            close()
            dlg.open()
        }

    }

} // Dialog
