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
import QtQuick.Layouts

import akaflieg_freiburg.enroute

Rectangle {
    id: grid
    
    color: "#AA000000"

    height: trueAltitude.implicitHeight + SafeInsets.bottom
    
    // Dummy control. Used to glean the font size.
    Control {
        id: dummy
        visible: false
    }

    function numVisibleItems() {
        var w = trueAltitude.m_implicitWidth + groundSpeed.m_implicitWidth + trueTrack.m_implicitWidth + utc.m_implicitWidth + 4*dummy.font.pixelSize
        if (w > grid.width)
            return 3
        w = w + flightLevel.m_implicitWidth + dummy.font.pixelSize
        if (w > grid.width)
            return 4
        return 5
    }


    RowLayout {
        anchors.fill: parent
        anchors.bottomMargin: SafeInsets.bottom
        anchors.leftMargin: SafeInsets.left
        anchors.rightMargin: SafeInsets.right

        Item { Layout.fillWidth: true }

        ColumnLayout {
            id: trueAltitude

            Layout.preferredWidth: m_implicitWidth
            property real m_implicitWidth: Math.max(trueAltitude_1.contentWidth, trueAltitude_2.contentWidth)


            TapHandler {
                onTapped: {
                    GlobalSettings.showAltitudeAGL = !GlobalSettings.showAltitudeAGL
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

            visible: grid.numVisibleItems() >= 5
            Layout.preferredWidth: visible ? m_implicitWidth : 0
            property real m_implicitWidth: Math.max(flightLevel_1.contentWidth, flightLevel_2.contentWidth)

            Label {
                id: flightLevel_1

                Layout.alignment: Qt.AlignHCenter

                text: PositionProvider.pressureAltitude.isFinite() ? "FL" + ("000" + Math.round(PositionProvider.pressureAltitude.toFeet()/100.0)).slice(-3) : "-"
                font.weight: Font.Bold
                font.pixelSize: dummy.font.pixelSize*1.3
                color: "white"
            }
            Label {
                id: flightLevel_2

                Layout.alignment: Qt.AlignHCenter

                color: "white"
                text: "FL"
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

            visible: grid.numVisibleItems() >= 4
            Layout.preferredWidth: m_implicitWidth
            property real m_implicitWidth: Math.max(utc_1.contentWidth, utc_2.contentWidth)

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

} // Rectangle
