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

import "../items"

Page {
    id: page

    title: qsTr("Moving Map")

    Loader {
        id: mapLoader

        anchors.fill: parent
    }

    Component.onCompleted: {
        mapLoader.source = "../items/MFM.qml"
    }

    Connections {
        // For unclear reasons, the line "target: GeoMapProvider" freezes the application
        // target: GeoMapProvider
        target: global.geoMapProvider()

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
