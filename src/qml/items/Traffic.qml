/***************************************************************************
 *   Copyright (C) 2021 by Stefan Kebekus                                  *
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

import QtLocation 5.15
import QtPositioning 5.15
import QtQuick 2.15


MapQuickItem {
    id: traffic1MapItem

    property var trafficInfo: ({})

    anchorPoint.x: image.width/2
    anchorPoint.y: image.height/2

    coordinate: trafficInfo.coordinate
    Behavior on coordinate {
        CoordinateAnimation { duration: 1000 }
        enabled: trafficInfo.animate
    }

    visible: trafficInfo.valid

    sourceItem: Item {
        Image {
            id: image

            rotation: isFinite(trafficInfo.TT) ? trafficInfo.TT-flightMap.bearing : 0

            source: trafficInfo.icon

            sourceSize.width: 40
            sourceSize.height: 40
        }
    }
}
