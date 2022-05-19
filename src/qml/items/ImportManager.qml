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
import QtQuick.Layouts 1.15

import enroute 1.0
import "../pages"

Item {
    id: importManager

    property string filePath: ""
    property int fileFunction: MobileAdaptor.UnknownFunction

    Connections {
        target: global.mobileAdaptor()

        function onOpenFileRequest(fileName, fileFunction) {
            view.raise()
            view.requestActivate()
            importManager.filePath = fileName
            importManager.fileFunction = fileFunction
            if (fileName === "")
                return

            if (fileFunction === MobileAdaptor.VectorMap) {
                errLbl.text = qsTr("The file <strong>%1</strong> contains a vector map. <strong>Enroute Flight Navigation</strong> can only import raster maps.").arg(fileName)
                errorDialog.open()
                return
            }
            if (fileFunction === MobileAdaptor.RasterMap) {
                importRasterMapDialog.open()
                return
            }
            if ((fileFunction === MobileAdaptor.FlightRoute_GPX) || (fileFunction === MobileAdaptor.FlightRoute_GeoJSON)) {
                if (global.navigator().flightRoute.size > 0)
                    importFlightRouteDialog.open()
                else
                    importFlightRouteDialog.onAccepted()
                return
            }

            errLbl.text = qsTr("The file type of the file <strong>%1</strong> cannot be recognized.").arg(fileName)
            errorDialog.open()
            return
        }
    } // Connections


    Dialog {
        id: importRasterMapDialog

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(view.width-view.font.pixelSize, 40*view.font.pixelSize)
        height: Math.min(view.height-view.font.pixelSize, implicitHeight)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 5.15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Import Raster Map")

        ColumnLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true

                text: qsTr("Enter a name for this map.")
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }

            TextField {
                id: mapName

                Layout.fillWidth: true
                focus: true
                placeholderText: qsTr("Map Name")

                onDisplayTextChanged: importRasterMapDialog.standardButton(DialogButtonBox.Ok).enabled = (displayText !== "")

                onAccepted: {
                    if (mapName.text === "")
                        return
                    importRasterMapDialog.accept()
                }
            }

            CheckBox {
                id: removeFile
                Layout.fillWidth: true
                text: qsTr("Remove file after import")
            }

            Label {
                Layout.fillWidth: true
                visible: global.dataManager().baseMapsVector.hasFile
                text: qsTr("To avoid conflicts between raster and vector maps, all vector maps will be uninstalled.")

                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }
        }

        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true

        onAboutToShow: {
            mapName.text = ""
            removeFile.checked = false
            importRasterMapDialog.standardButton(DialogButtonBox.Ok).enabled = false
        }

        onAccepted: {
            global.mobileAdaptor().vibrateBrief()

            var errorString = global.dataManager().import(importManager.filePath, mapName.text, removeFile.checked)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }
            toast.doToast( qsTr("Raster map imported") )
        }

    } // importDialog


    Dialog {
        id: importFlightRouteDialog

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(view.width-view.font.pixelSize, 40*view.font.pixelSize)
        height: Math.min(view.height-view.font.pixelSize, implicitHeight)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 5.15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        title: qsTr("Import Flight Route?")

        Label {
            id: lbl

            width: importFlightRouteDialog.availableWidth

            text: qsTr("This will overwrite the current route. Once overwritten, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        onAccepted: {
            global.mobileAdaptor().vibrateBrief()

            var errorString = ""

            if (importManager.fileFunction === MobileAdaptor.FlightRoute_GeoJSON)
                errorString = global.navigator().flightRoute.loadFromGeoJSON(importManager.filePath)
            if (importManager.fileFunction === MobileAdaptor.FlightRoute_GPX) {
                errorString = global.navigator().flightRoute.loadFromGpx(importManager.filePath, global.geoMapProvider())
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
        width: Math.min(view.width-view.font.pixelSize, 40*view.font.pixelSize)
        height: Math.min(view.height-view.font.pixelSize, implicitHeight)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        standardButtons: Dialog.Cancel
        modal: true

        title: qsTr("Data import error")

        Label {
            id: errLbl

            width: importFlightRouteDialog.availableWidth

            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

    } // errorDialog

}
