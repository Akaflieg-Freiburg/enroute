/***************************************************************************
 *   Copyright (C) 2020-2025 by Stefan Kebekus                             *
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
import "../pages"

Item {
    id: importManager

    property string filePath: ""
    property int fileFunction: FileExchange.UnknownFunction

    required property var stackView
    required property var toast
    required property var view

    Component.onCompleted: {
        FileExchange.onGUISetupCompleted()
    }

    Connections {
        target: FileExchange

        function onOpenFileRequest(fileName, info, fileFunction) {
            importManager.view.raise()
            importManager.view.requestActivate()

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
            if (fileFunction === FileExchange.Image) {
                errLbl.text = qsTr("The file <strong>%1</strong> seems to contain an image without georeferencing information.").arg(fileName)
                errorDialog.open()
                return
            }
            if (fileFunction === FileExchange.TripKit) {
                importTripKitDialog.open()
                return
            }
            if (fileFunction === FileExchange.OpenAir) {
                openAirInfoLabel.text = info;
                importOpenAirDialog.open()
                return
            }
            if (fileFunction === FileExchange.ZipFile) {
                errLbl.text = qsTr("The file <strong>%1</strong> seems to contain an zip file without the data required in a tripkit.").arg(fileName)
                errorDialog.open()
                return
            }

            errLbl.text = qsTr("The file type of the file <strong>%1</strong> cannot be recognized.").arg(fileName)
            errorDialog.open()
            return
        }

        function onOpenVACRequest(vac) {
            importManager.view.raise()
            importManager.view.requestActivate()

            importVACDialog.vac = vac
            mapNameVAC.text = vac.name
            importVACDialog.open()
        }

        function onOpenWaypointRequest(waypoint) {
            Global.dialogLoader.active = false
            Global.dialogLoader.setSource("../dialogs/WaypointDescription.qml", {waypoint: waypoint})
            Global.dialogLoader.active = true
        }

        function onUnableToProcessText(txt) {
            Global.dialogLoader.active = false
            Global.dialogLoader.setSource("../dialogs/LongTextDialog.qml", {
                                              title: qsTr("Unable to import text item"),
                                              text: "<p>" + qsTr("The text item could not be interpreted.") + "</p>"
                                                    + "<p><strong>" + txt + "</strong></p>",
                                              standardButtons: Dialog.Ok
                                          })
            Global.dialogLoader.active = true
        }
    }

    Connections {
        target: VACLibrary

        function onImportTripKitStatus(percent) {
            if (percent < 1.0) {
                pbar.value = percent
                importTripKitWaitDialog.open()
            } else
                importTripKitWaitDialog.close()
            return
        }
    }

    LongTextDialog {
        id: chooseFRorWPDialog

        title: qsTr("Import Waypoint Data")
        text: qsTr("The file contains a list of waypoints. Import as a flight route or add to the waypoint library?")

        standardButtons: Dialog.Abort
        modal: true

        footer: DialogButtonBox {

            Button {
                text: qsTr("Route")
                flat: true

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

            Button {
                text: qsTr("Library")
                flat: true

                onClicked: {
                    PlatformAdaptor.vibrateBrief()
                    importManager.fileFunction = FileExchange.WaypointLibrary
                    chooseFRorWPDialog.close()
                    importWPLibraryDialog.open()
                    }
            }
        }

        onRejected: chooseFRorWPDialog.close()
    }

    CenteringDialog {
        id: importOpenAirDialog

        title: qsTr("Import Airspace Data")
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

            MyTextField {
                id: mapNameOpenAir

                Layout.fillWidth: true
                focus: true

                onDisplayTextChanged: importOpenAirDialog.standardButton(DialogButtonBox.Ok).enabled = (displayText !== "")

                onAccepted: {
                    if (mapNameOpenAir.text === "")
                        return
                    importOpenAirDialog.accept()
                }
            }

            Label {
                id: openAirInfoLabel
                Layout.fillWidth: true
                visible: text !== ""

                wrapMode: Text.Wrap
                textFormat: Text.RichText
            }
        }

        onAboutToShow: {
            mapNameOpenAir.text = ""
            importOpenAirDialog.standardButton(DialogButtonBox.Ok).enabled = false
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()

            var errorString = DataManager.importOpenAir(importManager.filePath, mapNameOpenAir.text)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }
            importManager.toast.doToast( qsTr("Airspace data imported") )
        }
    }

    CenteringDialog {
        id: importVACDialog

        title: qsTr("Import Visual Approach Chart")
        standardButtons: Dialog.Ok | Dialog.Cancel
        modal: true

        property var vac

        ColumnLayout {
            anchors.fill: parent

            // Delays evaluation and prevents binding loops
            Binding on implicitHeight {
                delayed: true    // Prevent intermediary values from being assigned
            }

            Label {
                Layout.fillWidth: true

                text: qsTr("Enter a name for this chart. Existing approach charts with the same name will be overwritten.")
                wrapMode: Text.Wrap
            }

            MyTextField {
                id: mapNameVAC

                Layout.fillWidth: true
                focus: true

                onDisplayTextChanged: importVACDialog.standardButton(DialogButtonBox.Ok).enabled = (displayText !== "")

                onAccepted: {
                    if (mapNameVAC.text === "")
                        return
                    importVACDialog.accept()
                }
            }
        }

        onAboutToShow: importVACDialog.standardButton(DialogButtonBox.Ok).enabled = mapNameVAC.text !== ""

        onAccepted: {
            PlatformAdaptor.vibrateBrief()

            vac.name = mapNameVAC.text
            var errorString = VACLibrary.importVAC(vac)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }
            importManager.toast.doToast( qsTr("Visual approach chart data imported") )
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

            MyTextField {
                id: mapNameRaster

                Layout.fillWidth: true
                focus: true

                onDisplayTextChanged: importRasterMapDialog.standardButton(DialogButtonBox.Ok).enabled = (displayText !== "")

                onAccepted: {
                    if (mapNameRaster.text === "")
                        return
                    importRasterMapDialog.accept()
                }
            }
        }

        onAboutToShow: {
            mapNameRaster.text = ""
            importRasterMapDialog.standardButton(DialogButtonBox.Ok).enabled = false
        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()

            var errorString = DataManager.import(importManager.filePath, mapNameRaster.text)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }
            importManager.toast.doToast( qsTr("Raster map imported") )
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

            MyTextField {
                id: mapNameVector

                Layout.fillWidth: true
                focus: true

                onDisplayTextChanged: importRasterMapDialog.standardButton(DialogButtonBox.Ok).enabled = (displayText !== "")

                onAccepted: {
                    if (mapNameVector.text === "")
                        return
                    importVectorMapDialog.accept()
                }
            }

            Label {
                Layout.fillWidth: true
                visible: DataManager.baseMapsRaster.hasFile
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

            var errorString = DataManager.import(importManager.filePath, mapNameVector.text)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }
            importManager.toast.doToast( qsTr("Vector map imported") )
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

                text: qsTr("Skip over waypoints that already exist in the library")
            }

        }

        onAccepted: {
            PlatformAdaptor.vibrateBrief()

            var errorString = WaypointLibrary.import(importManager.filePath, skip.checked)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }

            if (!(importManager.stackView.currentItem instanceof WaypointLibraryPage)) {
                importManager.stackView.pop()
                importManager.stackView.push("../pages/WaypointLibraryPage.qml")
            }
            toast.doToast( qsTr("Waypoints imported") )
        }
    }

    LongTextDialog {
        id: importFlightRouteDialog

        title: qsTr("Import Flight Route?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        text: qsTr("This will overwrite the current route. Once overwritten, the current flight route cannot be restored.")

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
            if (!(importManager.stackView.currentItem instanceof FlightRouteEditor)) {
                importManager.stackView.pop()
                importManager.stackView.push("../pages/FlightRouteEditor.qml")
            }
            toast.doToast( qsTr("Flight route imported") )
        }
    }

    LongTextDialog {
        id: importTripKitDialog

        title: qsTr("Import Trip Kit?")
        standardButtons: Dialog.No | Dialog.Yes
        modal: true

        text: qsTr("This might overwrite some approach charts.")

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            importTripKitDialog.close()

            var errorString = VACLibrary.importTripKit(importManager.filePath)
            if (errorString !== "") {
                errLbl.text = errorString
                errorDialog.open()
                return
            }
            importManager.toast.doToast( qsTr("Trip kit imported") )
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

    CenteringDialog {
        id: importTripKitWaitDialog
        title: qsTr("Stand by")

        modal: true
        closePolicy: Popup.NoAutoClose

        ColumnLayout {
            anchors.fill: parent

            Label {
                id: txtLbl
                Layout.fillWidth: true

                text: qsTr("Extracting and converting files from the trip kit. Please do not interrupt or close the app.")
                wrapMode: Text.Wrap
                textFormat: Text.StyledText
            }

            Item {
                height: txtLbl.font.pixelSize
            }

            ProgressBar {
                id: pbar
                Layout.fillWidth: true
                value: 0.0
            }

        }
    }
}
