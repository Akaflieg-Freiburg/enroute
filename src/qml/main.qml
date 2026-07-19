/***************************************************************************
 *   Copyright (C) 2019-2026 by Stefan Kebekus                             *
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

import QtCore
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "dialogs"
import "items"


AppWindow {
    id: view
    objectName: "applicationWindow"

    flags: ((Qt.platform.os === "android") || (Qt.platform.os === "ios")) ? Qt.ExpandedClientAreaHint | Qt.NoTitleBarBackgroundHint | Qt.Window : Qt.Window

    Component.onCompleted: {
        Application.styleHints.colorScheme = Qt.ColorScheme.Dark
    }

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    // The safe area reported by Qt covers system bars and display cutouts,
    // but not the virtual keyboard. On iOS, the former C++ SafeInsets
    // implementation included the keyboard in the bottom inset, and the QML
    // code relies on that to keep footers and dialogs clear of the keyboard.
    // Additional margins propagate into the SafeArea.margins of every item
    // in this window. (On Android, the keyboard is already contained in
    // PlatformAdaptor.safeInsets, see below.)
    SafeArea.additionalMargins.bottom: {
        if (Qt.platform.os === "ios") {
            if (!Qt.inputMethod.visible)
                return 0
            var kr = Qt.inputMethod.keyboardRectangle
            if (kr.height <= 0) // Empty rectangle, not trustworthy
                return 0
            return Math.max(0, view.height - kr.y)
        }

        return 0
    }

    // Feed the window-global safe-area margins into the SafeInsets singleton,
    // which the rest of the QML code reads. On Android, Qt's SafeArea margins
    // are unreliable -- wrong in split-screen mode, and inconsistent about
    // the virtual keyboard -- so the insets that the Android window system
    // reports are used instead (PlatformAdaptor.safeInsets; zero on all
    // other platforms). Elsewhere, the window's SafeArea provides the
    // values; the unqualified SafeArea below attaches to this window, so
    // they include the keyboard margin set above.
    readonly property bool useNativeInsets: Qt.platform.os === "android"
    readonly property real safeAreaBottom: useNativeInsets ? PlatformAdaptor.safeInsets.bottom : SafeArea.margins.bottom
    readonly property real safeAreaLeft: useNativeInsets ? PlatformAdaptor.safeInsets.left : SafeArea.margins.left
    readonly property real safeAreaRight: useNativeInsets ? PlatformAdaptor.safeInsets.right : SafeArea.margins.right
    readonly property real safeAreaTop: useNativeInsets ? PlatformAdaptor.safeInsets.top : SafeArea.margins.top

    Binding { target: SafeInsets; property: "bottom"; value: view.safeAreaBottom }
    Binding { target: SafeInsets; property: "left"; value: view.safeAreaLeft }
    Binding { target: SafeInsets; property: "right"; value: view.safeAreaRight }
    Binding { target: SafeInsets; property: "top"; value: view.safeAreaTop }

    font.pixelSize: GlobalSettings.fontSize
    font.letterSpacing: GlobalSettings.fontSize > 15 ? 0.5 : 0.25

    visible: true
    title: "Enroute Flight Navigation"

    Settings {
        property alias x: view.x
        property alias y: view.y
        property alias width: view.width
        property alias height: view.height
    }

    Drawer {
        id: drawer

        height: parent.height
        width: col.implicitWidth
        Material.roundedScale: Material.NotRounded
        topPadding: 0

        // When the drawer closes it restores active focus to the page that was
        // showing when it opened — which, after navigating to a new page, steals
        // focus back from the freshly-pushed page. Once fully closed, hand focus
        // to the current page's preferred control instead (if it declares one).
        // Closing the drawer restores active focus to the page that was showing
        // when it opened, stealing it back from a freshly-pushed page. Re-assert
        // the current page's preferred focus once the drawer has fully closed.
        onClosed: stackView.focusCurrentPage()

        DecoratedScrollView {
            anchors.fill: parent

            contentWidth: availableWidth // Disable horizontal scrolling

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
                        color: "teal"
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 4

                    color: "teal"
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
                        color: "teal"
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 18
                    Layout.fillWidth: true

                    color: "teal"
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
                        stackView.push("pages/AircraftPage.qml", {"stackView": stackView})
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

                    id: menuItemApproach

                    text: qsTr("Approach Charts")
                    icon.source: "/icons/material/ic_flight_land.svg"
                    visible: !VACLibrary.isEmpty

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/VAC.qml", {"dialogLoader": dialogLoader, "stackView": stackView})
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

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    id: weatherItem

                    text: qsTr("Weather")
                    icon.source: "/icons/material/ic_cloud_queue.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/Weather.qml")
                        drawer.close()
                    }
                }

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    id: menuItemFlightLog

                    text: qsTr("Flight Log")
                    icon.source: "/icons/material/ic_flight_takeoff.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/FlightLogPage.qml")
                        drawer.close()
                    }
                }

                Rectangle {
                    Layout.preferredHeight: 1
                    Layout.fillWidth: true

                    color: Global.dividerColor
                }

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    text: qsTr("Library")
                    icon.source: "/icons/material/ic_library_books.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        libraryMenu.open()
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

                    color: Global.dividerColor
                }

                ItemDelegate {
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    text: qsTr("Information")
                    icon.source: "/icons/material/ic_info_outline.svg"

                    onClicked: {
                        PlatformAdaptor.vibrateBrief()
                        aboutMenu.open()
                    }

                    AutoSizingMenu { // Info Menu
                        id: aboutMenu

                        ItemDelegate { // SatNav Status
                            text: qsTr("SatNav Positioning")
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
                                stackView.push("pages/TrafficReceiver.qml", {"appWindow": view})
                                aboutMenu.close()
                                drawer.close()
                            }
                            background: Rectangle {
                                anchors.fill: parent
                                color: {
                                    if (!TrafficDataProvider.receivingHeartbeat)
                                        return "red"
                                    if (TrafficDataProvider.currentSourceIsInternetService)
                                        return "yellow"
                                    return "green"
                                }
                                opacity: 0.2
                            }
                        }

                        ItemDelegate { // PressureAltitude
                            text: qsTr("Device Sensors")
                            icon.source: "/icons/material/ic_speed.svg"
                            Layout.fillWidth: true
                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/PressureAltitude.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                        Rectangle {
                            height: 1
                            Layout.fillWidth: true
                            color: Global.dividerColor
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
                                if (Qt.platform.os === "ios") {
                                    Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteManual/forward.html#donate")
                                } else {
                                    PlatformAdaptor.vibrateBrief()
                                    stackView.pop()
                                    stackView.push("pages/DonatePage.qml")
                                    aboutMenu.close()
                                    drawer.close()
                                }
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
                        manualMenu.open()
                    }


                    AutoSizingMenu {
                        id: manualMenu

                        ItemDelegate {
                            text: qsTr("Read manual")
                            icon.source: "/icons/material/ic_book.svg"
                            Layout.fillWidth: true
                            visible: (Qt.platform.os === "ios") || (Qt.platform.os === "android")
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
                            visible: (Qt.platform.os === "ios")
                            height: visible ? 1 : 0
                            Layout.fillWidth: true
                            color: Global.dividerColor
                        }

                        ItemDelegate {
                            text: qsTr("Open in browser")
                            icon.source: "/icons/material/ic_open_in_browser.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                                if (GlobalSettings.alwaysOpenExternalWebsites)
                                {
                                    Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteManual")
                                    return
                                }
                                Global.dialogLoader.active = false
                                Global.dialogLoader.setSource("dialogs/PrivacyWarning.qml",
                                                              {
                                                                  openExternally: true,
                                                                  text: qsTr("In order to show the manual, <strong>Enroute Flight Navigation</strong> will ask your system to open an external web site hosted by GitHub."),
                                                                  url: "https://akaflieg-freiburg.github.io/enrouteManual"
                                                              })
                                Global.dialogLoader.active = true
                            }

                        }

                        ItemDelegate { // Manual… download as ebook
                            text: qsTr("Download as ebook")
                            icon.source: "/icons/material/ic_file_download.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                                if (GlobalSettings.alwaysOpenExternalWebsites)
                                {
                                    Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteManual/manual.epub")
                                    return
                                }
                                Global.dialogLoader.active = false
                                Global.dialogLoader.setSource("dialogs/PrivacyWarning.qml",
                                                              {
                                                                  openExternally: true,
                                                                  text: qsTr("In order to download the manual, <strong>Enroute Flight Navigation</strong> will ask your system to open an external web site hosted by GitHub."),
                                                                  url: "https://akaflieg-freiburg.github.io/enrouteManual/manual.epub"
                                                              })
                                Global.dialogLoader.active = true
                            }
                        }

                        ItemDelegate { // Manual… download as ebook
                            text: qsTr("Download as PDF")
                            icon.source: "/icons/material/ic_file_download.svg"
                            Layout.fillWidth: true

                            onClicked: {
                                PlatformAdaptor.vibrateBrief()
                                manualMenu.close()
                                aboutMenu.close()
                                drawer.close()
                                if (GlobalSettings.alwaysOpenExternalWebsites)
                                {
                                    Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteManual/manual.pdf")
                                    return
                                }
                                Global.dialogLoader.active = false
                                Global.dialogLoader.setSource("dialogs/PrivacyWarning.qml",
                                                              {
                                                                  openExternally: true,
                                                                  text: qsTr("In order to download the manual, <strong>Enroute Flight Navigation</strong> will ask your system to open an external web site hosted by GitHub."),
                                                                  url: "https://akaflieg-freiburg.github.io/enrouteManual/manual.pdf"
                                                              })
                                Global.dialogLoader.active = true
                            }
                        }

                    }

                }

                ItemDelegate { // Bug report
                    Layout.fillWidth: true

                    leftPadding: 16+SafeInsets.left

                    text: qsTr("Bug Report")
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

                    color: Global.dividerColor
                    visible: Qt.platform.os !== "ios" && Navigator.flightStatus !== Navigator.Flight
                }

                ItemDelegate { // Exit
                    Layout.fillWidth: true
                    visible: Qt.platform.os !== "ios"
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

        Component.onCompleted: Global.drawer = this
    }

    StackView {
        id: stackView

        initialItem: "pages/MapPage.qml"

        // A Page is a Control, not a FocusScope, so giving it active focus stops
        // at the page and never reaches the control inside it. Pages that want a
        // specific control focused on appearance expose it as defaultFocusItem;
        // focus it here whenever the current page changes (push/pop).
        function focusCurrentPage() {
            if (currentItem && currentItem.defaultFocusItem &&
                (Qt.platform.os !== "android") && (Qt.platform.os !== "ios"))
                currentItem.defaultFocusItem.forceActiveFocus()
        }
        onCurrentItemChanged: focusCurrentPage()

        // Fill the window. This formerly used SafeInsets.wHeight/wWidth to work
        // around ApplicationWindow not tracking its size in the Android split
        // view; current Qt versions track the window size correctly.
        anchors.fill: parent

        focus: true

        Component.onCompleted: {
            PlatformAdaptor.onGUISetupCompleted()

            if (!DataManager.aviationMaps.hasFile ||
                    (GlobalSettings.privacyHash !== Librarian.getStringHashFromRessource(":text/privacy.html")) ||
                    (GlobalSettings.acceptedTerms === 0)) {
                Global.dialogLoader.active = false
                Global.dialogLoader.setSource("dialogs/FirstRunDialog.qml")
                Global.dialogLoader.active = true
                return
            }
            Global.locationPermission.request()

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

            if ((GlobalSettings.lastWhatsNewHash !== Librarian.getStringHashFromRessource(":text/whatsnew.html")) &&
                    (Navigator.flightStatus !== Navigator.Flight)) {
                Global.dialogLoader.active = false
                Global.dialogLoader.setSource("dialogs/LongTextDialog.qml", {
                                                  title: qsTr("What's new…?"),
                                                  text: Librarian.getStringFromRessource(":text/whatsnew.html"),
                                                  standardButtons: Dialog.Ok
                                              })
                Global.dialogLoader.active = true
                GlobalSettings.lastWhatsNewHash = Librarian.getStringHashFromRessource(":text/whatsnew.html")
                return
            }

            if ((GlobalSettings.lastWhatsNewInMapsHash !== DataManager.whatsNewHash) &&
                    (DataManager.whatsNew !== "") &&
                    (Navigator.flightStatus !== Navigator.Flight)) {
                Global.dialogLoader.active = false
                Global.dialogLoader.setSource("dialogs/LongTextDialog.qml", {
                                                  title: qsTr("What's new…?"),
                                                  text: DataManager.whatsNew,
                                                  standardButtons: Dialog.Ok
                                              })
                Global.dialogLoader.active = true
                onOpened: GlobalSettings.lastWhatsNewInMapsHash = DataManager.whatsNewHash
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
                if (Global.dialogLoader.item)
                    Global.dialogLoader.item.close()
            }

            function onRequestOpenAircraftPage() {
                stackView.pop()
                stackView.push("pages/AircraftPage.qml", {"stackView": stackView})
            }

            function onRequestOpenNearbyPage() {
                stackView.pop()
                stackView.push("pages/Nearby.qml")
            }

            function onRequestOpenRoutePage() {
                stackView.pop()
                stackView.push("pages/FlightRouteEditor.qml")
            }

            function onRequestOpenWeatherPage() {
                stackView.pop()
                stackView.push("pages/Weather.qml")
            }

            function onRequestOpenWeatherDialog(station) {
                Global.dialogLoader.setSource("dialogs/MetarTafDialog.qml",
                                              {"weatherStation": station})
                Global.dialogLoader.item.open()
            }

            function onRequestVAC(vacName) {
                Global.currentVAC = VACLibrary.get(vacName)
            }

        }
    }

    DropArea {
        anchors.fill: stackView
        onDropped: (drop) => {
                       if (drop.urls.length === 1) {
                           FileExchange.processFileOpenRequest(drop.text)
                           return
                       }
                       dialogLoader.active = false
                       dialogLoader.setSource("dialogs/LongTextDialog.qml",
                                              {
                                                  title: qsTr("Error!"),
                                                  text: qsTr("Unable to import more than one file at once. Please import only one file at a time."),
                                                  standardButtons: Dialog.Ok
                                              }
                                              )
                       dialogLoader.active = true
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
            color: "teal"
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
            if (seqA.running) {
                toast.text = string + " • " + toast.text
            } else
                toast.text = string
            seqA.start()
        }

        Component.onCompleted: Global.toast = this

        Connections { // Traffic receiver
            target: TrafficDataProvider
            function onReceivingHeartbeatChanged() {
                if (TrafficDataProvider.receivingHeartbeat) {
                    if (TrafficDataProvider.currentSourceIsInternetService)
                        toast.doToast(qsTr("Connected to internet service providing traffic data."))
                    else
                        toast.doToast(qsTr("Connected to traffic receiver."))
                } else
                    toast.doToast(qsTr("Lost connection to traffic receiver."))
            }
        }

        Connections { // Notification manager
            target: NotificationManager
            function onToastPosted(text) {
                toast.doToast(text)
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

    Loader {
        onLoaded: item.open()
        Component.onCompleted: Global.dialogLoader = this
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
        onRejected: exitDialog.close()
    }

    Shortcut {
        sequences: [StandardKey.Quit]
        onActivated: Qt.quit()
    }

    Shortcut {
        sequences: [StandardKey.Close]
        onActivated: Qt.quit()
    }

    Shortcut {
        sequences: [StandardKey.Paste]
        onActivated: {
            if (PlatformAdaptor.clipboardText() === "")
                return
            FileExchange.processText(PlatformAdaptor.clipboardText())
        }
    }

    //
    // Connections
    //

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

    Connections { // FlightLog
        target: FlightLog

        function onTakeoffDetected(time) {
            toast.doToast(qsTr("Start Time: %1 UTC").arg(time))
        }

        function onLandingDetected(time) {
            toast.doToast(qsTr("Landing Time: %1 UTC").arg(time))
        }

        function onSaveError(message) {
            Global.dialogLoader.active = false
            Global.dialogLoader.setSource("dialogs/LongTextDialog.qml", {
                                              title: qsTr("Flight log error"),
                                              text: qsTr("The flight log could not be saved to storage. Recent changes will be lost when the app closes.")
                                                    + "<br><br>" + qsTr("Reason: %1").arg(message),
                                              standardButtons: Dialog.Close
                                          })
            Global.dialogLoader.active = true
        }
    }

    Connections { // SSLErrorHandler
        target: SSLErrorHandler

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
            sslErrorDialog.close()
            GlobalSettings.ignoreSSLProblems = true
            sslErrorConfirmation.open()
        }

        LongTextDialogMD {
            id: sslErrorConfirmation
            standardButtons: Dialog.Ok

            title: qsTr("Network security settings")
            text: qsTr("You have chosen to ignore network security errors in the future.") + " **" +
                  qsTr("This poses a security risk.") + "** " +
                  qsTr("Go to the 'Settings' page if you wish to restore the original, safe, behavior of this app.")
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
    }

    Connections { // PlatformAdaptor
        target: PlatformAdaptor

        function onError(message) {
            dialogLoader.active = false
            dialogLoader.setSource("dialogs/LongTextDialog.qml",
                                   {
                                       title: qsTr("Error!"),
                                       text: message,
                                       standardButtons: Dialog.Ok
                                   }
                                   )
            dialogLoader.active = true
        }

    }

    Connections { // Notifier
        target: DataManager.mapsAndData

        function onDownloadingChanged() {
            if (DataManager.mapsAndData.downloading) {
                toast.doToast(qsTr("Starting map update"))
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
    function onClosing (close) {
        // Use this hack only on the Android platform
        if (Qt.platform.os !== "android")
            return

        if(stackView.depth > 1) {
            close.accepted = false // prevent closing of the app
            stackView.pop(); // will close the stackView page
        }
    }

    function openManual(pageUrl) {

        if ((Qt.platform.os === "ios") ||
                ((Qt.platform.os === "android") && (Qt.application.version < "6.7.0")))
        {
            stackView.push("pages/Manual.qml", {"fileName": pageUrl})
            return
        }
        Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enrouteManual/"+pageUrl)
    }
}

