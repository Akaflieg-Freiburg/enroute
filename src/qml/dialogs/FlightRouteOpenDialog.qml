/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Controls.Material 2.14
import QtQuick.Layouts 1.14

import enroute 1.0

Dialog {
    id: dlg
    title: qsTr("Load Flight Route from Library…")
    modal: true
    focus: true

    // Size is chosen so that the dialog does not cover the parent in full
    width: Math.min(parent.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
    height: parent.height-2*Qt.application.font.pixelSize
    implicitHeight: height

    standardButtons: DialogButtonBox.Cancel

    Component {
        id: fileDelegate

        ItemDelegate {
            id: idel
            text: modelData
            icon.source: "/icons/material/ic_directions.svg"
            icon.color: "transparent"

            anchors.left: parent.left
            anchors.right: parent.right

            onClicked: {
                MobileAdaptor.vibrateBrief()
                finalFileName = modelData
                dlg.close()
                if (flightRoute.routeObjects.length > 0)
                    overwriteDialog.open()
                else
                    openFromLibrary()
            }
        }

    } // fileDelegate

    ColumnLayout {
        anchors.fill: parent

        Label {
            width: overwriteDialog.availableWidth

            text: qsTr("Choose a flight route from the list below.")
            color: Material.primary
            wrapMode: Text.Wrap
            textFormat: Text.RichText
        }

        TextField {
            id: filterName

            Layout.fillWidth: true
            placeholderText: "Filter Flight Route Names"

            onTextChanged: lView.model = library.flightRoutes(text)

            onAccepted: {
                if (text.length === 0)
                    return
                if (lView.model.length === 0)
                    return

                MobileAdaptor.vibrateBrief()
                finalFileName = lView.model[0]
                dlg.close()
                if (flightRoute.routeObjects.length > 0)
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
            model: librarian.flightRoutes(filterName.displayText)
            ScrollIndicator.vertical: ScrollIndicator {}

            delegate: fileDelegate
        }

    } // ColumnLayout

    onRejected: {
        MobileAdaptor.vibrateBrief()
        close()
    }

    Connections {
        target: sensorGesture
        onDetected: close()
    }

    // This is the name of the file that openFromLibrary will open
    property string finalFileName;

    function openFromLibrary() {
        var errorString = flightRoute.load(librarian.flightRouteFullPath(finalFileName))
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

            ScrollBar.horizontal.policy: ScrollBar.AlwaysOff

            // The visibility behavior of the vertical scroll bar is a little complex.
            // The following code guarantees that the scroll bar is shown initially. If it is not used, it is faded out after half a second or so.
            ScrollBar.vertical.policy: (height < lbl.implicitHeight) ? ScrollBar.AlwaysOn : ScrollBar.AsNeeded
            ScrollBar.vertical.interactive: false

            clip: true

            // The Label that we really want to show is wrapped into an Item. This allows
            // to set implicitHeight, and thus compute the implicitHeight of the Dialog
            // without binding loops
            Item {
                implicitHeight: lbl.implicitHeight
                width: dlg.availableWidth

                Label {
                    id: lbl
                    width: dlg.availableWidth
                    textFormat: Text.RichText
                    horizontalAlignment: Text.AlignJustify
                    wrapMode: Text.Wrap
                    onLinkActivated: Qt.openUrlExternally(link)
                } // Label
            } // Item
        } // ScrollView

        Connections {
            target: sensorGesture
            onDetected: close()
        }
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
            textFormat: Text.RichText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            MobileAdaptor.vibrateBrief()
            dlg.openFromLibrary()
        }
        onRejected: {
            MobileAdaptor.vibrateBrief()
            close()
            dlg.open()
        }

        Connections {
            target: sensorGesture
            onDetected: close()
        }
    }

} // Dialog
