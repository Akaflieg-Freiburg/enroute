/***************************************************************************
 *   Copyright (C) 2020-2024 by Stefan Kebekus                             *
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
            if (fileFunction === FileExchange.VAC) {
                mapNameVAC.text = info;
                importVACDialog.open()
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

        function onOpenWaypointRequest(waypoint) {
            Global.dialogLoader.active = false
            Global.dialogLoader.setSource("../dialogs/WaypointDescription.qml", {waypoint: waypoint})
            Global.dialogLoader.active = true
        }

        function onResolveURL(url, site) {
            PlatformAdaptor.vibrateBrief()
            if (GlobalSettings.alwaysOpenExternalWebsites === true) {
                stackView.push("../pages/URLResolver.qml", {mapURL: url})
                return
            }
            Global.dialogLoader.active = false
            Global.dialogLoader.setSource("dialogs/PrivacyWarning.qml",
                                          {
                                              openExternally: false,
                                              text: qsTr("You have shared a location with <strong>Enroute Flight Navigation</strong>.")
                                                    + " " + qsTr("In order to find the relevant geographic coordinate, the website <strong>Google Maps</strong> must briefly be opened in an embedded web browser window."),
                                              url: url
                                          })
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

/*
    CenteringDialog {
        id: privacyWarning

        property string url
        property string site

        modal: true

        title: qsTr("Privacy warning")

        ColumnLayout {
            anchors.fill: parent

            DecoratedScrollView{
                Layout.fillHeight: true
                Layout.fillWidth: true

                contentWidth: availableWidth // Disable horizontal scrolling

                clip: true

                Label {
                    id: lbl
                    text: "<p>"
                          + qsTr("You have shared a location with <strong>Enroute Flight Navigation</strong>.")
                          + " " + qsTr("In order to find the relevant geographic coordinate, the website <strong>%1</strong> must briefly be opened in an embedded web browser window.").arg(privacyWarning.site)
                          + " " + qsTr("The authors of <strong>Enroute Flight Navigation</strong> do not control this website.")
                          + " " + qsTr("They do not know what data it collects or how that data is processed.")
                          + "</p>"
                          + "<p>"
                          + " " + qsTr("With the click on OK, you consent to Enroute accessing <strong>%1</strong> from your device.").arg(privacyWarning.site)
                          + " " + qsTr("Click OK only if you agree with the terms and privacy policies of that site.")
                          + "</p>"

                    width: privacyWarning.availableWidth
                    textFormat: Text.RichText
                    wrapMode: Text.Wrap
                }
            }

            Item {
                Layout.preferredHeight: lbl.font.pixelSize
            }

            WordWrappingCheckDelegate {
                id: alwaysOpen

                Layout.fillWidth: true

                text: qsTr("Always open external web sites, do not ask again")
                checked: GlobalSettings.alwaysOpenExternalWebsites
            }
        }

        standardButtons: Dialog.Cancel|Dialog.Ok

        onAccepted: {
            PlatformAdaptor.vibrateBrief()
            GlobalSettings.alwaysOpenExternalWebsites = alwaysOpen.checked
            stackView.push("../pages/URLResolver.qml", {mapURL: url})
        }
    }
*/
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

        onRejected: close()
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

            var errorString = VACLibrary.importVAC(importManager.filePath, mapNameVAC.text)
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

            Label {
                Layout.fillWidth: true
                visible: DataManager.baseMapsVector.hasFile
                text: qsTr("To avoid conflicts, vector maps will be not be shown while raster maps are installed.")

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
            close()

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
