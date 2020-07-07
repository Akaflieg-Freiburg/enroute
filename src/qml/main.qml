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
import QtSensors 5.14

import enroute 1.0

import "dialogs"
import "items"
import "pages"

ApplicationWindow {
    id: view
    visible: true
    title: qsTr("Enroute Flight Navigation")
    width: 1000
    height: 800

    Drawer {
        id: drawer
        width: col.implicitWidth
        height: view.height

        ScrollView {
            anchors.fill: parent

            ColumnLayout {
                id: col
                anchors.fill: parent
                spacing: 0

                Label {
                    Layout.fillWidth: true

                    text: "<strong>Enroute Flight Navigation</strong><br>Akaflieg Freiburg"
                    color: "white"
                    padding: Qt.application.font.pixelSize

                    background: Rectangle {
                        color: Material.primary
                    }
                }

                ItemDelegate {
                    id: menuItemRoute
                    text: qsTr("Route")
                    icon.source: "/icons/material/ic_directions.svg"
                    icon.color: Material.primary
                    Layout.fillWidth: true

                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/FlightRouteEditor.qml")
                        drawer.close()
                    }
                }

                ItemDelegate {
                    id: menuItemNearby

                    text: qsTr("Nearby Waypoints")
                    icon.source: "/icons/material/ic_my_location.svg"
                    icon.color: Material.primary
                    Layout.fillWidth: true

                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        stackView.pop()
                        stackView.push("pages/Nearby.qml")
                        drawer.close()
                    }
                }

                ItemDelegate {
                    id: weatherItem

                    text: qsTr("Weather")
                    icon.source: "/icons/material/ic_cloud_queue.svg"
                    icon.color: Material.primary
                    Layout.fillWidth: true

                    onClicked: {
                        mobileAdaptor.vibrateBrief()
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

                ItemDelegate {
                    text: qsTr("Set Altimeter") + (satNav.hasAltitude ? `<br><font color="#606060" size="2">${satNav.altitudeInFeetAsString} AMSL</font>` : `<br><font color="#606060" size="2">`
                                                                        + qsTr("Insufficient reception")+`</font>`)
                    icon.source: "/icons/material/ic_speed.svg"
                    icon.color: Material.primary
                    Layout.fillWidth: true
                    enabled: satNav.hasAltitude
                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        drawer.close()
                        dialogLoader.active = false
                        dialogLoader.source = "dialogs/AltitudeCorrectionDialog.qml"
                        dialogLoader.active = true
                    }
                }

                ItemDelegate {
                    id: menuItemSettings

                    text: qsTr("Settings")
                    icon.source: "/icons/material/ic_settings.svg"
                    icon.color: Material.primary
                    Layout.fillWidth: true

                    onClicked: {
                        mobileAdaptor.vibrateBrief()
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

                ItemDelegate {
                    text: qsTr("Information")
                    icon.source: "/icons/material/ic_info_outline.svg"
                    icon.color: Material.primary
                    Layout.fillWidth: true
                    visible: !satNav.isInFlight

                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        aboutMenu.popup()
                    }

                    AutoSizingMenu {
                        id: aboutMenu

                        ItemDelegate {
                            text: qsTr("About Enroute Flight Navigation")
                            icon.source: "/icons/material/ic_info_outline.svg"
                            icon.color: Material.primary

                            onClicked: {
                                mobileAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/InfoPage.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate {
                            text: qsTr("Bug report")
                            icon.source: "/icons/material/ic_bug_report.svg"
                            icon.color: Material.primary

                            onClicked: {
                                mobileAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/BugReportPage.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                        }

                        ItemDelegate {
                            text: qsTr("Participate")
                            icon.source: "/icons/nav_participate.svg"
                            icon.color: Material.primary

                            onClicked: {
                                mobileAdaptor.vibrateBrief()
                                stackView.pop()
                                stackView.push("pages/ParticipatePage.qml")
                                aboutMenu.close()
                                drawer.close()
                            }
                        }
                    } // Menu
                }

                ItemDelegate {
                    text: qsTr("Manual")
                    icon.source: "/icons/material/ic_help_outline.svg"
                    icon.color: Material.primary
                    Layout.fillWidth: true
                    visible: !satNav.isInFlight

                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        stackView.pop()
                        Qt.openUrlExternally("https://akaflieg-freiburg.github.io/enroute/manual");
                        drawer.close()
                    }
                }

                Rectangle {
                    height: 1
                    Layout.fillWidth: true
                    color: Material.primary
                    visible: !satNav.isInFlight
                }

                ItemDelegate {
                    text: qsTr("Exit")
                    icon.source: "/icons/material/ic_exit_to_app.svg"
                    icon.color: Material.primary
                    Layout.fillWidth: true

                    onClicked: {
                        mobileAdaptor.vibrateBrief()
                        drawer.close()
                        if (satNav.isInFlight)
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

        Connections {
            target: sensorGesture
            function onDetected(gesture) {
                drawer.close()
            }
        }

    } // Drawer

    StackView {
        id: stackView
        initialItem: "pages/MapPage.qml"
        anchors.fill: parent

        focus: true

        Component.onCompleted: {
            // Things to do on startup. If the user has not yet accepted terms and conditions, show that.
            // Otherwise, if the user has not used this version of the app before, show the "what's new" dialog.
            // Otherwise, if the maps need updating, show the "update map" dialog.
            if (mobileAdaptor.missingPermissionsExist()) {
                dialogLoader.active = false
                dialogLoader.source = "dialogs/MissingPermissionsDialog.qml"
                dialogLoader.active = true
                return;
            }

            if (globalSettings.acceptedTerms === 0) {
                dialogLoader.active = false
                dialogLoader.source = "dialogs/FirstRunDialog.qml"
                dialogLoader.active = true
                return
            }

            // Start accepting files
            mobileAdaptor.startReceiveOpenFileRequests()

            if ((globalSettings.lastWhatsNewHash !== librarian.getStringHashFromRessource(":text/whatsnew.html")) && !satNav.isInFlight) {
                whatsNewDialog.open()
                return
            }

            if (mapManager.geoMaps.updatable && !satNav.isInFlight) {
                dialogLoader.active = false
                dialogLoader.source = "dialogs/UpdateMapDialog.qml"
                dialogLoader.active = true
                return
            }
        } // Component.onCompleted

        Keys.onReleased: {
            if (event.key === Qt.Key_Back) {
                if (stackView.depth > 1)
                    stackView.pop()
                else {
                    if (satNav.isInFlight)
                        exitDialog.open()
                    else
                        Qt.quit()
                }

                event.accepted = true
            }
        }

        Connections {
            target: sensorGesture
            function onDetected(gesture) {
                stackView.pop(null)
            }
        }

    }

    DropArea {
        anchors.fill: stackView
        onDropped: {
            mobileAdaptor.processFileOpenRequest(drop.text)
        }
    }

    Loader {
        id: dialogLoader
        anchors.fill: parent

        property string title
        property string text
        property var dialogArgs: undefined

        onLoaded: {
            item.anchors.centerIn = dialogLoader
            item.modal = true
            if (dialogArgs && item.hasOwnProperty('dialogArgs')) {
                item.dialogArgs = dialogArgs
            }
            item.open()
        }

        Connections {
            target: sensorGesture
            function onDetected(gesture) {
                dialogLoader.active = false
            }
        }
    }

    ImportManager {
        id: importMgr
    }

    Dialog {
        id: exitDialog

        x: Math.round((parent.width - width) / 2)
        y: Math.round((parent.height - height) / 2)

        title: qsTr("Do you wish to exit Enroute Flight Navigation?")

        standardButtons: Dialog.No | Dialog.Yes

        modal: true

        onAccepted: Qt.quit()
        onRejected: close()
    }

    LongTextDialog {
        id: whatsNewDialog
        standardButtons: Dialog.Ok
        anchors.centerIn: parent
        
        title: qsTr("What's new …?")
        text: librarian.getStringFromRessource(":text/whatsnew.html")
        onOpened: globalSettings.lastWhatsNewHash = librarian.getStringHashFromRessource(":text/whatsnew.html")
    }
    
    Shortcut {
        sequence: StandardKey.Quit
        onActivated: Qt.quit()
    }

    Shortcut {
        sequence: StandardKey.Close
        onActivated: Qt.quit()
    }

    SensorGesture {
        id: sensorGesture
        enabled: Qt.application.state === Qt.ApplicationActive
        gestures: ["QtSensors.turnover"]
        onDetected: mobileAdaptor.vibrateBrief()
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
    } // onClosing
}
