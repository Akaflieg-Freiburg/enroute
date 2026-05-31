/***************************************************************************
 *   Copyright (C) 2021-2026 by Stefan Kebekus                             *
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
import QtQuick

import akaflieg_freiburg.enroute


MapQuickItem {
    id: traffic1MapItem

    property double bearing
    property double pixelPer10km
    property var trafficInfo

    coordinate: trafficInfo.extrapolatedCoordinate
    visible: trafficInfo.relevant

    sourceItem: Item {
        rotation: {
            if (traffic1MapItem.trafficInfo.type === TrafficFactor_Abstract.Balloon)
                return 0
            if (traffic1MapItem.trafficInfo.type === TrafficFactor_Abstract.StaticObstacle)
                return 0
            if (!traffic1MapItem.trafficInfo.positionInfo.trueTrack().isFinite())
                return 0
            return traffic1MapItem.trafficInfo.positionInfo.trueTrack().toDEG() - traffic1MapItem.bearing
        }
        Behavior on rotation {
            RotationAnimation {
                direction: RotationAnimation.Shortest
                duration: 1000
            }
            enabled: traffic1MapItem.trafficInfo.animate
        }

        Rectangle {
            opacity: 0.2
            width: traffic1MapItem.trafficInfo.uncertaintyRadius.toM() * traffic1MapItem.pixelPer10km / 5000
            height: width
            x: -width/2
            y: -height/2
            radius: width/2
            color: traffic1MapItem.trafficInfo.color
        }

        FlightVector {
            width: 3
            opacity: traffic1MapItem.trafficInfo.uncertaintyRadius.toM() > 0 ? 0.7 : 0.8
            pixelPerTenKM: traffic1MapItem.pixelPer10km
            groundSpeedInMetersPerSecond: traffic1MapItem.trafficInfo.positionInfo.groundSpeed().toMPS()
            visible: (groundSpeedInMetersPerSecond > 5) && (traffic1MapItem.trafficInfo.positionInfo.trueTrack().isFinite())
        }

        Image {
            id: image

            width: 40
            height: width
            fillMode: Image.PreserveAspectFit
            x: -width/2.0
            y: -height/2.0

            opacity: traffic1MapItem.trafficInfo.uncertaintyRadius.toM() > 0 ? 0.7 : 1.0
            source: traffic1MapItem.trafficInfo.icon
        }
    }
}
