/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import "../dialogs"
import "../items"

Page {
    id: settingsPage
    title: qsTr("Settings")


    header: ToolBar {

        Material.foreground: "white"
        height: 60 + SafeInsets.top
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: SafeInsets.top

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                global.platformAdaptor().vibrateBrief()
                stackView.pop()
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: headerMenuToolButton.left

            text: stackView.currentItem.title
            elide: Label.ElideRight
            font.pixelSize: 20
            verticalAlignment: Qt.AlignVCenter
        }

        ToolButton {
            id: headerMenuToolButton

            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_info_outline.svg"
            onClicked: {
                global.platformAdaptor().vibrateBrief()
                openManual("03-reference/settings.html")
            }
        }

    }

    ScrollView {
        anchors.fill: parent
        anchors.topMargin: view.font.pixelSize
        anchors.bottomMargin: SafeInsets.bottom
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right

        contentWidth: availableWidth // Disable horizontal scrolling

        clip: true

        GridLayout {
            columns: 2
            width: settingsPage.width - SafeInsets.left - SafeInsets.right

            Item { // Spacer
                Layout.columnSpan: 2
                height: 3
            }

            Label {
                Layout.leftMargin: view.font.pixelSize
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: qsTr("Moving Map")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                id: hideUpperAsp
                text: {
                    var secondLineString = ""
                    var altitudeLimit = GlobalSettings.airspaceAltitudeLimit
                    if (!altitudeLimit.isFinite()) {
                        secondLineString = qsTr("Currently showing all airspaces")
                    } else {
                        // Mention
                        Navigator.aircraft.verticalDistanceUnit

                        var airspaceAltitudeLimit = GlobalSettings.airspaceAltitudeLimit
                        var airspaceAltitudeLimitString = Navigator.aircraft.verticalDistanceToString(airspaceAltitudeLimit)
                        secondLineString = qsTr("Currently showing airspaces up to %1").arg(airspaceAltitudeLimitString)
                    }
                    return qsTr("Airspace Altitude Limit") +
                            `<br><font color="#606060" size="2">` +
                            secondLineString +
                            `</font>`

                }
                icon.source: "/icons/material/ic_map.svg"
                Layout.fillWidth: true
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    heightLimitDialog.open()
                }
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    helpDialog.title = qsTr("Airspace Altitude Limit")
                    helpDialog.text = "<p>"+qsTr("If you never fly higher than 5.000ft, you will probably not be interested in airspaces that begin above FL100. Enroute Flight Navigation allows you to set an altitude limit to improve the readability of the moving map. Once set, the app will show only airspaces below that limit. Tap on the entry “Airspace Altitude Limit” to set or unset the altitude limit.")+"</p>"
                            +"<p>"+qsTr("Once you set an altitude limit, the moving map will display a little warning (“Airspaces up to 9,500 ft”) to remind you that the moving map does not show all airspaces. The app will automatically increase the limit when your aircraft approaches the altitude limit from below.")+"</p>"
                    helpDialog.open()
                }
            }

            Label {
                Layout.leftMargin: view.font.pixelSize
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: qsTr("Map Features")
                font.bold: true
                color: Material.accent
            }

            WordWrappingSwitchDelegate {
                id: glidingSectors
                text: qsTr("Gliding Sectors")
                icon.source: "/icons/material/ic_map.svg"
                Layout.fillWidth: true
                Component.onCompleted: {
                    glidingSectors.checked = !GlobalSettings.hideGlidingSectors
                }
                onToggled: {
                    global.platformAdaptor().vibrateBrief()
                    GlobalSettings.hideGlidingSectors = !glidingSectors.checked
                }
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    helpDialog.title = qsTr("Gliding Sectors")
                    helpDialog.text = "<p>"+qsTr("In regions with high glider traffic, local regulations often allow gliders to fly in airspaces that are otherwise difficult to access, such as control zones. The moving map displays these “Gliding Sectors” in bright yellow. If you are not flying a glider, the gliding sectors are probably not relevant. Hiding the gliding sectors might improve the readability of the moving map.")+"</p>"
                    helpDialog.open()
                }
            }

            WordWrappingSwitchDelegate {
                id: hillshading
                text: qsTr("Hillshading")
                icon.source: "/icons/material/ic_map.svg"
                Layout.fillWidth: true
                Component.onCompleted: {
                    hillshading.checked = GlobalSettings.hillshading
                }
                onToggled: {
                    global.platformAdaptor().vibrateBrief()
                    GlobalSettings.hillshading = hillshading.checked
                }
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    helpDialog.title = qsTr("Hillshading")
                    helpDialog.text = "<p>"+qsTr("We have received a report from one user, who complained about issues with the hillshading graphics on very old devices, potentially because of buggy system libraries. If you experience problems, use this switch to disable the hillshading feature.")+"</p>"
                    helpDialog.open()
                }
            }

            Label {
                Layout.leftMargin: view.font.pixelSize
                Layout.fillWidth: true
                Layout.columnSpan: 2
                text: qsTr("Navigation Bar")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                id: showAltAGL
                text: {
                    const line1 = qsTr("Altimeter Mode")
                    const line2 = GlobalSettings.showAltitudeAGL ? qsTr("Currently showing altitude AGL") : qsTr("Currently showing altitude AMSL")
                    return line1 + `<br><font color="#606060" size="2">` + line2 + `</font>`
                }

                icon.source: "/icons/material/ic_speed.svg"
                Layout.fillWidth: true
                onClicked: altimeterDialog.open()
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    helpDialog.title = qsTr("Altimeter Mode")
                    helpDialog.text = "<p>"+qsTr("Use this settings item to chose if the altimeter shows height above ground level (AGL) or height above main sea level (AMSL).")+"</p>"
                    helpDialog.open()
                }
            }

            Label {
                Layout.leftMargin: view.font.pixelSize
                Layout.columnSpan: 2
                text: qsTr("System")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                id: trafficDataReceiverPositioning
                text: {
                    var secondLineString = ""
                    if (GlobalSettings.positioningByTrafficDataReceiver) {
                        secondLineString = qsTr("Currently using traffic data receiver")
                    } else {
                        secondLineString = qsTr("Currently using built-in satnav receiver")
                    }
                    return qsTr("Primary Position Data Source") +
                            `<br><font color="#606060" size="2">` +
                            secondLineString +
                            `</font>`
                }
                icon.source: "/icons/material/ic_satellite.svg"
                Layout.fillWidth: true
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    primaryPositionDataSourceDialog.open()
                }
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    helpDialog.title = qsTr("Primary Position Data Source")
                    helpDialog.text = "<p>" + qsTr("Enroute Flight Navigation can either use the built-in satnav receiver of your device or a connected traffic receiver as a primary position data source. This setting is essential if your device has reception problems or if you use Enroute Flight Navigation together with a flight simulator.") + "</p>"
                            + "<p>" + qsTr("You will most likely prefer the built-in satnav receiver for actual flight. The built-in receiver provides one position update per second on a typical Android system, while traffic receivers do not always provide timely position updates.") + "</p>"
                            + "<p>" + qsTr("If you use Enroute Flight Navigation together with a flight simulator, you must choose the traffic receiver as a primary position data source. Flight simulators broadcast position information of simulated aircraft via Wi-Fi, using the same protocol that a traffic data receiver would use in a real plane. As long as the built-in satnav receiver is selected, all position information provided by your flight simulator is ignored.") + "</p>"
                    helpDialog.open()
                }
            }

            WordWrappingSwitchDelegate {
                id: nightMode
                text: qsTr("Night Mode")
                icon.source: "/icons/material/ic_brightness_3.svg"
                Layout.fillWidth: true
                Component.onCompleted: {
                    nightMode.checked = GlobalSettings.nightMode
                }
                onToggled: {
                    global.platformAdaptor().vibrateBrief()
                    GlobalSettings.nightMode = nightMode.checked
                }
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    helpDialog.title = qsTr("Night Mode")
                    helpDialog.text = "<p>" + qsTr("The “Night Mode” of Enroute Flight Navigation is similar to the “Dark Mode” found in many other apps. We designed the night mode for pilots performing VFR flights by night, whose eyes have adapted to the darkness. Compared with other apps, you will find that the display is quite dark indeed.") + "</p>"
                    helpDialog.open()
                }
            }

            WordWrappingSwitchDelegate {
                id: ignoreSSL
                text: qsTr("Ignore Network Security Errors")
                icon.source: "/icons/material/ic_lock.svg"
                Layout.fillWidth: true
                visible: GlobalSettings.ignoreSSLProblems
                Component.onCompleted: {
                    ignoreSSL.checked = GlobalSettings.ignoreSSLProblems
                }
                onToggled: {
                    global.platformAdaptor().vibrateBrief()
                    GlobalSettings.ignoreSSLProblems = ignoreSSL.checked
                }
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                visible: GlobalSettings.ignoreSSLProblems
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    helpDialog.title = qsTr("Ignore Network Security Errors")
                    helpDialog.text = "<p>" + qsTr("This entry is visible if you have asked the app to download data via insecure internet connections after a secure connection attempt failed. Uncheck this item to revert to the standard policy, which enforces secure connections.") + "</p>"
                    helpDialog.open()
                }
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                icon.source: "/icons/material/ic_lock.svg"
                text: qsTr("Clear Password Storage")
                onClicked: clearPasswordDialog.open()
                visible: !global.passwordDB().empty
            }
            ToolButton {
                icon.source: "/icons/material/ic_info_outline.svg"
                visible: !global.passwordDB().empty
                onClicked: {
                    global.platformAdaptor().vibrateBrief()
                    helpDialog.title = qsTr("Clear Password Storage")
                    helpDialog.text = "<p>" + qsTr("This entry is visible if you have connected to a traffic data receiver that requires a password in addition to the Wi-Fi password and if you have asked the app to remember the password. Tap on this entry to clear the password storage.") + "</p>"
                    helpDialog.open()
                }
            }

            Label {
                Layout.leftMargin: view.font.pixelSize
                Layout.columnSpan: 2
                text: qsTr("Help")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                Layout.columnSpan: 2
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your traffic receiver…")
                onClicked: openManual("02-steps/traffic.html")
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                Layout.columnSpan: 2
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your flight simulator…")
                onClicked: openManual("02-steps/simulator.html")
            }

            Item { // Spacer
                Layout.columnSpan: 2
                height: 3
            }

        }
    }

    LongTextDialog {
        id: helpDialog

        modal: true

        standardButtons: Dialog.Ok
    }

    LongTextDialog {
        id: missingTerrainDataWarning

        title: qsTr("Terrain Data Missing")
        text: qsTr("The height above ground level cannot be computed for your current position, because the relevant terrain maps for your region have not been installed.")

        footer: DialogButtonBox {
            ToolButton {
                text: qsTr("Install now")
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
            ToolButton {
                text: qsTr("Cancel")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }
        } // DialogButtonBox

        onRejected: {
            global.platformAdaptor().vibrateBrief()
            showAltAGL.checked = false
            close()
        }

        onAccepted: {
            global.platformAdaptor().vibrateBrief()
            close()
            stackView.pop()
            stackView.push("../pages/DataManager.qml")
        }
    }

    CenteringDialog {
        id: clearPasswordDialog

        title: qsTr("Clear Password Storage?")
        modal: true

        Label {
            width: clearPasswordDialog.availableWidth

            text: qsTr("Once the storage is cleared, the passwords can no longer be retrieved.")
            wrapMode: Text.Wrap
        }

        footer: DialogButtonBox {
            ToolButton {
                text: qsTr("Clear")
                DialogButtonBox.buttonRole: DialogButtonBox.AcceptRole
            }
            ToolButton {
                text: qsTr("Cancel")
                DialogButtonBox.buttonRole: DialogButtonBox.RejectRole
            }

        }

        onAccepted: {
            global.passwordDB().clear()
            toast.doToast(qsTr("Password storage cleared"))
        }

    }

    CenteringDialog {
        id: heightLimitDialog

        modal: true
        title: qsTr("Airspace Altitude Limit")
        standardButtons: (slider.from < slider.to) ? Dialog.Ok|Dialog.Cancel : Dialog.Cancel

        ColumnLayout {
            width: heightLimitDialog.availableWidth

            Label {
                text: qsTr("Set an altitude limit to improve the readability of the moving map. Once set, the app will show only airspaces below that limit.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            SwitchDelegate {
                id: altLimitCheck
                enabled: slider.from < slider.to
                text: qsTr("Set altitude limit")
                Layout.fillWidth: true
            }

            Slider {
                id: slider
                Layout.fillWidth: true
                enabled: (from < to) && (altLimitCheck.checked)
                from: {
                    var positionInfo = global.positionProvider().positionInfo
                    if (!positionInfo.isValid())
                        return GlobalSettings.airspaceAltitudeLimit_min.toFeet()
                    var trueAlt = positionInfo.trueAltitudeAMSL()
                    if (!trueAlt.isFinite())
                        return GlobalSettings.airspaceAltitudeLimit_min.toFeet()
                    return Math.min(GlobalSettings.airspaceAltitudeLimit_max.toFeet(), 500.0*Math.ceil(trueAlt.toFeet()/500.0+2))
                }
                to: GlobalSettings.airspaceAltitudeLimit_max.toFeet()

                stepSize: 500
            }

            Label {
                enabled: slider.from < slider.to
                text: {
                    if (altLimitCheck.checked) {
                        return qsTr("Show airspaces up to %1 ft / %2 m.").arg(slider.value.toLocaleString()).arg( (slider.value/3.2808).toLocaleString(Qt.locale(),'f',0) )
                    } else {
                        return qsTr("No limit, all airspaces shown")
                    }
                }
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            Label {
                visible: slider.from >= slider.to
                text: qsTr("Cannot set reasonable airspaces altitude limit because the present own altitude is too high.")
                color: "red"
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }



        }

        onAccepted: {
            if (altLimitCheck.checked) {
                GlobalSettings.airspaceAltitudeLimit = distance.fromFT(slider.value)
            } else {
                GlobalSettings.airspaceAltitudeLimit = distance.fromFT(99999)
            }
        }

        onAboutToShow: {
            altLimitCheck.checked = (GlobalSettings.airspaceAltitudeLimit.toM() < GlobalSettings.airspaceAltitudeLimit_max.toM())
            slider.value = GlobalSettings.lastValidAirspaceAltitudeLimit.toFeet()
        }

    }

    CenteringDialog {
        id: primaryPositionDataSourceDialog

        modal: true
        title: qsTr("Position Data Source")
        standardButtons: Dialog.Ok|Dialog.Cancel

        ColumnLayout {
            width: primaryPositionDataSourceDialog.availableWidth

            Label {
                text: qsTr("Most users will choose the built-in satnav receiver. Choose the traffic data receiver when the satnav receiver of your device has reception problems, or when you use this app together with a flight simulator.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            WordWrappingCheckDelegate {
                id: a
                text: qsTr("Built-in satnav receiver")
                Layout.fillWidth: true
                checked: !GlobalSettings.positioningByTrafficDataReceiver
                onCheckedChanged: b.checked = !checked
            }

            WordWrappingCheckDelegate {
                id: b
                text: qsTr("Traffic data reveiver (when available)")
                Layout.fillWidth: true
                onCheckedChanged: a.checked = !checked
            }
        }

        onAboutToShow: {
            a.checked = !GlobalSettings.positioningByTrafficDataReceiver
            b.checked = !a.checked
        }

        onAccepted: GlobalSettings.positioningByTrafficDataReceiver = b.checked

    }

    CenteringDialog {
        id: altimeterDialog

        modal: true

        title: qsTr("Altimeter Mode")
        standardButtons: Dialog.Ok|Dialog.Cancel

        ColumnLayout {
            width: altimeterDialog.availableWidth

            Label {
                text: qsTr("This setting applies to the altimeter in the Navigation Bar, at the bottom of the moving map screen.")
                Layout.fillWidth: true
                wrapMode: Text.Wrap
            }

            WordWrappingCheckDelegate {
                id: a1
                text: qsTr("Height above ground level (AGL)")
                Layout.fillWidth: true
                checked: GlobalSettings.showAltitudeAGL
                onCheckedChanged: b1.checked = !checked
            }

            WordWrappingCheckDelegate {
                id: b1
                text: qsTr("Height above main sea level (AMSL)")
                Layout.fillWidth: true
                onCheckedChanged: a1.checked = !checked
            }
        }

        onAboutToShow: {
            a1.checked = GlobalSettings.showAltitudeAGL
            b1.checked = !a1.checked
        }

        onAccepted: GlobalSettings.showAltitudeAGL = a1.checked

    }

} // Page
