/***************************************************************************
 *   Copyright (C) 2021-2023 by Stefan Kebekus                             *
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

import QtLocation
import QtPositioning
import QtQuick

import akaflieg_freiburg.enroute


MapQuickItem {
    id: traffic1MapItem

    property Map map: ({})
    property var trafficInfo: ({})

    coordinate: trafficInfo.positionInfo.coordinate()
    Behavior on coordinate {
        CoordinateAnimation { duration: 1000 }
        enabled: trafficInfo.animate
    }

    visible: trafficInfo.valid

    Connections {
        // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
        // is not updated when the height of the map changes. It does get updated when the
        // width of the map changes. We use the undocumented method polishAndUpdate() here.
        target: map
        function onHeightChanged() { traffic1MapItem.polishAndUpdate() }
    }

    sourceItem: Item {
        rotation: trafficInfo.positionInfo.trueTrack().isFinite() ? trafficInfo.positionInfo.trueTrack().toDEG()-map.bearing : 0

        FlightVector {
            width: 3
            pixelPerTenKM: map.pixelPer10km
            groundSpeedInMetersPerSecond: trafficInfo.positionInfo.groundSpeed().toMPS()
            visible: (groundSpeedInMetersPerSecond > 5) && (trafficInfo.positionInfo.trueTrack().isFinite())
            opacity: 0.8
        }

        Image {
            id: image

            x: -width/2.0
            y: -height/2.0

            source: trafficInfo.icon

            sourceSize.width: 30
            sourceSize.height: 30
        }
    }
}
