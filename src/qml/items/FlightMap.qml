/***************************************************************************
 *   Copyright (C) 2019-2024 by Stefan Kebekus                             *
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
import QtQml
import QtQuick
import QtQuick.Controls

import QtLocation.MapLibre 3.0
import akaflieg_freiburg.enroute

Map {
    id: flightMap

    /*! \brief Pixel per 10 kilometer

    This read-only propery is set to the number of screen pixel per ten
    kilometers on the map. It is updated whenever the zoom value changes. If the
    value cannot be determined for whatever reason, the property is set to zero.

    @warning The value is only a rough approximation and can be wrong at times.
    */
    property real pixelPer10km: 0.0

    /*
    * Handle changes in zoom level
    */

    onZoomLevelChanged: {
        var vec1 = flightMap.fromCoordinate(flightMap.center, false)
        var vec2 = flightMap.fromCoordinate(flightMap.center.atDistanceAndAzimuth(10000.0, 0.0), false)
        var dx = vec2.x - vec1.x
        var dy = vec2.y - vec1.y
        pixelPer10km = Math.sqrt(dx*dx+dy*dy);
    }

    onMapReadyChanged: {
        flightMap.onZoomLevelChanged(zoomLevel)
    }

    maximumZoomLevel: 17
    minimumZoomLevel: 7.0001  // When setting 7 precisely, MapBox is looking for tiles of zoom 6, which we do not have…


    MapLibre.style: Style {
        id: style

        SourceParameter {
            id: approachChart

            styleId: "vac"
            type: "image"

            property string url: {
                var vac = Global.currentVAC
                if (!vac.isValid)
                    return "qrc:/icons/appIcon.png"
                return "file://" + vac.fileName
            }

            // NOTE: At of 15Feb24, the FlightMap does not react to changes of this property.
            // As a temporary workaround, MapPage.qml will reload the map in full
            // whenever the approach chart changes.
            property var coordinates: {
                var vac = Global.currentVAC
                if (vac.isValid)
                    return [[vac.topLeft.longitude, vac.topLeft.latitude],
                            [vac.topRight.longitude, vac.topRight.latitude],
                            [vac.bottomRight.longitude, vac.bottomRight.latitude],
                            [vac.bottomLeft.longitude, vac.bottomLeft.latitude]]

                // Default bounding box, at a place where no-one will see it,
                return [ [1, 77.001], [1.001, 77.001], [1.001, 77], [1, 77] ]

                // Default bounding box for debugging purposes, south-west of Freiburg.
                // return [ [7, 48], [8, 48], [8, 47], [7, 47] ]
            }

        }

        LayerParameter {
            id: approachChartLayer

            styleId: "vacLayer"
            type: "raster"
            property string source: "vac"

            layout: {
                "visibility": Global.currentVAC.isValid ? 'visible' : 'none'
            }
        }

        SourceParameter {
            id: waypointLib

            styleId: "waypointlib"
            type: "geojson"
            property string data: WaypointLibrary.GeoJSON
        }

        LayerParameter {
            id: waypointLibParam

            styleId: "waypoint-layer"

            type: "symbol"
            property string source: "waypointlib"

            layout: {
                "icon-image": '["get", "CAT"]',
                "text-field": '["get", "NAM"]',
                "text-size": 12,
                "text-anchor": "top",
                "text-offset": [0, 1],
                "text-optional": true,
            }

            paint: {
                "text-color": "black",
                "text-halo-width": 2,
                "text-halo-color": "white"
            }
        }

    }

} // End of FlightMap
