/***************************************************************************
 *   Copyright (C) 2020-2022 by Stefan Kebekus                             *
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
import enroute 1.0
import "../dialogs"
import "../pages"

Item {
    id: importManager

    property string filePath: ""
    property int fileFunction: FileExchange.UnknownFunction

    Connections {
        target: FileExchange

        function onOpenFileRequest(fileName, fileFunction) {
            view.raise()
            view.requestActivate()

            importManager.filePath = fileName
            importManager.fileFunction = fileFunction
            if (fileName === "")
                return

            if (fileFunction === FileExchange.WaypointLibrary) {
                importWPLibraryDialog.open()
                return
            }
            if (fileFunction === FileExchange.FlightRouteOrWaypointLibrary) {
                chooseFRorWPDialog.open()
                return
            }

            if (fileFunction === FileExchange.VectorMap) {
                importVectorMapDialog.open()
                return
            }
            if (fileFunction === FileExchange.RasterMap) {
                importRasterMapDialog.open()
                return
            }
            if (fileFunction === FileExchange.FlightRoute) {
                if (Navigator.flightRoute.size > 0)
                    importFlightRouteDialog.open()
                else
                    importFlightRouteDialog.onAccepted()
                return
            }

            errLbl.text = qsTr("The file type of the file <strong>%1</strong> cannot be recognized.").arg(fileName)
            errorDialog.open()
            return
        }
    }

    CenteringDialog {
        id: chooseFRorWPDialog

        title: qsTr("Import Waypoint Data")
        standardButtons: Dialog.Abort
        modal: true

        ColumnLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true

                text: qsTr("The file contains a list of waypoints. Import as a flight route or add to the waypoint library?")
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }

        }

        footer: DialogButtonBox {

            ToolButton {
                text: qsTr("Route")

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    importManager.fileFunction = FileExchange.FlightRoute
                    chooseFRorWPDialog.close()
                    if (Navigator.flightRoute.size > 0)
                        importFlightRouteDialog.open()
                    else
                        importFlightRouteDialog.onAccepted()
                }
            }

            ToolButton {
                    text: qsTr("Library")

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        importManager.fileFunction = FileExchange.WaypointLibrary
                        chooseFRorWPDialog.close()
                        importWPLibraryDialog.open()
                    }
                }

            onRejected: close()
        }

    }

    CenteringDialog {
        id: importRasterMapDialog

        title: qsTr("Import Raster Map")
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true

        ColumnLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true

                text: qsTr("Enter a name for this map.")
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }

            TextField {
                id: mapNameRaster

                Layout.fillWidth: true
                focus: true
                placeholderText: qsTr("Map Name")

                onDisplayTextChanged: importRasterMapDialog.standardButton(DialogButtonBox.Ok).enabled = (displayText !== "")

                onAccepted: {
                    if (mapNameRaster.text === "")
                        return
                    importRasterMapDialog.accept()
                }
            }

            Label {
                Layout.fillWidth: true
                visible: global.dataManager().baseMapsVector.hasFile
                text: qsTr("To avoid conflicts between raster and vector maps, all vector maps will be uninstalled.")

                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }
        }

        onAboutToShow: {
            mapNameRaster.text = ""
            importRasterMapDialog.standardButton(DialogButtonBox.Ok).enabled = false
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()

            var errorString = global.dataManager().import(importManager.filePath, mapNameRaster.text)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }
            toast.doToast( qsTr("Raster map imported") )
        }
    }

    CenteringDialog {
        id: importVectorMapDialog

        title: qsTr("Import Vector Map")
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true

        ColumnLayout {
            anchors.fill: parent

            Label {
                Layout.fillWidth: true

                text: qsTr("Enter a name for this map.")
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }

            TextField {
                id: mapNameVector

                Layout.fillWidth: true
                focus: true
                placeholderText: qsTr("Map Name")

                onDisplayTextChanged: importRasterMapDialog.standardButton(DialogButtonBox.Ok).enabled = (displayText !== "")

                onAccepted: {
                    if (mapNameVector.text === "")
                        return
                    importVectorMapDialog.accept()
                }
            }

            Label {
                Layout.fillWidth: true
                visible: global.dataManager().baseMapsRaster.hasFile
                text: qsTr("To avoid conflicts between raster and vector maps, all raster maps will be uninstalled.")

                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }
        }

        onAboutToShow: {
            mapNameVector.text = ""
            importRasterMapDialog.standardButton(DialogButtonBox.Ok).enabled = false
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()

            var errorString = global.dataManager().import(importManager.filePath, mapNameVector.text)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }
            toast.doToast( qsTr("Vector map imported") )
        }

    }

    CenteringDialog {
        id: importWPLibraryDialog

        title: qsTr("Import Waypoint Library")

        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true

        ColumnLayout {
            anchors.fill: parent

            WordWrappingCheckDelegate {
                id: skip
                Layout.fillWidth: true

                text: qsTr("Skip over waypoint that already exist in the library")
            }

        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()

            var errorString = global.waypointLibrary().import(importManager.filePath, skip.checked)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }

            if (!(stackView.currentItem instanceof WaypointLibrary)) {
                stackView.pop()
                stackView.push("../pages/WaypointLibrary.qml")
            }
            toast.doToast( qsTr("Waypoints imported") )
        }
    }

    CenteringDialog {
        id: importFlightRouteDialog

        title: qsTr("Import Flight Route?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        Label {
            id: lbl

            width: importFlightRouteDialog.availableWidth

            text: qsTr("This will overwrite the current route. Once overwritten, the current flight route cannot be restored.")
            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()

            var errorString = ""

            if (importManager.fileFunction === FileExchange.FlightRoute)
                errorString = Navigator.flightRoute.load(importManager.filePath)

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
    }

    CenteringDialog {
        id: errorDialog

        standardButtons: Dialog.Cancel
        modal: true

        title: qsTr("Data Import Error")

        Label {
            id: errLbl

            width: importFlightRouteDialog.availableWidth

            wrapMode: Text.Wrap
            textFormat: Text.StyledText
        }
    }

}
