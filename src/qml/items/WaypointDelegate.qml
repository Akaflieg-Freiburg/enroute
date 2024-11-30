/***************************************************************************
 *   Copyright (C) 2023-2024 by Stefan Kebekus                             *
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

import akaflieg_freiburg.enroute



Item {
    id: waypointDelegate

    required property waypoint waypoint

    property var weatherStation: WeatherDataProvider.findWeatherStation(waypoint.ICAOCode)
    Connections {
        target: WeatherDataProvider

        function onWeatherStationsChanged() {
            waypointDelegate.weatherStation = WeatherDataProvider.findWeatherStation(waypointDelegate.waypoint.ICAOCode)
        }
    }

    implicitWidth: parent ? parent.width : 0
    implicitHeight: idel.implicitHeight

    // Background color according to METAR/FAA flight category
    Rectangle {
        anchors.fill: parent
        color: {
            if (waypointDelegate.weatherStation === null)
                return "transparent"
            if (waypointDelegate.weatherStation.metar.isValid)
                return waypointDelegate.weatherStation.metar.flightCategoryColor
            return "transparent"
        }
        opacity: 0.2
    }

    WordWrappingItemDelegate {
        id: idel

        width: parent.width

        icon.source: waypointDelegate.waypoint.icon

        text: {
            // Mention horizontal distance
            Navigator.aircraft.horizontalDistanceUnit

            var result = waypointDelegate.waypoint.twoLineTitle

            var wayTo  = Navigator.aircraft.describeWay(PositionProvider.positionInfo.coordinate(), waypoint.coordinate)
            if (wayTo !== "")
                result = result + "<br>" + wayTo

            if ((waypointDelegate.weatherStation !== null) && waypointDelegate.weatherStation.metar.isValid)
                result = result + "<br>" + waypointDelegate.weatherStation.metar.summary(Navigator.aircraft, Clock.time)
            return result
        }

        onClicked: {
            PlatformAdaptor.vibrateBrief()
            wpDescriptionLoader.active = false
            wpDescriptionLoader.setSource("../dialogs/WaypointDescription.qml", { waypoint: waypointDelegate.waypoint })
            wpDescriptionLoader.active = true
        }
    }

    Loader {
        id: wpDescriptionLoader
        onLoaded: item.open()
    }
}
