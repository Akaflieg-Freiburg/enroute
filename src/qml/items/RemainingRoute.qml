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
import QtQuick.Layouts

import akaflieg_freiburg.enroute
import enroute 1.0

Rectangle {   
    color: "#AA000000"
    height: grid.implicitHeight + SafeInsets.top

    visible: grid.rri.status !== RemainingRouteInfo.NoRoute

    GridLayout {
        id: grid

        anchors.fill: parent
        anchors.leftMargin: view.font.pixelSize + SafeInsets.left
        anchors.topMargin: SafeInsets.top
        anchors.rightMargin: view.font.pixelSize + SafeInsets.right
        rowSpacing: 0
        columns: 4

        property var rri: global.navigator().remainingRouteInfo

        Item { width: 1 }
        Item { Layout.fillWidth: true }
        Item { Layout.fillWidth: true }
        Item { Layout.fillWidth: true }

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: view.font.pixelSize*0.9
        }
        Label {
            text: "DIST"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: view.font.pixelSize*0.9
        }
        Label {
            text: "ETE"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: view.font.pixelSize*0.9
        }
        Label {
            text: "ETA"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: view.font.pixelSize*0.9
        }

        Label {
            text: grid.rri.nextWP.shortName
            elide: Text.ElideRight
            color: "white"
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: view.font.pixelSize*1.3
        }
        Label {
            text: global.navigator().aircraft.horizontalDistanceToString(grid.rri.nextWP_DIST)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: view.font.pixelSize*1.3
        }
        Label {
            text: "%1 h".arg(grid.rri.nextWP_ETE.toHoursAndMinutes())
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: view.font.pixelSize*1.3
        }
        Label {
            text: grid.rri.nextWP_ETAAsUTCString
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: view.font.pixelSize*1.3
        }


        Label {
            text: grid.rri.finalWP.shortName
            elide: Text.ElideRight
            color: "white"
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth
            visible: (grid.rri.status === RemainingRouteInfo.OnRoute) && grid.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: view.font.pixelSize*1.3
        }
        Label {
            text: global.navigator().aircraft.horizontalDistanceToString(grid.rri.finalWP_DIST)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: (grid.rri.status === RemainingRouteInfo.OnRoute) && grid.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: view.font.pixelSize*1.3
        }
        Label {
            text: "%1 h".arg(grid.rri.finalWP_ETE.toHoursAndMinutes())
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: (grid.rri.status === RemainingRouteInfo.OnRoute) && grid.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: view.font.pixelSize*1.3
        }
        Label {
            text: grid.rri.finalWP_ETAAsUTCString
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: (grid.rri.status === RemainingRouteInfo.OnRoute) && grid.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: view.font.pixelSize*1.3
        }

        Label {
            Layout.columnSpan: 4
            Layout.fillWidth: true
            visible: text !== ""
            color: "white"
            wrapMode: Text.Wrap
            horizontalAlignment:  Text.AlignHCenter

            text: {
                switch (grid.rri.status) {
                    case RemainingRouteInfo.PositionUnknown:
                        return qsTr("Position unknown.");
                    case RemainingRouteInfo.OffRoute:
                        return qsTr("More than %1 off route.").arg(global.navigator().aircraft.horizontalDistanceToString(leg.nearThreshold));
                    case RemainingRouteInfo.NearDestination:
                        return qsTr("Near destination.");
                }
                return grid.rri.note
            }

        }

        Item {
            Layout.columnSpan: 4
            Layout.fillWidth: true
            height: 0.2*view.font.pixelSize
        }


    }

} // Rectangle
