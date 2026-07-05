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

import QtQuick
// Qualified import: the unqualified name "Scale" is taken by the map scale
// indicator from akaflieg_freiburg.enroute, but the transform below needs
// QtQuick's Scale.
import QtQuick as Quick
import QtQuick.Controls
import QtQuick.Layouts

import akaflieg_freiburg.enroute


Rectangle {
    id: grid

    color: "#303030"

    implicitHeight: trueAltitude.implicitHeight + SafeInsets.bottom

    // Dummy control. Used to glean the font size.
    Control {
        id: dummy
        visible: false
    }

    // Fixed worst-case width for the UTC readout ("H:mm"). Using the live
    // clock string here would let the ticking time participate in the layout
    // decision and make items flicker in and out.
    TextMetrics {
        id: utcMetrics
        font: utc_1.font
        text: "88:88"
    }

    // When the pressure altitude readout has data to show, it is more
    // important in flight than the UTC clock; when it would only show "-",
    // it is the least important item. The drop priority switches accordingly.
    readonly property bool pAltHasData: PositionProvider.pressureAltitude.isFinite() && !PositionProvider.pressureAltitude.isNegative()

    readonly property real gapWidth: dummy.font.pixelSize
    readonly property real availableWidth: width - SafeInsets.left - SafeInsets.right

    // Layout decision: the number of items to show, and a scale factor that
    // is applied to the row. Before an item is dropped, the row is allowed to
    // shrink to 90%; only if that is not sufficient, the least important item
    // (see pAltHasData) disappears and the cascade restarts at full size.
    readonly property var barLayout: {
        var n3 = trueAltitude.m_implicitWidth + groundSpeed.m_implicitWidth + trueTrack.m_implicitWidth + 3*gapWidth
        var w4 = pAltHasData ? flightLevel.m_implicitWidth : utc.m_implicitWidth
        var w5 = pAltHasData ? utc.m_implicitWidth : flightLevel.m_implicitWidth
        var n4 = n3 + w4 + gapWidth
        var n5 = n4 + w5 + gapWidth
        if (n5 <= availableWidth)
            return {items: 5, scale: 1}
        if (0.9*n5 <= availableWidth)
            return {items: 5, scale: availableWidth/n5}
        if (n4 <= availableWidth)
            return {items: 4, scale: 1}
        if (0.9*n4 <= availableWidth)
            return {items: 4, scale: availableWidth/n4}
        return {items: 3, scale: Math.max(0.9, Math.min(1, availableWidth/n3))}
    }


    RowLayout {
        id: row

        // The row is laid out at full size in a correspondingly wider
        // coordinate system and then scaled down visually. Scaling the
        // transform instead of the font keeps the text measurements
        // (contentWidth) independent of the layout decision, which would
        // otherwise be circular.
        x: SafeInsets.left
        y: 0
        width: grid.availableWidth / grid.barLayout.scale
        height: parent.height - SafeInsets.bottom
        transform: Quick.Scale {
            origin.x: 0
            origin.y: row.height/2
            xScale: grid.barLayout.scale
            yScale: grid.barLayout.scale
        }

        Item { Layout.fillWidth: true }

        ColumnLayout {
            id: trueAltitude

            Layout.preferredWidth: m_implicitWidth
            property real m_implicitWidth: Math.max(trueAltitude_1.contentWidth, trueAltitude_2.contentWidth)


            TapHandler {
                onTapped: {
                    GlobalSettings.showAltitudeAGL = !GlobalSettings.showAltitudeAGL
                    if (GlobalSettings.showAltitudeAGL)
                        Global.toast.doToast(qsTr("Showing Altitude Above Ground Level"))
                    else
                        Global.toast.doToast(qsTr("Showing Altitude Above Main Sea Level"))
                }
            }

            Label {
                id: trueAltitude_1

                Layout.alignment: Qt.AlignHCenter

                text: {
                    // Mention
                    Navigator.aircraft.verticalDistanceUnit

                    if (GlobalSettings.showAltitudeAGL) {
                        const talt = PositionProvider.positionInfo.trueAltitudeAGL();
                        return Navigator.aircraft.verticalDistanceToString(talt)
                    }

                    const talt = PositionProvider.positionInfo.trueAltitudeAMSL();
                    return Navigator.aircraft.verticalDistanceToString(talt)
                }
                font.weight: Font.Bold
                font.pixelSize: dummy.font.pixelSize*1.3
                color: "white"
            }
            Label {
                id: trueAltitude_2

                Layout.alignment: Qt.AlignHCenter

                color: "white"
                text: GlobalSettings.showAltitudeAGL ? "T.ALT AGL" : "T.ALT AMSL"
                font.pixelSize: dummy.font.pixelSize*0.9
            }
        }

        Item { Layout.fillWidth: true }

        ColumnLayout {
            id: flightLevel

            visible: (grid.barLayout.items >= 5) || ((grid.barLayout.items === 4) && grid.pAltHasData)
            Layout.preferredWidth: visible ? m_implicitWidth : 0
            property real m_implicitWidth: Math.max(flightLevel_1.contentWidth, flightLevel_2.contentWidth)

            Label {
                id: flightLevel_1

                Layout.alignment: Qt.AlignHCenter

                text: {
                    if (!PositionProvider.pressureAltitude.isFinite())
                        return "-"
                    if (PositionProvider.pressureAltitude.isNegative())
                        return "-"
                    return "FL" + ("000" + Math.round(PositionProvider.pressureAltitude.toFeet()/100.0)).slice(-3)
                }
                font.weight: Font.Bold
                font.pixelSize: dummy.font.pixelSize*1.3
                color: "white"
            }
            Label {
                id: flightLevel_2

                Layout.alignment: Qt.AlignHCenter

                color: "white"
                text: "P.ALT"
                font.pixelSize: dummy.font.pixelSize*0.9

            }
        }

        Item { Layout.fillWidth: flightLevel.visible }

        ColumnLayout {
            id: groundSpeed

            Layout.preferredWidth: m_implicitWidth
            property real m_implicitWidth: Math.max(groundSpeed_1.contentWidth, groundSpeed_2.contentWidth)

            Label {
                id: groundSpeed_1

                Layout.alignment: Qt.AlignHCenter

                text: Navigator.aircraft.horizontalSpeedToString( PositionProvider.positionInfo.groundSpeed() )
                font.weight: Font.Bold
                font.pixelSize: dummy.font.pixelSize*1.3
                color: "white"
            }
            Label {
                id: groundSpeed_2
                Layout.alignment: Qt.AlignHCenter

                text: "GS"
                color: "white"
                font.pixelSize: dummy.font.pixelSize*0.9
            }
        }

        Item { Layout.fillWidth: true }

        ColumnLayout {
            id: trueTrack

            Layout.preferredWidth: m_implicitWidth
            property real m_implicitWidth: Math.max(trueTrack_1.contentWidth, trueTrack_2.contentWidth)

            Label {
                id: trueTrack_1

                Layout.alignment: Qt.AlignHCenter

                text: {
                    var tt = PositionProvider.positionInfo.trueTrack();
                    return tt.isFinite() ? Math.round(tt.toDEG()) + "°" : "-"
                }

                font.weight: Font.Bold
                font.pixelSize: dummy.font.pixelSize*1.3
                color: "white"
            }
            Label {
                id: trueTrack_2

                Layout.alignment: Qt.AlignHCenter

                text: "TT"
                color: "white"
                font.pixelSize: dummy.font.pixelSize*0.9
            }
        }

        Item { Layout.fillWidth: true }

        ColumnLayout {
            id: utc

            visible: (grid.barLayout.items >= 5) || ((grid.barLayout.items === 4) && !grid.pAltHasData)
            Layout.preferredWidth: visible ? m_implicitWidth : 0
            property real m_implicitWidth: Math.max(utcMetrics.width, utc_2.contentWidth)

            Label {
                id: utc_1
                Layout.alignment: Qt.AlignHCenter

                text: Clock.timeAsUTCString
                font.weight: Font.Bold
                font.pixelSize: dummy.font.pixelSize*1.3
                color: "white"
            } // Label
            Label {
                id: utc_2
                Layout.alignment: Qt.AlignHCenter

                text: "UTC"
                color: "white"
                font.pixelSize: dummy.font.pixelSize*0.9
            }
        }

        Item { Layout.fillWidth: utc.visible }

    }

}
