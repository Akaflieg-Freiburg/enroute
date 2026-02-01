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

pragma Singleton

import QtCore
import QtQuick
import QtQuick.Controls

import akaflieg_freiburg.enroute

Item {
    id: global

    property Loader dialogLoader
    property Drawer drawer
    property var toast
    property vac currentVAC
    property vac defaultVAC

    //
    // GUI Warnings
    //
    property bool warnNOTAMLocation: true
    property bool warnMETARPerformance: true
    property bool showMETARPerformanceExplanation: true
    Settings {
        category: "GUIWarnings"
        property alias warnNOTAMLocation: global.warnNOTAMLocation
        property alias warnMETARPerformance: global.warnMETARPerformance
        property alias showMETARPerformanceExplanation: global.showMETARPerformanceExplanation
    }

    //
    // Parameters for the moving map display
    //

    property int mapBearingPolicy: 0
    readonly property int mapBearingPolicyRect: {
        // Rectified version, always returns sane values
        if (!mapBearingPolicy || (mapBearingPolicy < 0) || (mapBearingPolicy > 2)) {
            return MFM.NUp
        }
        return mapBearingPolicy
    }
    onMapBearingPolicyChanged: {
        if (mapBearingPolicy != MFM.UserDefinedBearingUp)
        {
            mapBearingRevertPolicy = mapBearingPolicy
        }

        if (toast) {
            if (global.mapBearingPolicy === MFM.NUp) {
                toast.doToast(qsTr("Map Mode: North Up"))
            } else if (Global.mapBearingPolicy === MFM.TTUp) {
                toast.doToast(qsTr("Map Mode: Track Up"))
            } else {
                toast.doToast(qsTr("Map Mode: User Defined Direction Up"))
            }
        }
    }
    property int mapBearingRevertPolicy: 0
    readonly property int mapBearingRevertPolicyRect: {
        // Rectified version, always returns sane values
        if (!mapBearingRevertPolicy || (mapBearingRevertPolicy < 0) || (mapBearingRevertPolicy > 2)) {
            return MFM.NUp
        }
        return mapBearingRevertPolicy
    }
    property bool followGPS: true
    onFollowGPSChanged: {
        if (toast) {
            if (followGPS) {
                toast.doToast(qsTr("Map Mode: Autopan"))
            } else {
                toast.doToast(qsTr("Map Mode: No Autopan"))
            }
        }
    }
    property real mapZoomLevelMax: 17
    property real mapZoomLevelMin: 7.0001 // When setting 7 precisely, MapBox is looking for tiles of zoom 6, which we do not have…
    property real mapZoomLevel: 12
    readonly property real mapZoomLevelRect: {
        // Rectified version, always returns sane values
        if (!mapZoomLevel || !isFinite(mapZoomLevel) || (mapZoomLevel < mapZoomLevelMin) || (mapZoomLevel > mapZoomLevelMax)) {
            return 12
        }
        return mapZoomLevel
    }
    property real mapBearing: 0
    readonly property real mapBearingRect: {
        // Rectified version, always returns sane values
        if (!mapBearing || !isFinite(mapBearing) || (mapBearing < 0) || (mapBearing > 360)) {
            return 0
        }
        return mapBearing
    }
    property var mapCenter
    readonly property var mapCenterRect: {
        // Rectified version, always returns sane values
        if (!mapCenter || !mapCenter.isValid) {
            return PositionProvider.lastValidCoordinate
        }
        return mapCenter
    }
    Settings {
        category: "MovingMap"
        property alias mapBearingPolicy: global.mapBearingPolicy
        property alias mapBearingRevertPolicy: global.mapBearingRevertPolicy
        property alias mapFollowGPSPolicy: global.followGPS
        property alias mapZoomLevel: global.mapZoomLevel
        property alias mapBearing: global.mapBearing
        property alias mapCenter: global.mapCenter
    }

    //
    // Permissions
    //

    property LocationPermission locationPermission: LocationPermission {
        id: locationPermission

        accuracy: LocationPermission.Precise
        availability: LocationPermission.WhenInUse

        onStatusChanged: PositionProvider.startUpdates()
        Component.onCompleted: PositionProvider.startUpdates()
    }
}
