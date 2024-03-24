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

import QtPositioning
import QtQuick
import QtQuick.Controls

import akaflieg_freiburg.enroute
import "../items"

Page {
    id: page

    title: qsTr("Moving Map")

    header: PageHeader {

        height: 60 + SafeInsets.top
        leftPadding: SafeInsets.left
        rightPadding: SafeInsets.right
        topPadding: SafeInsets.top
        visible: GeoMapProvider.approachChart.isValid

        ToolButton {
            id: backButton

            anchors.left: parent.left
            anchors.verticalCenter: parent.verticalCenter

            icon.source: "/icons/material/ic_arrow_back.svg"

            onClicked: {
                PlatformAdaptor.vibrateBrief()
                GeoMapProvider.setApproachChart("")
            }
        }

        Label {
            id: lbl

            anchors.verticalCenter: parent.verticalCenter

            anchors.left: parent.left
            anchors.leftMargin: 72
            anchors.right: parent.right

            text: {
                var result = qsTr("Approach Chart")
                if (GeoMapProvider.approachChartBaseName !== "")
                    result = result + ": " + GeoMapProvider.approachChartBaseName
                return result
            }
            elide: Label.ElideRight
            font.pixelSize: 20
            verticalAlignment: Qt.AlignVCenter
        }
    }


    Loader {
        id: mapLoader

        anchors.fill: parent
    }

    Component.onCompleted: {
        mapLoader.source = "../items/MFM.qml"
    }

    Connections {
        target: GeoMapProvider

        // NOTE: At of 15Feb24, the FlightMap does not react to changes of the property approachChart.coordinates
        // As a temporary workaround, we reload the map in full
        // whenever the approach chart changes.
        function onApproachChartChanged() {
            mapLoader.active = false
            mapLoader.source = "../items/MFM.qml"
            mapLoader.active = true
        }
        function onStyleFileURLChanged() {
            mapLoader.active = false
            mapLoader.source = "../items/MFM.qml"
            mapLoader.active = true
        }
        function onGeoJSONChanged() {
            mapLoader.active = false
            mapLoader.source = "../items/MFM.qml"
            mapLoader.active = true
        }
    }
}
