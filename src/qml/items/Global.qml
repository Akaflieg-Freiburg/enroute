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

    // Warning
    property bool warnNOTAMLocation: true
    property bool warnMETARPerformance: true
    property bool showMETARPerformanceExplanation: true

    property LocationPermission locationPermission: LocationPermission {
        id: locationPermission

        accuracy: LocationPermission.Precise
        availability: LocationPermission.WhenInUse

        onStatusChanged: PositionProvider.startUpdates()
        Component.onCompleted: PositionProvider.startUpdates()
    }

    Settings {
        category: "GUIWarnings"
        property alias warnNOTAMLocation: global.warnNOTAMLocation
        property alias warnMETARPerformance: global.warnMETARPerformance
        property alias showMETARPerformanceExplanation: global.showMETARPerformanceExplanation
    }


}
