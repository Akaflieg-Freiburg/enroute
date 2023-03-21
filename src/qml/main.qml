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
import QtQuick.Controls.Material
import Qt.labs.settings
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import enroute 1.0

import "dialogs"
import "items"
import "pages"

ApplicationWindow {
    id: view
    objectName: "applicationWindow"

    // On Android, fullscreen mode is set in JAVA code
    flags: ((Qt.platform.os === "android") || (Qt.platform.os === "ios")) ? Qt.MaximizeUsingFullscreenGeometryHint | Qt.Window : Qt.Window
    font.pixelSize: GlobalSettings.fontSize
    font.letterSpacing: GlobalSettings.fontSize > 15 ? 0.5 : 0.25

    visible: true
    title: "Enroute Flight Navigation"
    width: 800
    height: 800

    Settings {
        property alias x: view.x
        property alias y: view.y
        property alias width: view.width
        property alias height: view.height
    }

    Material.theme: GlobalSettings.nightMode ? Material.Dark : Material.Light
    Material.primary: Material.theme === Material.Dark ? Qt.darker("teal") : "teal"
    Material.accent: Material.theme === Material.Dark ? Qt.lighter("teal") : "teal"

    Drawer {
        id: drawer

        height: view.height
        width: col.implicitWidth

        ScrollView {
            anchors.fill: parent

            ColumnLayout {
                id: col

                spacing: 0

                Label { // Title
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left
                    rightPadding: 16
                    topPadding: 16+SafeInsets.top

                    text: "Enroute Flight Navigation"
                    color: "white"
                    font.pixelSize: 20
                    font.weight: Font.Medium

                    background: Rectangle {
                        color: Material.primary
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 4

                    color: Material.primary
                }

                Label { // Subtitle
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left
                    rightPadding: 16
                    Layout.preferredHeight: 20

                    text: "Akaflieg Freiburg" + " • v" + Qt.application.version
                    font.pixelSize: 16
                    color: "white"

                    background: Rectangle {
                        color: Material.primary
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 18
                    Layout.fillWidth: true

                    color: Material.primary
                }

                ItemDelegate { // Aircraft
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    id: menuItemAircraft
                    text: qsTr("Aircraft")
                    icon.source: "/icons/material/ic_airplanemode_active.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/AircraftPage.qml", {"dialogLoader": dialogLoader, "stackView": stackView})
                        drawer.close()
                    }
                }

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    id: menuItemRoute
                    text: qsTr("Route and Wind")
                    icon.source: "/icons/material/ic_directions.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/FlightRouteEditor.qml")
                        drawer.close()
                    }
                }

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    id: menuItemNearby

                    text: qsTr("Nearby Waypoints")
                    icon.source: "/icons/material/ic_my_location.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/Nearby.qml")
                        drawer.close()
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true

                    color: Material.primary
                }

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    text: qsTr("Library")
                    icon.source: "/icons/material/ic_library_books.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        libraryMenu.popup()
                    }

                    AutoSizingMenu {
                        id: libraryMenu

                        ItemDelegate {
                            text: qsTr("Aircraft")
                            icon.source: "/icons/material/ic_airplanemode_active.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.push("pages/AircraftLibrary.qml")
                                libraryMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate {
                            text: qsTr("Flight Routes")
                            icon.source: "/icons/material/ic_directions.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.push("pages/FlightRouteLibrary.qml")
                                libraryMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate {
                            text: qsTr("Maps and Data")
                                  + ( DataManager.mapsAndData.updateSize.isNull() ? "" : `<br><font color="#606060" size="2">` + qsTr("Updates available") + "</font>")
                                  + ( (Navigator.flightStatus === Navigator.Flight) ? `<br><font color="#606060" size="2">` +qsTr("Item not available in flight") + "</font>" : "")
                            icon.source: "/icons/material/ic_map.svg"
                            Layout.fillWidth: true

                            enabled: Navigator.flightStatus !== Navigator.Flight
                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.push("pages/DataManagerPage.qml", {"dialogLoader": dialogLoader, "stackView": stackView})
                                libraryMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate {
                            text: qsTr("Waypoints")
                            icon.source: "/icons/waypoints/WP.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.push("pages/WaypointLibraryPage.qml")
                                libraryMenu.close()
                                drawer.close()
                            }
                        }
                    }
                }

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    id: menuItemSettings

                    text: qsTr("Settings")
                    icon.source: "/icons/material/ic_settings.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/SettingsPage.qml")
                        drawer.close()
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true

                    color: Material.primary
                }

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    text: qsTr("Information")
                    icon.source: "/icons/material/ic_info_outline.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        aboutMenu.popup()
                    }

                    AutoSizingMenu { // Info Menu
                        id: aboutMenu

                        ItemDelegate { // Sat Status
                            text: qsTr("Positioning")
                                  +`<br><font color="#606060" size="2">`
                                  + (PositionProvider.receivingPositionInfo ? qsTr("Receiving position information.") : qsTr("Not receiving position information."))
                                  + `</font>`
                            icon.source: "/icons/material/ic_satellite.svg"
                            Layout.fillWidth: true
                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/Positioning.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                            background: Rectangle {
                                anchors.fill: parent
                                color: PositionProvider.receivingPositionInfo ? "green" : "red"
                                opacity: 0.2
                            }
                        }

                        ItemDelegate { // FLARM Status
                            Layout.fillWidth: true

                            text: qsTr("Traffic Receiver")
                                  + `<br><font color="#606060" size="2">`
                                  + ((TrafficDataProvider.receivingHeartbeat) ? qsTr("Receiving heartbeat.") : qsTr("Not receiving heartbeat."))
                                  + `</font>`
                            icon.source: "/icons/material/ic_airplanemode_active.svg"
                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/TrafficReceiver.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                            background: Rectangle {
                                anchors.fill: parent
                                color: (TrafficDataProvider.receivingHeartbeat) ? "green" : "red"
                                opacity: 0.2
                            }
                        }

                        Rectangle {
                            height: 1
                            Layout.fillWidth: true
                            color: Material.primary
                        }

                        ItemDelegate { // About
                            text: qsTr("About Enroute Flight Navigation")
                            icon.source: "/icons/material/ic_info_outline.svg"

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/InfoPage.qml", {"stackView": stackView, "toast": toast})
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate { // Privacy Policy
                            text: qsTr("Privacy Policy")
                            icon.source: "/icons/material/ic_vpn_key.svg"

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/PrivacyPage.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate { // Participate
                            text: qsTr("Participate")
                            icon.source: "/icons/nav_participate.svg"

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/ParticipatePage.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate { // Donate
                            text: qsTr("Donate")
                            icon.source: "/icons/material/ic_attach_money.svg"

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/DonatePage.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                        }
                    }

                }

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    text: qsTr("Manual")
                    icon.source: "/icons/material/ic_book.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        manualMenu.popup()
                    }


                    AutoSizingMenu {
                        id: manualMenu

                        ItemDelegate {
                            text: qsTr("Read manual")
                            icon.source: "/icons/material/ic_book.svg"
                            Layout.fillWidth: true
                            visible: Qt.platform.os === "android"
                            height: visible ? undefined : 0

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/Manual.qml", {"fileName": "index.html"})
                                aboutMenu.close()
                                drawer.close()

                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                            }

                        }

                        Rectangle {
                            visible: Qt.platform.os === "android"
                            height: visible ? 1 : 0
                            Layout.fillWidth: true
                            color: Material.primary
                        }

                        ItemDelegate {
                            text: qsTr("Open in browser")
                            icon.source: "/icons/material/ic_open_in_browser.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteText/manual")
                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                            }

                        }

                        ItemDelegate { // Manual… download as ebook
                            text: qsTr("Download as ebook")
                            icon.source: "/icons/material/ic_file_download.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteText/manual.epub")
                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate { // Manual… download as ebook
                            text: qsTr("Download as PDF")
                            icon.source: "/icons/material/ic_file_download.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteText/manual.pdf")

                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                    }

                }

                ItemDelegate { // Bug report
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    text: qsTr("Bug report")
                    icon.source: "/icons/material/ic_bug_report.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/BugReportPage.qml")
                        aboutMenu.close()
                        drawer.close()
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true

                    color: Material.primary
                    visible: Navigator.flightStatus !== Navigator.Flight
                }

                ItemDelegate { // Exit
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    text: qsTr("Exit")
                    icon.source: "/icons/material/ic_exit_to_app.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        drawer.close()
                        if (Navigator.flightStatus === Navigator.Flight)
                            exitDialog.open()
                        else
                            Qt.quit()
                    }
                }

                Item {
                    Layout.preferredHeight: SafeInsets.bottom
                }

                Item {
                    id: spacer
                    Layout.fillHeight: true
                }
            }
        }

    }


    StackView {
        id: stackView

        initialItem: "pages/MapPage.qml"

        anchors.fill: parent

        focus: true

        Component.onCompleted: {
            PlatformAdaptor.onGUISetupCompleted()

            if (firstRunDialog.conditionalOpen())
                return;

            if (DataManager.appUpdateRequired) {
                dialogLoader.active = false
                dialogLoader.setSource("dialogs/LongTextDialog.qml",
                                       {
                                           title: qsTr("Update required!"),
                                           text: Librarian.getStringFromRessource("appUpdateRequired"),
                                           standardButtons: Dialog.Ok
                                       }
                                       )
                dialogLoader.active = true
                return
            }

            if ((GlobalSettings.lastWhatsNewHash !== Librarian.getStringHashFromRessource(":text/whatsnew.html")) && (Navigator.flightStatus !== Navigator.Flight)) {
                whatsNewDialog.open()
                return
            }

            if ((GlobalSettings.lastWhatsNewInMapsHash !== DataManager.whatsNewHash) &&
                    (DataManager.whatsNew !== "") &&
                    (Navigator.flightStatus !== Navigator.Flight)) {
                whatsNewInMapsDialog.open()
                return
            }

        }

        Keys.onReleased: function (event) {
            if (event.key === Qt.Key_Back) {
                if (stackView.depth > 1)
                    stackView.pop()
                else {
                    if (Navigator.flightStatus === Navigator.Flight)
                        exitDialog.open()
                    else
                        Qt.quit()
                }

                event.accepted = true
            }
        }

        Connections {
            target: DemoRunner

            function onRequestClosePages() {
                stackView.pop()
                whatsNewDialog.close()
            }

            function onRequestOpenAircraftPage() {
                stackView.pop()
                stackView.push("pages/AircraftPage.qml")
            }

            function onRequestOpenNearbyPage() {
                stackView.pop()
                stackView.push("pages/Nearby.qml")
            }

            function onRequestOpenRoutePage() {
                stackView.pop()
                stackView.push("pages/FlightRouteEditor.qml", {"toast": toast})
            }

            function onRequestOpenWeatherPage() {
                stackView.pop()
                stackView.push("pages/WeatherPage.qml")
            }

        }
    }

    DropArea {
        anchors.fill: stackView
        onDropped: (drop) => {
            FileExchange.processFileOpenRequest(drop.text)
        }
    }

    Label {
        id: toast

        width: Math.min(parent.width-4*view.font.pixelSize, 40*view.font.pixelSize)
        x: (parent.width-width)/2.0
        y: parent.height*(3.0/4.0)-height/2.0

        text: "Lirum Larum, Löffelstiel"
        wrapMode: Text.Wrap

        color: "white"
        bottomInset: -5
        topInset: -5
        leftInset: -5
        rightInset: -5

        horizontalAlignment: Text.AlignHCenter
        background: Rectangle {
            color: Material.primary
            radius: 5
        }

        opacity: 0
        SequentialAnimation {
            id: seqA

            NumberAnimation { target: toast; property: "opacity"; to: 1; duration: 400 }
            PauseAnimation { duration: 1000 }
            NumberAnimation { target: toast; property: "opacity"; to: 0; duration: 400 }
        }

        function doToast(string) {
            toast.text = string
            seqA.start()
        }

        Connections { // Traffic receiver
            target: TrafficDataProvider
            function onReceivingHeartbeatChanged() {
                if (TrafficDataProvider.receivingHeartbeat)
                    toast.doToast(qsTr("Connected to traffic receiver."))
                else
                    toast.doToast(qsTr("Lost connection to traffic receiver."))
            }
        }

    }

    Loader {
        id: dialogLoader
        anchors.fill: parent

        property string title
        property string text
        property var dialogArgs: undefined

        onLoaded: {
            item.anchors.centerIn = Overlay.overlay
            item.modal = true
            if (dialogArgs && item.hasOwnProperty('dialogArgs')) {
                item.dialogArgs = dialogArgs
            }
            item.open()
        }

    }

    ImportManager {
        id: importMgr

        // Repeater properties
        stackView: stackView
        toast: toast
        view: view
    }

    LongTextDialog {
        id: exitDialog
        standardButtons: Dialog.No | Dialog.Yes

        title: qsTr("Exit…?")
        text: qsTr("Do you wish to exit <strong>Enroute Flight Navigation</strong>?")

        onAccepted: Qt.quit()
        onRejected: close()
    }

    LongTextDialog {
        id: whatsNewDialog
        standardButtons: Dialog.Ok
        
        title: qsTr("What's new…?")
        text: Librarian.getStringFromRessource(":text/whatsnew.html")
        onOpened: GlobalSettings.lastWhatsNewHash = Librarian.getStringHashFromRessource(":text/whatsnew.html")
    }

    LongTextDialog {
        id: whatsNewInMapsDialog
        standardButtons: Dialog.Ok

        title: qsTr("What's new…?")
        text: DataManager.whatsNew
        onOpened: GlobalSettings.lastWhatsNewInMapsHash = DataManager.whatsNewHash
    }

    FirstRunDialog {
        id: firstRunDialog
    }

    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }

    Shortcut {
        sequences: [StandardKey.Close]
        onActivated: Qt.quit()
    }

    //
    // Connections
    //

    Connections { // items
        target: DataManager.items

        function onDownloadingChanged(downloading) {
            if (downloading) {
                global.notifier().showNotification(Notifier.DownloadInfo, "", "");
            } else {
                global.notifier().hideNotification(Notifier.DownloadInfo);
            }
        }
    }

    Connections { // Navigator
        target: Navigator

        function onAirspaceAltitudeLimitAdjusted() {
            var airspaceAltitudeLimit = GlobalSettings.airspaceAltitudeLimit
            if (airspaceAltitudeLimit.isFinite()) {
                var airspaceAltitudeLimitString = Navigator.aircraft.verticalDistanceToString(airspaceAltitudeLimit)
                toast.doToast(qsTr("Now showing airspaces up to %1.").arg(airspaceAltitudeLimitString))
            } else {
                toast.doToast(qsTr("Now showing all airspaces."))
            }
        }

    }

    Connections { // Notifier
        target: global.notifier()

        function onAction(act) {
            if ((act === Notifier.DownloadInfo_Clicked) && (stackView.currentItem.objectName !== "DataManagerPage")) {
                stackView.push("pages/DataManagerPage.qml", {"dialogLoader": dialogLoader, "stackView": stackView})
            }
            if ((act === Notifier.TrafficReceiverSelfTestError_Clicked) && (stackView.currentItem.objectName !== "TrafficReceiverPage")) {
                stackView.push("pages/TrafficReceiver.qml")
            }
            if ((act === Notifier.TrafficReceiverRuntimeError_Clicked) && (stackView.currentItem.objectName !== "TrafficReceiverPage")) {
                stackView.push("pages/TrafficReceiver.qml")
            }
            if ((act === Notifier.GeoMapUpdatePending_Clicked) && (stackView.currentItem.objectName !== "DataManagerPage")) {
                stackView.push("pages/DataManagerPage.qml", {"dialogLoader": dialogLoader, "stackView": stackView})
            }
            if (act === Notifier.GeoMapUpdatePending_UpdateRequested) {
                DataManager.mapsAndData.update()
                toast.doToast(qsTr("Starting map update"))
            }
        }

    }

    Connections { // SSLErrorHandler
        target: global.sslErrorHandler()


        function onSslError(message) {
            sslErrorDialog.text = message
            sslErrorDialog.open()
        }

    }

    LongTextDialog {
        id: sslErrorDialog

        standardButtons: Dialog.Close|Dialog.Ignore

        title: qsTr("Network security error")

        onAccepted: {
            close()
            GlobalSettings.ignoreSSLProblems = true
            sslErrorConfirmation.open()
        }

        LongTextDialogMD {
            id: sslErrorConfirmation
            standardButtons: Dialog.Ok

            title: qsTr("Network security settings")
            text: qsTr("You have chosen to ignore network security errors in the future. \
**This poses a security risk.** \
Go to the 'Settings' page if you wish to restore the original, safe, behavior of this app.")
        }
    }

    Connections { // TrafficDataProvider
        target: TrafficDataProvider

        function onPasswordRequest(ssid) {
            dialogLoader.active = false
            dialogLoader.dialogArgs = ssid
            dialogLoader.source = "dialogs/PasswordDialog.qml"
            dialogLoader.active = true
        }

        function onPasswordStorageRequest(ssid, password) {
            dialogLoader.active = false
            dialogLoader.dialogArgs = ssid
            dialogLoader.text = password
            dialogLoader.source = "dialogs/PasswordStorageDialog.qml"
            dialogLoader.active = true
        }

        function onTrafficReceiverRuntimeErrorChanged(message) {
            if (message === "") {
                global.notifier().hideNotification(Notifier.TrafficReceiverRuntimeError);
            } else {
                global.notifier().showNotification(Notifier.TrafficReceiverRuntimeError, message, message);
            }
        }

        function onTrafficReceiverSelfTestErrorChanged(message) {
            if (message === "") {
                global.notifier().hideNotification(Notifier.TrafficReceiverSelfTestError);
            } else {
                global.notifier().showNotification(Notifier.TrafficReceiverSelfTestError, message, message);
            }
        }
    }

    // Enroute closed unexpectedly if...
    // * the "route" page is open
    // * the route menu is opened
    // * then the menu is closed again with the back button w/o selecting a route menu item
    // * then the back button is pressed again (while the route page is still open)
    //
    // apparently for this scenario the stackView doesn't have focus but rather
    // the main application window. Therefore we've to handle the back event
    // here and decide on the stackView.depth if we close the application
    // or rather use stackView.pop().
    //
    // solution from
    // see https://stackoverflow.com/questions/25968661/android-back-button-press-doesnt-trigger-keys-onreleased-qml
    //
    onClosing: {
        // Use this hack only on the Android platform
        if (Qt.platform.os !== "android")
            return

        if(stackView.depth > 1) {
            close.accepted = false // prevent closing of the app
            stackView.pop(); // will close the stackView page
        }
    }

    function openManual(pageUrl) {
        if (Qt.platform.os === "android")
            stackView.push("pages/Manual.qml", {"fileName": pageUrl})
        else
            Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteText/manual/"+pageUrl)
    }
}
