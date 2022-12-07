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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import QtQuick.Layouts 1.15

import "../dialogs"
import "../items"

Page {
    id: settingsPage
    title: qsTr("Settings")

    header: StandardHeader {}

    ScrollView {
        id: view
        anchors.fill: parent
        anchors.topMargin: view.font.pixelSize
        contentWidth: availableWidth

        ColumnLayout {
            width: settingsPage.width
            implicitWidth: settingsPage.width

            Label {
                Layout.leftMargin: view.font.pixelSize
                Layout.fillWidth: true
                text: qsTr("Moving Map")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                id: hideUpperAsp
                text: {
                    var secondLineString = ""
                    var altitudeLimit = global.settings().airspaceAltitudeLimit
                    if (!altitudeLimit.isFinite()) {
                        secondLineString = qsTr("Currently showing all airspaces")
                    } else {
                        // Mention
                        global.navigator().aircraft.verticalDistanceUnit

                        var airspaceAltitudeLimit = global.settings().airspaceAltitudeLimit
                        var airspaceAltitudeLimitString = global.navigator().aircraft.verticalDistanceToString(airspaceAltitudeLimit)
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

            Label {
                Layout.leftMargin: view.font.pixelSize
                Layout.fillWidth: true
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
                    glidingSectors.checked = !global.settings().hideGlidingSectors
                }
                onToggled: {
                    global.platformAdaptor().vibrateBrief()
                    global.settings().hideGlidingSectors = !glidingSectors.checked
                }
            }

            WordWrappingSwitchDelegate {
                id: hillshading
                text: qsTr("Hillshading")
                icon.source: "/icons/material/ic_map.svg"
                Layout.fillWidth: true
                Component.onCompleted: {
                    hillshading.checked = global.settings().hillshading
                }
                onToggled: {
                    global.platformAdaptor().vibrateBrief()
                    global.settings().hillshading = hillshading.checked
                }
            }

            Label {
                Layout.leftMargin: view.font.pixelSize
                Layout.fillWidth: true
                text: qsTr("Navigation")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingSwitchDelegate {
                id: showAltAGL
                text: {
                    const firstLine = qsTr("Show Altitude AGL")
                    if (!checked) {
                        return firstLine + `<br><font color="#606060" size="2">` + qsTr("Currently showing altitude AMSL") + `</font>`
                    }
                    return firstLine
                }

                icon.source: "/icons/material/ic_speed.svg"
                Layout.fillWidth: true
                Component.onCompleted: {
                    showAltAGL.checked = global.settings().showAltitudeAGL
                }
                onToggled: {
                    global.platformAdaptor().vibrateBrief()
                    global.settings().showAltitudeAGL = showAltAGL.checked
                    var pInfo = global.positionProvider().positionInfo
                    if (showAltAGL.checked &&
                            pInfo.isValid() &&
                            !pInfo.terrainElevationAMSL().isFinite()) {
                        missingTerrainDataWarning.open()
                    }
                }
            }

            Label {
                Layout.leftMargin: view.font.pixelSize
                text: qsTr("System")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                id: trafficDataReceiverPositioning
                text: {
                    var secondLineString = ""
                    if (global.settings().positioningByTrafficDataReceiver) {
                        secondLineString = qsTr("Currently using traffic data receiver")
                    } else {
                        secondLineString = qsTr("Currently using built-in satnav receiver")
                    }
                    return qsTr("Primary position data source") +
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

            WordWrappingSwitchDelegate {
                id: nightMode
                text: qsTr("Night Mode")
                icon.source: "/icons/material/ic_brightness_3.svg"
                Layout.fillWidth: true
                Component.onCompleted: {
                    nightMode.checked = global.settings().nightMode
                }
                onToggled: {
                    global.platformAdaptor().vibrateBrief()
                    global.settings().nightMode = nightMode.checked
                }
            }

            WordWrappingSwitchDelegate {
                id: ignoreSSL
                text: qsTr("Ignore Network Security Errors")
                icon.source: "/icons/material/ic_lock.svg"
                Layout.fillWidth: true
                visible: global.settings().ignoreSSLProblems
                Component.onCompleted: {
                    ignoreSSL.checked = global.settings().ignoreSSLProblems
                }
                onToggled: {
                    global.platformAdaptor().vibrateBrief()
                    global.settings().ignoreSSLProblems = ignoreSSL.checked
                }
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                icon.source: "/icons/material/ic_lock.svg"
                text: qsTr("Clear Password Storage")
                onClicked: clearPasswordDialog.open()
                visible: !global.passwordDB().empty
            }

            Label {
                Layout.leftMargin: view.font.pixelSize
                text: qsTr("Help")
                font.pixelSize: view.font.pixelSize*1.2
                font.bold: true
                color: Material.accent
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your traffic receiver…")
                onClicked: openManual("02-steps/traffic.html")
            }

            WordWrappingItemDelegate {
                Layout.fillWidth: true
                icon.source: "/icons/material/ic_info_outline.svg"
                text: qsTr("How to connect your flight simulator…")
                onClicked: openManual("02-steps/simulator.html")
            }

            Item { // Spacer
                height: 3
            }

        } // ColumnLayout
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

    Dialog {
        id: clearPasswordDialog

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-view.font.pixelSize, 40*view.font.pixelSize)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        topMargin: view.font.pixelSize/2.0
        bottomMargin: view.font.pixelSize/2.0

        modal: true

        title: qsTr("Clear Password Storage?")

        Label {
            width: heightLimitDialog.availableWidth

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

        } // DialogButtonBox


        onAccepted: {
            global.passwordDB().clear()
            toast.doToast(qsTr("Password storage cleared"))
        }

    }

    Dialog {
        id: heightLimitDialog

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-view.font.pixelSize, 40*view.font.pixelSize)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        topMargin: view.font.pixelSize/2.0
        bottomMargin: view.font.pixelSize/2.0

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
                        return global.settings().airspaceAltitudeLimit_min.toFeet()
                    var trueAlt = positionInfo.trueAltitudeAMSL()
                    if (!trueAlt.isFinite())
                        return global.settings().airspaceAltitudeLimit_min.toFeet()
                    return Math.min(global.settings().airspaceAltitudeLimit_max.toFeet(), 500.0*Math.ceil(trueAlt.toFeet()/500.0+2))
                }
                to: global.settings().airspaceAltitudeLimit_max.toFeet()

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
                global.settings().airspaceAltitudeLimit = distance.fromFT(slider.value)
            } else {
                global.settings().airspaceAltitudeLimit = distance.fromFT(99999)
            }
        }

        onAboutToShow: {
            altLimitCheck.checked = (global.settings().airspaceAltitudeLimit.toM() < global.settings().airspaceAltitudeLimit_max.toM())
            slider.value = global.settings().lastValidAirspaceAltitudeLimit.toFeet()
        }

    }

    Dialog {
        id: primaryPositionDataSourceDialog

        // Size is chosen so that the dialog does not cover the parent in full
        width: Math.min(parent.width-view.font.pixelSize, 40*view.font.pixelSize)

        // Center in Overlay.overlay. This is a funny workaround against a bug, I believe,
        // in Qt 15.1 where setting the parent (as recommended in the Qt documentation) does not seem to work right if the Dialog is opend more than once.
        parent: Overlay.overlay
        x: (parent.width-width)/2.0
        y: (parent.height-height)/2.0

        topMargin: view.font.pixelSize/2.0
        bottomMargin: view.font.pixelSize/2.0

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
                checked: !global.settings().positioningByTrafficDataReceiver
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
            a.checked = !global.settings().positioningByTrafficDataReceiver
            b.checked = !a.checked
        }

        onAccepted: global.settings().positioningByTrafficDataReceiver = b.checked

    }

} // Page
