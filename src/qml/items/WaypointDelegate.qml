/***************************************************************************
 *   Copyright (C) 2023 by Stefan Kebekus                                  *
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
    id: wpDelegate

    // Expect a waypoint as a model
    required property var model

    width: parent ? parent.width : 0
    height: idel.height

    // Background color according to METAR/FAA flight category
    Rectangle {
        anchors.fill: parent
        color: wpDelegate.model.modelData.hasMETAR ? wpDelegate.model.modelData.weatherStation.metar.flightCategoryColor : "transparent"
        opacity: 0.2
    }

    WordWrappingItemDelegate {
        id: idel

        width: parent.width

        icon.source: wpDelegate.model.modelData.icon

        text: {
            // Mention horizontal distance
            Navigator.aircraft.horizontalDistanceUnit

            var result = wpDelegate.model.modelData.twoLineTitle

            var wayTo  = Navigator.aircraft.describeWay(PositionProvider.positionInfo.coordinate(), model.modelData.coordinate)
            if (wayTo !== "")
                result = result + "<br>" + wayTo

            if (wpDelegate.model.modelData.hasMETAR)
                result = result + "<br>" + model.modelData.weatherStation.metar.summary
            return result
        }

        onClicked: {
            PlatformAdaptor.vibrateBrief()
            wpDescriptionLoader.active = false
            wpDescriptionLoader.setSource("../dialogs/WaypointDescription.qml", { waypoint: wpDelegate.model.modelData })
            wpDescriptionLoader.active = true
        }
    }

    Loader {
        id: wpDescriptionLoader

        onLoaded: item.open()
    }
}
