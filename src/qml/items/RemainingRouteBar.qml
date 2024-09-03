/*************************************%**************************************
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
import QtQuick.Layouts

import akaflieg_freiburg.enroute

Rectangle {
    id: baseRect

    // Remaining route info shown in this item
    property var rri: Navigator.remainingRouteInfo

    implicitHeight: grid.implicitHeight + SafeInsets.top
    Behavior on implicitHeight { NumberAnimation { duration: 100 } }

    clip: true

    // True course is only visible if display is wide enough
    property bool tcVisible: baseRect.width > 25*dummyControl.font.pixelSize

    color: "#AA000000"

    // Dummy control, used to glean the font size
    Control {
        id: dummyControl
        visible: false
    }

    GridLayout {
        id: grid

        anchors.fill: parent
        anchors.leftMargin: dummyControl.font.pixelSize + SafeInsets.left
        anchors.topMargin: SafeInsets.top + 0.2*dummyControl.font.pixelSize
        anchors.rightMargin: dummyControl.font.pixelSize + SafeInsets.right

        Layout.fillWidth: true

        rowSpacing: 0
        columns: baseRect.tcVisible ? 5 : 4

        NotificationArea {
            Layout.columnSpan: grid.columns
            Layout.fillWidth: true
            Layout.maximumWidth: 40*dummyControl.font.pixelSize
            Layout.alignment: Qt.AlignHCenter
            Layout.bottomMargin: 0.2*dummyControl.font.pixelSize
        }

        Item { Layout.preferredWidth: 1 }
        Item {
            Layout.fillWidth: true
            visible: baseRect.tcVisible
        }
        Item { Layout.fillWidth: true }
        Item { Layout.fillWidth: true }
        Item { Layout.fillWidth: true }

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            visible: baseRect.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: dummyControl.font.pixelSize*0.9
        }
        Label {
            text: "TC"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: baseRect.tcVisible && (baseRect.rri.status === RemainingRouteInfo.OnRoute)
            font.pixelSize: dummyControl.font.pixelSize*0.9
        }
        Label {
            text: "DIST"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: baseRect.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: dummyControl.font.pixelSize*0.9
        }
        Label {
            text: "ETE"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: baseRect.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: dummyControl.font.pixelSize*0.9
        }
        Label {
            text: "ETA"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: baseRect.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: dummyControl.font.pixelSize*0.9
        }

        Label {
            text: baseRect.rri.nextWP.shortName
            elide: Text.ElideRight
            color: "white"
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth
            visible: baseRect.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: dummyControl.font.pixelSize*1.3
        }
        Label {
            text: {
                var tt = baseRect.rri.nextWP_TC
                return tt.isFinite() ? Math.round(tt.toDEG()) + "°" : "-"
            }
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: baseRect.tcVisible && (baseRect.rri.status === RemainingRouteInfo.OnRoute)
            font.weight: Font.Bold
            font.pixelSize: dummyControl.font.pixelSize*1.3
        }
        Label {
            text: Navigator.aircraft.horizontalDistanceToString(baseRect.rri.nextWP_DIST)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: baseRect.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: dummyControl.font.pixelSize*1.3
        }
        Label {
            text: "%1 h".arg(baseRect.rri.nextWP_ETE.toHoursAndMinutes())
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: baseRect.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: dummyControl.font.pixelSize*1.3
        }
        Label {
            text: baseRect.rri.nextWP_ETAAsUTCString
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: baseRect.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: dummyControl.font.pixelSize*1.3
        }

        Label {
            text: baseRect.rri.finalWP.shortName
            elide: Text.ElideRight
            color: "white"
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth
            visible: (baseRect.rri.status === RemainingRouteInfo.OnRoute) && baseRect.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: dummyControl.font.pixelSize*1.3
        }
        Item {
            visible: baseRect.tcVisible && (baseRect.rri.status === RemainingRouteInfo.OnRoute)
        }
        Label {
            text: Navigator.aircraft.horizontalDistanceToString(baseRect.rri.finalWP_DIST)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: (baseRect.rri.status === RemainingRouteInfo.OnRoute) && baseRect.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: dummyControl.font.pixelSize*1.3
        }
        Label {
            text: "%1 h".arg(baseRect.rri.finalWP_ETE.toHoursAndMinutes())
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: (baseRect.rri.status === RemainingRouteInfo.OnRoute) && baseRect.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: dummyControl.font.pixelSize*1.3
        }
        Label {
            text: baseRect.rri.finalWP_ETAAsUTCString
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: (baseRect.rri.status === RemainingRouteInfo.OnRoute) && baseRect.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: dummyControl.font.pixelSize*1.3
        }

        Label {
            Layout.columnSpan: grid.columns
            Layout.fillWidth: true
            visible: text !== ""
            color: "white"
            wrapMode: Text.Wrap
            horizontalAlignment:  Text.AlignHCenter

            property leg staticLeg

            text: {
                switch (baseRect.rri.status) {
                case RemainingRouteInfo.PositionUnknown:
                    return qsTr("Position unknown.");
                case RemainingRouteInfo.OffRoute:
                    return qsTr("More than %1 off route.").arg(Navigator.aircraft.horizontalDistanceToString(staticLeg.nearThreshold));
                case RemainingRouteInfo.NearDestination:
                    return qsTr("Near destination.");
                }
                return baseRect.rri.note
            }

        }

        Item {
            Layout.columnSpan: grid.columns
            Layout.fillWidth: true
            Layout.preferredHeight: 0.2*dummyControl.font.pixelSize

            visible: baseRect.rri.status !== RemainingRouteInfo.NoRoute
        }

    }


} // Rectangle
