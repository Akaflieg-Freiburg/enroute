/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import enroute 1.0

import QtQml 2.15


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

    sourceItem: Item {
        Image {
            id: image

            rotation: trafficInfo.TT-flightMap.bearing

            source: trafficInfo.icon
            sourceSize.width: 40
            sourceSize.height: 40
        }

        Label {
            anchors.left: image.right
            anchors.bottom: image.top
            text: trafficInfo.vDistText

            leftInset: -2
            rightInset: -2
            bottomInset: -1
            topInset: -2

            background: Rectangle {
                color: "white"
            }
        }
    }
}
