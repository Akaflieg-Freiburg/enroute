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

import enroute 1.0

Rectangle {   
    color: "#AA000000"
    height: grid.implicitHeight // grid.rri.isValid ? grid.implicitHeight : 0

    GridLayout {
        id: grid

        anchors.fill: parent
        anchors.leftMargin: Qt.application.font.pixelSize
        rowSpacing: 0
        columns: 4

        property var rri: global.navigator().remainingRouteInfo

        Label {
            Layout.columnSpan: 4
            Layout.alignment: Qt.AlignHCenter
            visible: text !== ""
            color: "white"

            text: {
                switch (grid.rri.status) {
                    case RemainingRouteInfo.PositionUnknown:
                        return qsTr("Position unknown.");
                    case RemainingRouteInfo.OffRoute:
                        return qsTr("More than %1 off route.").arg(global.navigator().aircraft.horizontalDistanceToString(leg.nearThreshold));
                    case RemainingRouteInfo.NearDestination:
                        return qsTr("Near destination.");
                }
                return ""
            }

        }

        Item { width: 1 }
        Item { Layout.fillWidth: true }
        Item { Layout.fillWidth: true }
        Item { Layout.fillWidth: true }

        Label {
            Layout.alignment: Qt.AlignHCenter
            Layout.fillWidth: true
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: Qt.application.font.pixelSize*0.9
        }
        Label {
            text: "DIST"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: Qt.application.font.pixelSize*0.9
        }
        Label {
            text: "ETE"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: Qt.application.font.pixelSize*0.9
        }
        Label {
            text: "ETA"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.pixelSize: Qt.application.font.pixelSize*0.9
        }

        Label {
            text: grid.rri.nextWP.shortName
            elide: Text.ElideRight
            color: "white"
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
        }
        Label {
            text: global.navigator().aircraft.horizontalDistanceToString(grid.rri.nextWP_DIST)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
        }
        Label {
            text: "%1 h".arg(grid.rri.nextWP_ETE.toHoursAndMinutes())
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
        }
        Label {
            text: grid.rri.nextWP_ETE.isFinite() ? Qt.formatDateTime(grid.rri.nextWP_ETA, "h:mm") : "-:--"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: grid.rri.status === RemainingRouteInfo.OnRoute
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
        }


        Label {
            text: grid.rri.finalWP.shortName
            elide: Text.ElideRight
            color: "white"
            Layout.fillWidth: true
            Layout.maximumWidth: implicitWidth
            visible: (grid.rri.status === RemainingRouteInfo.OnRoute) && grid.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
        }
        Label {
            text: global.navigator().aircraft.horizontalDistanceToString(grid.rri.finalWP_DIST)
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: (grid.rri.status === RemainingRouteInfo.OnRoute) && grid.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
        }
        Label {
            text: "%1 h".arg(grid.rri.finalWP_ETE.toHoursAndMinutes())
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: (grid.rri.status === RemainingRouteInfo.OnRoute) && grid.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
        }
        Label {
            text: grid.rri.finalWP_ETE.isFinite() ? Qt.formatDateTime(grid.rri.finalWP_ETA, "h:mm") : "-:--"
            color: "white"
            Layout.alignment: Qt.AlignHCenter
            Layout.minimumWidth: implicitWidth
            visible: (grid.rri.status === RemainingRouteInfo.OnRoute) && grid.rri.finalWP.isValid
            font.weight: Font.Bold
            font.pixelSize: Qt.application.font.pixelSize*1.3
        }

    }

} // Rectangle
