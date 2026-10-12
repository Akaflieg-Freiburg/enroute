/***************************************************************************
 *   Copyright (C) 2025 by Stefan Kebekus                                  *
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
import QtQuick.Shapes

import akaflieg_freiburg.enroute

/* Terrain silhouette and vertical airspace cuts of a profile view, as computed
   by SideviewQuickItem or RouteProfileQuickItem. Shared so that the side view
   and the route profile look the same. */

Shape {
    id: profileShape

    property var terrain: []
    property var airspaces: ({})

    // Night-mode-aware colors. Terrain has no equivalent on the moving map; the
    // night hues are picked to blend with the dark base map. The airspace hues
    // themselves live in the Global singleton (Global.airspaceBlue etc.), see
    // FlightMap.
    readonly property color terrainFillColor:   GlobalSettings.nightMode ? "#33251a" : "brown"
    readonly property color terrainStrokeColor: GlobalSettings.nightMode ? "#54402c" : "saddlebrown"

    // Same role as the properties of the same name in FlightMap: the
    // semi-transparent airspace bands and fills wash out to near-invisible over
    // a dark base, so their opacity is raised at night.
    readonly property real airspaceBandOpacity: GlobalSettings.nightMode ? 0.35 : 0.2
    readonly property real airspaceFillOpacity: GlobalSettings.nightMode ? 0.30 : 0.2

    preferredRendererType: Shape.CurveRenderer
    asynchronous: true

    ShapePath {
        id: terrainPath
        strokeWidth: -1
        strokeColor: profileShape.terrainStrokeColor
        fillColor: profileShape.terrainFillColor

        PathPolyline { path: profileShape.terrain } // qmllint disable unresolved-type
    }

    ShapePath {
        id: airspacesABorder
        strokeWidth: 10
        strokeColor: Qt.alpha(Global.airspaceBlue, profileShape.airspaceBandOpacity)
        fillColor: "transparent"

        PathMultiline { paths: profileShape.airspaces["A"] }
    }

    ShapePath {
        id: airspacesA
        strokeWidth: 2
        strokeColor: Global.airspaceBlue
        fillColor: "transparent"

        PathMultiline { paths: profileShape.airspaces["A"] }
    }

    ShapePath {
        id: airspacesRBorder
        strokeWidth: 10
        strokeColor: Qt.alpha(Global.airspaceRed, profileShape.airspaceBandOpacity)
        fillColor: "transparent"
        fillRule: ShapePath.WindingFill

        PathMultiline { paths: profileShape.airspaces["R"] }
    }

    ShapePath {
        id: airspacesR
        strokeWidth: 2
        strokeColor: Global.airspaceRed
        fillColor: "transparent"

        PathMultiline { paths: profileShape.airspaces["R"] }
    }

    ShapePath {
        id: airspacesPJE
        strokeWidth: 2
        strokeColor: Global.airspaceRed
        fillColor: "transparent"
        dashPattern: [4,3]
        strokeStyle: ShapePath.DashLine

        PathMultiline { paths: profileShape.airspaces["PJE"] }
    }

    ShapePath {
        id: airspacesTMZ
        strokeWidth: 2
        strokeColor: Global.airspaceNeutral
        fillColor: "transparent"
        dashPattern: [4.0, 3.0, 0.5, 3.0]
        strokeStyle: ShapePath.DashLine

        PathMultiline { paths: profileShape.airspaces["TMZ"] }
    }

    ShapePath {
        id: airspacesNRABorder
        strokeWidth: 10
        strokeColor: Qt.alpha(Global.airspaceGreen, profileShape.airspaceBandOpacity)
        fillColor: "transparent"
        fillRule: ShapePath.WindingFill

        PathMultiline { paths: profileShape.airspaces["NRA"] }
    }

    ShapePath {
        id: airspacesNRA
        strokeWidth: 2
        strokeColor: Global.airspaceGreen
        fillColor: "transparent"

        PathMultiline { paths: profileShape.airspaces["NRA"] }
    }

    ShapePath {
        id: airspacesRMZ
        strokeWidth: 2
        strokeColor: Global.airspaceBlue
        fillColor: Qt.alpha(Global.airspaceBlue, profileShape.airspaceFillOpacity)
        dashPattern: [4,3]
        strokeStyle: ShapePath.DashLine
        fillRule: ShapePath.WindingFill

        PathMultiline { paths: profileShape.airspaces["RMZ"] }
    }

    ShapePath {
        id: airspacesCTR
        strokeWidth: 2
        strokeColor: Global.airspaceBlue
        fillColor: Qt.alpha(Global.airspaceRed, profileShape.airspaceFillOpacity)
        dashPattern: [4,3]
        strokeStyle: ShapePath.DashLine
        fillRule: ShapePath.WindingFill

        PathMultiline { paths: profileShape.airspaces["CTR"] }
    }
}
