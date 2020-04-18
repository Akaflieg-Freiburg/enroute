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

import QtLocation 5.14
import QtQuick 2.14
import QtQuick.Controls 2.14

import enroute 1.0

Map {
    id: flightMap

    /*! \brief Pixel per 10 kilometer

    This read-only propery is set to the number of screen pixel per ten
    kilometers on the map. It is updated whenever the zoom value changes. If the
    value cannot be determined for whatever reason, the property is set to zero.
    
    @warning The value is only a rough approximation and can be wrong at times.
    */
    property real pixelPer10km: 0.0

    /*! \brief Name of a GeoJSON file containing airspace and waypoint information */
    property string geoJSON
    
    /*! \brief Width of thick lines around airspaces, such as class D */
    property real airspaceLineWidth: 5.0

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
        onZoomLevelChanged(zoomLevel)
        MobileAdaptor.hideSplashScreen()
    }

    maximumZoomLevel: 13
    minimumZoomLevel: 7
    
    
    /*************************************
     * Aviation Data
     *************************************/
    
    MapParameter {
        type: "source"
        
        property var name: "aviationData"
        property var sourceType: "geojson"
        property var data: flightMap.geoJSON
    }

    /*************************************
     * Airspaces
     *************************************/
    
    MapParameter {
        type: "image"
        
        property var name: "WhiteBox"
        property var sprite: ":flightMap/sprites/WhiteBox.png"
    }
    
    
    /*
     * RMZ - Radio Mandatory Zone
     */
    
    MapParameter {
        type: "layer"
        
        property var name: "RMZ"
        property var layerType: "fill"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "RMZ"]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "RMZ"
        property var fillColor: "blue"
        property var fillOpacity: 0.2
    }
    
    MapParameter {
        type: "layer"
        
        property var name: "RMZoutline"
        property var layerType: "line"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "RMZ"]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "RMZoutline"
        property var lineColor: "blue"
        property var lineWidth: 2.0
        property var lineDasharray: [3.0, 3.0]
    }
    
    MapParameter {
        type: "layer"
        
        property var name: "RMZLabels"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "RMZ"]
        property var minzoom: 10
    }
    
    MapParameter {
        type: "layout"
        
        property var layer: "RMZLabels"
        property var textField: ["get", "NAM"]
        property var textSize: 12
        property var iconImage: "WhiteBox"
        property var iconTextFit: "both"
        property var iconTextFitPadding: [2,5,2,5]
    }
    
    
    /*
     * TMZ - Transponder Mandatory Zone
     */
    
    MapParameter {
        type: "layer"
        
        property var name: "TMZ"
        property var layerType: "line"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "TMZ"]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "TMZ"
        property var lineColor: "black"
        property var lineWidth: 2.0
        property var lineDasharray: [4.0, 3.0, 0.5, 3.0]
    }
    
    
    /*
     * Danger Zone - PJE
     */
    
    MapParameter {
        type: "layer"
        
        property var name: "PJE"
        property var layerType: "line"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "PJE"]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "PJE"
        property var lineColor: "red"
        property var lineWidth: 2.0
        property var lineDasharray: [4.0, 3.0]
    }
    
    
    /*
     * Airspaces A, B, C, D that are not control zones
     */
    
    MapParameter {
        type: "layer"
        
        property var name: "ABCDOutlines"
        property var layerType: "line"
        property var source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "A"], ["==", ["get", "CAT"], "B"], ["==", ["get", "CAT"], "C"], ["==", ["get", "CAT"], "D"]]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "ABCDOutlines"
        property var lineColor: "blue"
        property var lineWidth: 2.0
    }
    
    MapParameter {
        type: "layer"
        
        property var name: "ABCDs"
        property var layerType: "line"
        property var source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "A"], ["==", ["get", "CAT"], "B"], ["==", ["get", "CAT"], "C"], ["==", ["get", "CAT"], "D"]]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "ABCDs"
        property var lineColor: "blue"
        property var lineOpacity: 0.2
        property var lineWidth: airspaceLineWidth
        property var lineOffset: airspaceLineWidth/2.0
    }
    
    
    /*
     * Control Zones
     */
    
    MapParameter {
        type: "layer"
        
        property var name: "controlZones"
        property var layerType: "fill"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "CTR"]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "controlZones"
        property var fillColor: "red"
        property var fillOpacity: 0.2
    }
    
    MapParameter {
        type: "layer"
        
        property var name: "controlZoneOutlines"
        property var layerType: "line"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "CTR"]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "controlZoneOutlines"
        property var lineColor: "blue"
        property var lineWidth: 2.0
        property var lineDasharray: [4.0, 3.0]
    }
    
    MapParameter {
        type: "layer"
        
        property var name: "controlZoneLabels"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "CTR"]
        property var minzoom: 10
    }
    
    MapParameter {
        type: "layout"

        property var layer: "controlZoneLabels"
        property var textField: ["get", "NAM"]
        property var textSize: 12
        property var iconImage: "WhiteBox"
        property var iconTextFit: "both"
        property var iconTextFitPadding: [2,5,2,5]
    }
    
    
    /*
     * Danger Zone, Prohibited Zone, Restricted Zone
     */
    
    MapParameter {
        type: "layer"
        
        property var name: "dangerZones"
        property var layerType: "line"
        property var source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "DNG"], ["==", ["get", "CAT"], "R"], ["==", ["get", "CAT"], "P"]]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "dangerZones"
        property var lineColor: "red"
        property var lineOpacity: 0.2
        property var lineWidth: airspaceLineWidth
        property var lineOffset: airspaceLineWidth/2.0
    }
    
    MapParameter {
        type: "layer"
        
        property var name: "dangerZoneOutlines"
        property var layerType: "line"
        property var source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "DNG"], ["==", ["get", "CAT"], "R"], ["==", ["get", "CAT"], "P"]]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "dangerZoneOutlines"
        property var lineColor: "red"
        property var lineWidth: 2.0
        property var lineDasharray: [4.0, 3.0]
    }
    
    MapParameter {
        type: "layer"
        
        property var name: "dangerZoneLabels"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "R"], ["==", ["get", "CAT"], "P"]]
        property var minzoom: 10
    }
    
    MapParameter {
        type: "paint"
        property var layer: "dangerZoneLabels"
        property var textHaloWidth: 2
        property var textHaloColor: "white"
    }
    
    MapParameter {
        type: "layout"
        
        property var layer: "dangerZoneLabels"
        property var textField: ["get", "NAM"]
        property var textSize: 12
        property var iconImage: "WhiteBox"
        property var iconTextFit: "both"
        property var iconTextFitPadding: [2,5,2,5]
    }
    
    // End of airspaces

    /*
     * TFC - Traffic circuit
     */
    
    MapParameter {
        type: "layer"
        
        property var name: "TFC"
        property var layerType: "line"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "PRC"]
        property var minzoom: 10
    }
    MapParameter {
        type: "paint"
        property var layer: "TFC"
        property var lineColor: ["get", "GAC"]
        property var lineWidth: 3.0
    }
    
    MapParameter {
        type: "layer"
        
        property var name: "TFCLabels"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "PRC"]
        property var minzoom: 10
    }
    MapParameter {
        type: "layout"
        
        property var layer: "TFCLabels"
        property var symbolPlacement: "line"
        property var textField: ["get", "NAM"]
        property var textSize: 12
    }
    MapParameter {
        type: "paint"
        property var layer: "TFCLabels"
        property var textHaloWidth: 2
        property var textHaloColor: "white"
    }

    /*************************************
     * Waypoints
     *************************************/

    // Define the necessary images
    
    MapParameter {
        type: "image"
        
        property var name: "AD"
        property var sprite: ":flightMap/sprites/AD"
    }
    MapParameter {
        type: "image"
        
        property var name: "AD-GLD"
        property var sprite: ":flightMap/sprites/AD-GLD"
    }
    MapParameter {
        type: "image"
        
        property var name: "AD-GRASS"
        property var sprite: ":flightMap/sprites/AD-GRASS"
    }
    MapParameter {
        type: "image"
        
        property var name: "AD-INOP"
        property var sprite: ":flightMap/sprites/AD-INOP"
    }
    MapParameter {
        type: "image"
        
        property var name: "AD-MIL"
        property var sprite: ":flightMap/sprites/AD-MIL"
    }
    MapParameter {
        type: "image"
        
        property var name: "AD-MIL-GRASS"
        property var sprite: ":flightMap/sprites/AD-MIL-GRASS"
    }
    MapParameter {
        type: "image"
        
        property var name: "AD-MIL-PAVED"
        property var sprite: ":flightMap/sprites/AD-MIL-PAVED"
    }
    MapParameter {
        type: "image"
        
        property var name: "AD-PAVED"
        property var sprite: ":flightMap/sprites/AD-PAVED"
    }
    MapParameter {
        type: "image"
        
        property var name: "AD-UL"
        property var sprite: ":flightMap/sprites/AD-UL"
    }
    MapParameter {
        type: "image"
        
        property var name: "AD-WATER"
        property var sprite: ":flightMap/sprites/AD-WATER"
    }
    MapParameter {
        type: "image"
        
        property var name: "MRP"
        property var sprite: ":flightMap/sprites/MRP"
    }
    MapParameter {
        type: "image"
        
        property var name: "NDB"
        property var sprite: ":flightMap/sprites/NDB"
    }
    MapParameter {
        type: "image"
        
        property var name: "RP"
        property var sprite: ":flightMap/sprites/RP"
    }
    MapParameter {
        type: "image"
        
        property var name: "VOR"
        property var sprite: ":flightMap/sprites/VOR"
    }
    MapParameter {
        type: "image"
        
        property var name: "VOR-DME"
        property var sprite: ":flightMap/sprites/VORDME"
    }
    MapParameter {
        type: "image"
        
        property var name: "VORTAC"
        property var sprite: ":flightMap/sprites/VORTAC"
    }
    MapParameter {
        type: "image"
        
        property var name: "DVOR"
        property var sprite: ":flightMap/sprites/VOR"
    }
    MapParameter {
        type: "image"
        
        property var name: "DVOR-DME"
        property var sprite: ":flightMap/sprites/VORDME"
    }
    MapParameter {
        type: "image"
        
        property var name: "DVORTAC"
        property var sprite: ":flightMap/sprites/VORTAC"
    }
    
    
    // NavAids - Text. Will be drawn with least priority
    
    MapParameter {
        type: "layer"
        
        property var name: "optionalText"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var filter: ["==", ["get", "TYP"], "NAV"]
    }
    
    MapParameter {
        type: "layout"

        property var layer: "optionalText"
        property var textField: ["get", "COD"]
        property var textSize: 12
        property var textAnchor: "top"
        property var textOffset: [0, 1]
        property var textOptional: true
    }
    
    MapParameter {
        type: "paint"
        property var layer: "optionalText"
        property var textHaloWidth: 2
        property var textHaloColor: "white"
    }

    
    // Gliding and UL Airfields
    
    MapParameter {
        type: "layer"
        
        property var name: "WPs"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "AD-GLD"], ["==", ["get", "CAT"], "AD-INOP"], ["==", ["get", "CAT"], "AD-UL"], ["==", ["get", "CAT"], "AD-WATER"]]
    }

    
    MapParameter {
        type: "layout"

        property var layer: "WPs"
        property var textField: ["get", "NAM"]
        property var textSize: 12
        property var textAnchor: "top"
        property var textOffset: [0, 1]
        property var textOptional: true
        property var iconImage: ["get", "CAT"]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "WPs"
        property var textHaloWidth: 2
        property var textHaloColor: "white"
    }


    // Reporting points
    
    MapParameter {
        type: "layer"
        
        property var name: "RPs"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var minzoom: 8
        property var filter: ["any", ["==", ["get", "CAT"], "RP"], ["==", ["get", "CAT"], "MRP"]]
    }
    
    MapParameter {
        type: "layout"

        property var layer: "RPs"
        property var textField: ["get", "SCO"]
        property var textSize: 12
        property var textAnchor: "top"
        property var textOffset: [0, 1]
        property var textOptional: true
        property var iconImage: ["get", "CAT"]
    }
    
    MapParameter {
        type: "paint"
        property var layer: "RPs"
        property var textHaloWidth: 2
        property var textHaloColor: "white"
    }

    
    // GA Airfields with grass runway
    
    MapParameter {
        type: "layer"
        
        property var name: "AD-GRASS"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "AD-GRASS"], ["==", ["get", "CAT"], "AD-MIL-GRASS"]]
    }
    
    MapParameter {
        type: "layout"

        property var layer: "AD-GRASS"
        property var textField: ["get", "NAM"]
        property var textSize: 12
        property var textAnchor: "top"
        property var textOffset: [0, 1]
        property var textOptional: true
        property var iconImage: ["get", "CAT"]
        property var iconRotate: ["get", "ORI"]
        property var iconRotationAlignment: "map"
    }
    
    MapParameter {
        type: "paint"
        property var layer: "AD-GRASS"
        property var textHaloWidth: 2
        property var textHaloColor: "white"
    }

    
    // NavAids - Icons. Will always be drawn, but might be overdrawn by other stuff.
    
    MapParameter {
        type: "layer"
        
        property var name: "NavAidIcons"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var filter: ["==", ["get", "TYP"], "NAV"]
    }
    
    MapParameter {
        type: "layout"

        property var layer: "NavAidIcons"
        property var iconImage: ["get", "CAT"]
        property var iconIgnorePlacement: true
        property var iconAllowOverlap: true
    }

    
    // GA Airfields with paved or unknown runway
    
    MapParameter {
        type: "layer"
        
        property var name: "AD-PAVED"
        property var layerType: "symbol"
        property var source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "AD"], ["==", ["get", "CAT"], "AD-PAVED"], ["==", ["get", "CAT"], "AD-MIL"], ["==", ["get", "CAT"], "AD-MIL-PAVED"]]
    }
    
    MapParameter {
        type: "layout"

        property var layer: "AD-PAVED"
        property var textField: ["get", "NAM"]
        property var textSize: 12
        property var textAnchor: "top"
        property var textOffset: [0, 1]
        property var textOptional: true
        property var iconImage: ["get", "CAT"]
        property var iconRotate: ["get", "ORI"]
        property var iconRotationAlignment: "map"
    }
    
    MapParameter {
        type: "paint"
        property var layer: "AD-PAVED"
        property var textHaloWidth: 2
        property var textHaloColor: "white"
    }
    

} // End of FlightMap
