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
import QtQuick.Layouts 1.15


Rectangle {
    id: grid
    
    color: "#AA000000"

    height: trueAltitude.implicitHeight
    

    function numVisibleItems() {
        var w = trueAltitude.m_implicitWidth + groundSpeed.m_implicitWidth + trueTrack.m_implicitWidth + utc.m_implicitWidth + 4*Qt.application.font.pixelSize
        if (w > grid.width)
            return 3
        w = w + flightLevel.m_implicitWidth + Qt.application.font.pixelSize
        if (w > grid.width)
            return 4
        return 5
    }

    RowLayout {
        anchors.fill: parent

        Item { Layout.fillWidth: true }

        ColumnLayout {
            id: trueAltitude

            Layout.preferredWidth: m_implicitWidth
            property var m_implicitWidth: Math.max(trueAltitude_1.contentWidth, trueAltitude_2.contentWidth)

            Label {
                id: trueAltitude_1

                Layout.alignment: Qt.AlignHCenter

                text: {
                    const talt= positionProvider.positionInfo.trueAltitude();
                    return talt.isFinite() ? Math.round(talt.toFeet()) + " ft" : "-"
                }
                font.weight: Font.Bold
                font.pixelSize: Qt.application.font.pixelSize*1.3
                color: "white"
            }
            Label {
                id: trueAltitude_2

                Layout.alignment: Qt.AlignHCenter

                color: "white"
                text: "T.ALT"
            }
        }

        Item { Layout.fillWidth: true }

        ColumnLayout {
            id: flightLevel

            visible: grid.numVisibleItems() >= 5
            Layout.preferredWidth: visible ? m_implicitWidth : 0
            property var m_implicitWidth: Math.max(flightLevel_1.contentWidth, flightLevel_2.contentWidth)

            Label {
                id: flightLevel_1

                Layout.alignment: Qt.AlignHCenter

                text: positionProvider.pressureAltitude.isFinite() ? "FL" + ("000" + Math.round(positionProvider.pressureAltitude.toFeet()/100.0)).slice(-3) : "-"
                font.weight: Font.Bold
                font.pixelSize: Qt.application.font.pixelSize*1.3
                color: "white"
            }
            Label {
                id: flightLevel_2

                Layout.alignment: Qt.AlignHCenter

                color: "white"
                text: "FL"
            }
        }

        Item { Layout.fillWidth: flightLevel.visible }

        ColumnLayout {
            id: groundSpeed

            Layout.preferredWidth: m_implicitWidth
            property var m_implicitWidth: Math.max(groundSpeed_1.contentWidth, groundSpeed_2.contentWidth)

            Label {
                id: groundSpeed_1

                Layout.alignment: Qt.AlignHCenter

                text: {
                    const gs = positionProvider.positionInfo.groundSpeed();
                    if (!gs.isFinite())
                        return "-"
                    return globalSettings.useMetricUnits ? Math.round(gs.toKMH()) + " km/h" : Math.round(gs.toKN()) + " kn"
                }
                font.weight: Font.Bold
                font.pixelSize: Qt.application.font.pixelSize*1.3
                color: "white"
            }
            Label {
                id: groundSpeed_2
                Layout.alignment: Qt.AlignHCenter

                text: "GS"
                color: "white"
            }
        }

        Item { Layout.fillWidth: true }

        ColumnLayout {
            id: trueTrack

            Layout.preferredWidth: m_implicitWidth
            property var m_implicitWidth: Math.max(trueTrack_1.contentWidth, trueTrack_2.contentWidth)

            Label {
                id: trueTrack_1

                Layout.alignment: Qt.AlignHCenter

                text: {
                    var tt = positionProvider.positionInfo.trueTrack();
                    return tt.isFinite() ? Math.round(tt.toDEG()) + "°" : "-"
                }

                font.weight: Font.Bold
                font.pixelSize: Qt.application.font.pixelSize*1.3
                color: "white"
            }
            Label {
                id: trueTrack_2

                Layout.alignment: Qt.AlignHCenter

                text: "TT"
                color: "white"
            }
        }

        Item { Layout.fillWidth: true }

        ColumnLayout {
            id: utc

            visible: grid.numVisibleItems() >= 4
            Layout.preferredWidth: m_implicitWidth
            property var m_implicitWidth: Math.max(utc_1.contentWidth, utc_2.contentWidth)

            Label {
                id: utc_1
                Layout.alignment: Qt.AlignHCenter

                text: clock.timeAsUTCString
                font.weight: Font.Bold
                font.pixelSize: Qt.application.font.pixelSize*1.3
                color: "white"
            } // Label
            Label {
                id: utc_2
                Layout.alignment: Qt.AlignHCenter

                text: "UTC"
                color: "white"
            }
        }

        Item { Layout.fillWidth: utc.visible }

    }

} // Rectangle
