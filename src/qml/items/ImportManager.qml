/***************************************************************************
 *   Copyright (C) 2020-2021 by Stefan Kebekus                             *
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

import enroute 1.0
import "../pages"

Item {
    id: importManager

    property string filePath: ""
    property int fileFunction: MobileAdaptor.UnknownFunction

    Connections {
        target: global.mobileAdaptor
        function onOpenFileRequest(fileName, fileFunction) {
            view.raise()
            view.requestActivate()
            if (fileName === "")
                return
            if (fileFunction === MobileAdaptor.UnknownFunction) {
                errLbl.text = qsTr("The file type of the file <strong>%1</strong> could not be recognized.").arg(fileName)
                errorDialog.open()
                return
            }

            importManager.filePath = fileName
            importManager.fileFunction = fileFunction
            if (flightRoute.routeObjects.length > 0)
                importDialog.open()
            else
                importDialog.onAccepted()
      }
    } // Connections


    Dialog {
        id: importDialog

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(view.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(view.height-Qt.application.font.pixelSize, implicitHeight)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Import Flight Route?")

        Label {
            id: lbl

            width: importDialog.availableWidth

            text: qsTr("This will overwrite the current route. Once overwritten, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            global.mobileAdaptor.vibrateBrief()

            var errorString = ""

            if (importManager.fileFunction === MobileAdaptor.FlightRoute_GeoJSON)
                errorString = flightRoute.loadFromGeoJSON(importManager.filePath)
            if (importManager.fileFunction === MobileAdaptor.FlightRoute_GPX) {
                errorString = flightRoute.loadFromGpx(importManager.filePath, geoMapProvider)
            }

            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }
            if (!(stackView.currentItem instanceof FlightRouteEditor)) {
                stackView.pop()
                stackView.push("../pages/FlightRouteEditor.qml")
            }
            toast.doToast( qsTr("Flight route imported") )
        }

    } // importDialog

    Dialog {
        id: errorDialog

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(view.width-Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
        height: Math.min(view.height-Qt.application.font.pixelSize, implicitHeight)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        standardButtons: Dialog.Cancel
        modal: true

        title: qsTr("Error importing flight route")

        Label {
            id: errLbl

            width: importDialog.availableWidth

            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

    } // errorDialog

}
