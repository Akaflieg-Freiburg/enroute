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
import QtPositioning
import QtQuick

import akaflieg_freiburg.enroute


MapQuickItem {
    id: traffic1MapItem

    property double bearing
    property double pixelPer10km
    property var trafficInfo

    coordinate: trafficInfo.extrapolatedCoordinate
    visible: trafficInfo.relevant

    // Smoothly glide between the extrapolated positions that
    // TrafficFactor_WithPosition publishes once per second, so that motion looks
    // continuous even though the C++ side only updates at 1 Hz. Gated by
    // "animate" so that newly-appearing or teleporting traffic snaps into place
    // instead of sliding across the map.
    Behavior on coordinate {
        CoordinateAnimation {
            duration: 1000
        }
        enabled: traffic1MapItem.trafficInfo.animate
    }

    sourceItem: Item {
        // Does this traffic have a meaningful heading to point at?
        readonly property bool hasHeading:
               traffic1MapItem.trafficInfo.type !== TrafficFactor_Abstract.Balloon
            && traffic1MapItem.trafficInfo.type !== TrafficFactor_Abstract.StaticObstacle
            && traffic1MapItem.trafficInfo.positionInfo.trueTrack().isFinite()

        // The traffic's own heading, in degrees. ONLY this is animated, so that
        // heading changes are smoothed but map rotation is not.
        property double trueTrackDEG: hasHeading
            ? traffic1MapItem.trafficInfo.positionInfo.trueTrack().toDEG()
            : 0
        Behavior on trueTrackDEG {
            RotationAnimation {
                direction: RotationAnimation.Shortest
                duration: 1000
            }
            enabled: traffic1MapItem.trafficInfo.animate
        }

        // Screen rotation. The map bearing is applied here, live and un-animated,
        // so that rotating the map tracks the traffic immediately; only the
        // traffic's own heading (trueTrackDEG) is animated.
        rotation: hasHeading ? trueTrackDEG - traffic1MapItem.bearing : 0

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
