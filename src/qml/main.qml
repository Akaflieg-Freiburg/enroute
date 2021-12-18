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

import "dialogs"
import "items"
import "pages"

ApplicationWindow {
    id: view
    objectName: "applicationWindow"

    visible: true
    title: "Enroute Flight Navigation"
    width: 800
    height: 800

    Material.theme: global.settings().nightMode ? Material.Dark : Material.Light
    Material.primary: Material.theme === Material.Dark ? Qt.darker("teal") : "teal"
    Material.accent: Material.theme === Material.Dark ? Qt.lighter("teal") : "teal"

    Drawer {
        id: drawer

        height: view.height

        ScrollView {
            anchors.fill: parent

            ColumnLayout {
                id: col

                spacing: 0

                Rectangle {
                    height: 16
                    Layout.fillWidth: true
                    color: Material.primary
                }

                Label {
                    Layout.fillWidth: true
                    leftPadding: 16
                    rightPadding: 16

                    text: "Enroute Flight Navigation " + Qt.application.version
                    color: "white"
                    font.pixelSize: 20
                    font.weight: Font.Medium

                    background: Rectangle {
                        color: Material.primary
                    }
                }

                Rectangle {
                    height: 4

                    Layout.fillWidth: true
                    color: Material.primary
                }

                Label {
                    Layout.fillWidth: true
                    leftPadding: 16
                    rightPadding: 16
                    height: 20

                    text: "Akaflieg Freiburg"
                    font.pixelSize: 16
                    color: "white"

                    background: Rectangle {
                        color: Material.primary
                    }
                }

                Rectangle {
                    height: 18

                    Layout.fillWidth: true
                    color: Material.primary
                }

                ItemDelegate {
                    id: menuItemAircraft
                    text: qsTr("Aircraft")
                    icon.source: "/icons/material/ic_airplanemode_active.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/Aircraft.qml")
                        drawer.close()
                    }
                }

                ItemDelegate {
                    id: menuItemRoute
                    text: qsTr("Route and Wind")
                    icon.source: "/icons/material/ic_directions.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/FlightRouteEditor.qml")
                        drawer.close()
                    }
                }


                ItemDelegate {
                    id: menuItemNearby

                    text: qsTr("Nearby Waypoints")
                    icon.source: "/icons/material/ic_my_location.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/Nearby.qml")
                        drawer.close()
                    }
                }

                ItemDelegate {
                    id: weatherItem

                    text: qsTr("Weather")
                    icon.source: "/icons/material/ic_cloud_queue.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/WeatherPage.qml")
                        drawer.close()
                    }
                }

                Rectangle {
                    height: 1
                    Layout.fillWidth: true
                    color: Material.primary
                }

                ItemDelegate { // Library
                    text: qsTr("Library")
                    icon.source: "/icons/material/ic_library_books.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        libraryMenu.popup()
                    }

                    AutoSizingMenu {
                        id: libraryMenu                        

                        ItemDelegate {
                            text: qsTr("Aircraft")
                            icon.source: "/icons/material/ic_airplanemode_active.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                global.mobileAdaptor().vibrateBrief()
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
                                global.mobileAdaptor().vibrateBrief()
                                stackView.push("pages/FlightRouteLibrary.qml")
                                libraryMenu.close()
                                drawer.close()
                            }
                        }


                        ItemDelegate {
                            text: qsTr("Maps and Data")
                                  + (global.dataManager().geoMaps.updatable ? `<br><font color="#606060" size="2">` +qsTr("Updates available") + "</font>" : "")
                                  + (global.navigator().isInFlight ? `<br><font color="#606060" size="2">` +qsTr("Item not available in flight") + "</font>" : "")
                            icon.source: "/icons/material/ic_map.svg"
                            Layout.fillWidth: true

                            enabled: !global.navigator().isInFlight
                            onClicked: {
                                global.mobileAdaptor().vibrateBrief()
                                stackView.push("pages/DataManager.qml")
                                libraryMenu.close()
                                drawer.close()
                            }
                        }

                    }

                }

                ItemDelegate {
                    id: menuItemSettings

                    text: qsTr("Settings")
                    icon.source: "/icons/material/ic_settings.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/SettingsPage.qml")
                        drawer.close()
                    }
                }

                Rectangle {
                    height: 1
                    Layout.fillWidth: true
                    color: Material.primary
                }

                ItemDelegate { // Info
                    text: qsTr("Information")
                    icon.source: "/icons/material/ic_info_outline.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        aboutMenu.popup()
                    }

                    AutoSizingMenu { // Info Menu
                        id: aboutMenu

                        ItemDelegate { // Sat Status
                            text: qsTr("Positioning")
                                  +`<br><font color="#606060" size="2">`
                                  + (global.positionProvider().receivingPositionInfo ? qsTr("Receiving position information.") : qsTr("Not receiving position information."))
                                  + `</font>`
                            icon.source: "/icons/material/ic_satellite.svg"
                            Layout.fillWidth: true
                            onClicked: {
                                global.mobileAdaptor().vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/Positioning.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                            background: Rectangle {
                                anchors.fill: parent
                                color: global.positionProvider().receivingPositionInfo ? "green" : "red"
                                opacity: 0.2
                            }
                        }

                        ItemDelegate { // FLARM Status
                            Layout.fillWidth: true

                            text: qsTr("Traffic Receiver")
                                  + `<br><font color="#606060" size="2">`
                                  + ((global.trafficDataProvider().receivingHeartbeat) ? qsTr("Receiving heartbeat.") : qsTr("Not receiving heartbeat."))
                                  + `</font>`
                            icon.source: "/icons/material/ic_airplanemode_active.svg"
                            onClicked: {
                                global.mobileAdaptor().vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/TrafficReceiver.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                            background: Rectangle {
                                anchors.fill: parent
                                color: (global.trafficDataProvider().receivingHeartbeat) ? "green" : "red"
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
                                global.mobileAdaptor().vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/InfoPage.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate { // Participate
                            text: qsTr("Participate")
                            icon.source: "/icons/nav_participate.svg"

                            onClicked: {
                                global.mobileAdaptor().vibrateBrief()
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
                                global.mobileAdaptor().vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/DonatePage.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                        }
                    }

                }

                ItemDelegate { // Manual
                    text: qsTr("Manual")
                    icon.source: "/icons/material/ic_book.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        manualMenu.popup()
                    }


                    AutoSizingMenu {
                        id: manualMenu

                        ItemDelegate {
                            text: qsTr("Read manual")
                            icon.source: "/icons/material/ic_book.svg"
                            Layout.fillWidth: true
                            visible: Qt.platform.os == "android"
                            height: visible ? undefined : 0

                            onClicked: {
                                global.mobileAdaptor().vibrateBrief()
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
                            visible: Qt.platform.os == "android"
                            height: visible ? 1 : 0
                            Layout.fillWidth: true
                            color: Material.primary
                        }

                        ItemDelegate {
                            text: qsTr("Open in browser")
                            icon.source: "/icons/material/ic_open_in_browser.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                global.mobileAdaptor().vibrateBrief()
                                Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteText/manual")
                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                            }

                        }

                        ItemDelegate { // Manual … download as ebook
                            text: qsTr("Download as ebook")
                            icon.source: "/icons/material/ic_file_download.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                global.mobileAdaptor().vibrateBrief()
                                Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteText/manual.epub")
                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate { // Manual … download as ebook
                            text: qsTr("Download as PDF")
                            icon.source: "/icons/material/ic_file_download.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                global.mobileAdaptor().vibrateBrief()
                                Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteText/manual.pdf")

                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                    }

                }

                ItemDelegate { // Bug report
                    text: qsTr("Bug report")
                    icon.source: "/icons/material/ic_bug_report.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/BugReportPage.qml")
                        aboutMenu.close()
                        drawer.close()
                    }
                }

                Rectangle {
                    height: 1
                    Layout.fillWidth: true
                    color: Material.primary
                    visible: !global.navigator().isInFlight
                }

                ItemDelegate { // Exit
                    text: qsTr("Exit")
                    icon.source: "/icons/material/ic_exit_to_app.svg"
                    Layout.fillWidth: true

                    onClicked: {
                        global.mobileAdaptor().vibrateBrief()
                        drawer.close()
                        if (global.navigator().isInFlight)
                            exitDialog.open()
                        else
                            Qt.quit()
                    }
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
            // Things to do on startup. If the user has not yet accepted terms and conditions, show that.
            // Otherwise, if the user has not used this version of the app before, show the "what's new" dialog.
            // Otherwise, if the maps need updating, show the "update map" dialog.
            if (global.mobileAdaptor().missingPermissionsExist()) {
                dialogLoader.active = false
                dialogLoader.source = "dialogs/MissingPermissionsDialog.qml"
                dialogLoader.active = true
                return;
            }

            if (global.settings().acceptedTerms === 0) {
                dialogLoader.active = false
                dialogLoader.source = "dialogs/FirstRunDialog.qml"
                dialogLoader.active = true
                return
            }

            // Start accepting files
            global.mobileAdaptor().startReceiveOpenFileRequests()

            if ((global.settings().lastWhatsNewHash !== global.librarian().getStringHashFromRessource(":text/whatsnew.html")) && !global.navigator().isInFlight) {
                whatsNewDialog.open()
                return
            }

            if (global.dataManager().geoMaps.updatable && !global.navigator().isInFlight) {
                dialogLoader.active = false
                dialogLoader.source = "dialogs/UpdateMapDialog.qml"
                dialogLoader.active = true
                return
            }
        }

        Keys.onReleased: {
            if (event.key === Qt.Key_Back) {
                if (stackView.depth > 1)
                    stackView.pop()
                else {
                    if (global.navigator().isInFlight)
                        exitDialog.open()
                    else
                        Qt.quit()
                }

                event.accepted = true
            }
        }

        Connections {
            target: global.demoRunner()

            function onRequestClosePages() {
                stackView.pop()
            }

            function onRequestOpenAircraftPage() {
                stackView.pop()
                stackView.push("pages/Aircraft.qml")
            }

            function onRequestOpenNearbyPage() {
                stackView.pop()
                stackView.push("pages/Nearby.qml")
            }

            function onRequestOpenWeatherPage() {
                stackView.pop()
                stackView.push("pages/WeatherPage.qml")
            }

        }
    }

    DropArea {
        anchors.fill: stackView
        onDropped: {
            global.mobileAdaptor().processFileOpenRequest(drop.text)
        }
    }

    Label {
        id: toast

        width: Math.min(parent.width-4*Qt.application.font.pixelSize, 40*Qt.application.font.pixelSize)
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
            target: global.trafficDataProvider()
            function onReceivingHeartbeatChanged() {
                if (global.trafficDataProvider().receivingHeartbeat)
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
        
        title: qsTr("What's new …?")
        text: global.librarian().getStringFromRessource(":text/whatsnew.html")
        onOpened: global.settings().lastWhatsNewHash = global.librarian().getStringHashFromRessource(":text/whatsnew.html")
    }

    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }

    Shortcut {
        sequence: StandardKey.Close
        onActivated: Qt.quit()
    }

    //
    // Connections
    //

    Connections { // GeoMaps
        target: global.dataManager().geoMaps

        function onDownloadingChanged(downloading) {
            if (downloading) {
                global.notifier().showNotification(Notifier.DownloadInfo, "", "");
            } else {
                global.notifier().hideNotification(Notifier.DownloadInfo);
            }
        }
    }

    Connections { // Notifier
        target: global.notifier()

        function onNotificationClicked(notifyID) {
            if ((notifyID === 0) && (stackView.currentItem.objectName !== "DataManagerPage")) {
                stackView.push("pages/DataManager.qml")
            }
            if ((notifyID === 1) && (stackView.currentItem.objectName !== "TrafficReceiverPage")) {
                stackView.push("pages/TrafficReceiver.qml")
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
            global.settings().ignoreSSLProblems = true
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
        target: global.trafficDataProvider()

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
