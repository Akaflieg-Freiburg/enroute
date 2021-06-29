/***************************************************************************
 *   Copyright (C) 2019-2021 by Stefan Kebekus                             *
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
import QtQuick 2.15
import QtQuick.Controls 2.15

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
    property real airspaceLineWidth: 7.0

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
        global.mobileAdaptor().hideSplashScreen()
    }

    maximumZoomLevel: 13.5
    minimumZoomLevel: 7
    
    
    /*************************************
     * Aviation Data
     *************************************/
    
    MapParameter {
        type: "source"
        
        property string name: "aviationData"
        property string sourceType: "geojson"
        property string data: flightMap.geoJSON
    }

    /*************************************
     * Airspaces
     *************************************/
    
    MapParameter {
        type: "image"
        
        property string name: "WhiteBox"
        property string sprite: ":flightMap/sprites/WhiteBox.png"
    }
    

    /*
     * FIS - Flight Information Sector
     */

    MapParameter {
        type: "layer"

        property string name: "FIS"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "FIS"]
        property int maxzoom: 10
    }

    MapParameter {
        type: "paint"
        property string layer: "FIS"
        property string lineColor: "black"
        property real lineWidth: 0.5
    }

    /*
     * Gliding Sectors
     */

    MapParameter {
        type: "layer"

        property string name: "glidingSector"
        property string layerType: "fill"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "GLD"]
    }
    MapParameter {
        type: "paint"
        property string layer: "glidingSector"
        property string fillColor: "yellow"
        property real fillOpacity: 0.1
    }

    MapParameter {
        type: "layer"

        property string name: "glidingSectorOutlines"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "GLD"]
    }
    MapParameter {
        type: "paint"
        property string layer: "glidingSectorOutlines"
        property string lineColor: "yellow"
        property real lineOpacity: 0.8 //0.3
        property real lineWidth: 2.0
    }

    MapParameter {
        type: "layer"

        property string name: "glidingSectorLabels"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "GLD"]
        property int minzoom: 10
    }
    MapParameter {
        type: "layout"

        property string layer: "glidingSectorLabels"
        property var textField: ["get", "NAM"]
        property real textSize: 12
        property string iconImage: "WhiteBox"
        property string iconTextFit: "both"
        property var iconTextFitPadding: [2,5,2,5]
    }


    /*
     * RMZ - Radio Mandatory Zone
     */
    
    MapParameter {
        type: "layer"
        
        property string name: "RMZ"
        property string layerType: "fill"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "RMZ"]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "RMZ"
        property string fillColor: "blue"
        property real fillOpacity: 0.2
    }
    
    MapParameter {
        type: "layer"
        
        property string name: "RMZoutline"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "RMZ"]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "RMZoutline"
        property string lineColor: "blue"
        property real lineWidth: 2.0
        property var lineDasharray: [3.0, 3.0]
    }
    
    MapParameter {
        type: "layer"
        
        property string name: "RMZLabels"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "RMZ"]
        property int minzoom: 10
    }
    
    MapParameter {
        type: "layout"
        
        property string layer: "RMZLabels"
        property var textField: ["get", "NAM"]
        property real textSize: 12
        property string iconImage: "WhiteBox"
        property string iconTextFit: "both"
        property var iconTextFitPadding: [2,5,2,5]
    }
    
    
    /*
     * TMZ - Transponder Mandatory Zone
     */
    
    MapParameter {
        type: "layer"
        
        property string name: "TMZ"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "TMZ"]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "TMZ"
        property string lineColor: "black"
        property real lineWidth: 2.0
        property var lineDasharray: [4.0, 3.0, 0.5, 3.0]
    }
    
    
    /*
     * Danger Zone - PJE
     */
    
    MapParameter {
        type: "layer"
        
        property string name: "PJE"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "PJE"]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "PJE"
        property string lineColor: "red"
        property real lineWidth: 2.0
        property var lineDasharray: [4.0, 3.0]
    }
    
    
    /*
     * Airspaces A, B, C, D that are not control zones
     */
    
    MapParameter {
        type: "layer"
        
        property string name: "ABCDOutlines"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "A"], ["==", ["get", "CAT"], "B"], ["==", ["get", "CAT"], "C"], ["==", ["get", "CAT"], "D"]]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "ABCDOutlines"
        property string lineColor: "blue"
        property real lineWidth: 2.0
    }
    
    MapParameter {
        type: "layer"
        
        property string name: "ABCDs"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "A"], ["==", ["get", "CAT"], "B"], ["==", ["get", "CAT"], "C"], ["==", ["get", "CAT"], "D"]]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "ABCDs"
        property string lineColor: "blue"
        property real lineOpacity: 0.2
        property real lineWidth: airspaceLineWidth
        property real lineOffset: airspaceLineWidth/2.0
    }
    
    
    /*
     * Control Zones
     */
    
    MapParameter {
        type: "layer"
        
        property string name: "controlZones"
        property string layerType: "fill"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "CTR"]
    }
    MapParameter {
        type: "paint"
        property string layer: "controlZones"
        property string fillColor: "red"
        property real fillOpacity: 0.2
    }
    
    MapParameter {
        type: "layer"
        
        property string name: "controlZoneOutlines"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "CTR"]
    }
    MapParameter {
        type: "paint"
        property string layer: "controlZoneOutlines"
        property string lineColor: "blue"
        property real lineWidth: 2.0
        property var lineDasharray: [4.0, 3.0]
    }
    MapParameter {
        type: "layer"
        
        property string name: "controlZoneLabels"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "CTR"]
        property int minzoom: 10
    }
    MapParameter {
        type: "layout"

        property string layer: "controlZoneLabels"
        property var textField: ["get", "NAM"]
        property real textSize: 12
        property string iconImage: "WhiteBox"
        property string iconTextFit: "both"
        property var iconTextFitPadding: [2,5,2,5]
    }

    
    /*
     * Nature Reserve Area
     */

    MapParameter {
        type: "layer"

        property string name: "natureReserveAreas"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "NRA"]
    }
    MapParameter {
        type: "paint"
        property string layer: "natureReserveAreas"
        property string lineColor: "green"
        property real lineOpacity: 0.15
        property real lineWidth: airspaceLineWidth
        property real lineOffset: airspaceLineWidth/2.0
    }

    MapParameter {
        type: "layer"

        property string name: "natureReserveAreaOutlines"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "NRA"]
    }
    MapParameter {
        type: "paint"
        property string layer: "natureReserveAreaOutlines"
        property string lineColor: "green"
        property real lineOpacity: 0.3
        property real lineWidth: 2.0
    }

    MapParameter {
        type: "layer"

        property string name: "natureReserveAreaLabels"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["==", ["get", "CAT"], "NRA"]
        property int minzoom: 10
    }
    MapParameter {
        type: "layout"

        property string layer: "natureReserveAreaLabels"
        property var textField: ["get", "NAM"]
        property real textSize: 12
        property string iconImage: "WhiteBox"
        property string iconTextFit: "both"
        property var iconTextFitPadding: [2,5,2,5]
    }


    /*
     * Danger Zone, Prohibited Zone, Restricted Zone
     */
    
    MapParameter {
        type: "layer"
        
        property string name: "dangerZones"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "DNG"], ["==", ["get", "CAT"], "R"], ["==", ["get", "CAT"], "P"]]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "dangerZones"
        property string lineColor: "red"
        property real lineOpacity: 0.2
        property real lineWidth: airspaceLineWidth
        property real lineOffset: airspaceLineWidth/2.0
    }
    
    MapParameter {
        type: "layer"
        
        property string name: "dangerZoneOutlines"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "DNG"], ["==", ["get", "CAT"], "R"], ["==", ["get", "CAT"], "P"]]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "dangerZoneOutlines"
        property string lineColor: "red"
        property real lineWidth: 2.0
        property var lineDasharray: [4.0, 3.0]
    }
    
    MapParameter {
        type: "layer"
        
        property string name: "dangerZoneLabels"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "R"], ["==", ["get", "CAT"], "P"]]
        property int minzoom: 10
    }
    MapParameter {
        type: "paint"
        property string layer: "dangerZoneLabels"
        property real textHaloWidth: 2
        property string textHaloColor: "white"
    }
    MapParameter {
        type: "layout"
        
        property string layer: "dangerZoneLabels"
        property var textField: ["get", "NAM"]
        property real textSize: 12
        property string iconImage: "WhiteBox"
        property string iconTextFit: "both"
        property var iconTextFitPadding: [2,5,2,5]
    }
    
    // End of airspaces

    /*
     * Procedures
     */
    
    MapParameter {
        type: "layer"
        
        property string name: "PRC_DEP"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["all", ["==", ["get", "CAT"], "PRC"], ["==", ["get", "USE"], "DEP"]]
        property int minzoom: 10
    }
    MapParameter {
        type: "paint"
        property string layer: "PRC_DEP"
        property var lineColor: ["get", "GAC"]
        property real lineWidth: 3.0
        property var lineDasharray: [3.0, 3.0]
    }

    MapParameter {
        type: "layer"

        property string name: "PRC_ARR"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["all", ["==", ["get", "CAT"], "PRC"], ["==", ["get", "USE"], "ARR"]]
        property int minzoom: 10
    }
    MapParameter {
        type: "paint"
        property string layer: "PRC_ARR"
        property var lineColor: ["get", "GAC"]
        property real lineWidth: 3.0
        property var lineDasharray: [9.0, 3.0]
    }

    MapParameter {
        type: "layer"

        property string name: "PRC_OTH"
        property string layerType: "line"
        property string source: "aviationData"
        property var filter: ["all", ["==", ["get", "CAT"], "PRC"], ["!=", ["get", "USE"], "ARR"], ["!=", ["get", "USE"], "DEP"]]
        property int minzoom: 10
    }
    MapParameter {
        type: "paint"
        property string layer: "PRC_OTH"
        property var lineColor: ["get", "GAC"]
        property real lineWidth: 3.0
    }

    // We print PRC labels first and then label the traffic circuits. This way,
    // traffic circuit labels will be printed with higher priority
    MapParameter {
        type: "layer"

        property string name: "PRCLabels"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["all", ["==", ["get", "CAT"], "PRC"], ["!=", ["get", "USE"], "TFC"]]
        property int minzoom: 10
    }
    MapParameter {
        type: "layout"

        property string layer: "PRCLabels"
        property string symbolPlacement: "line"
        property var textField: ["get", "NAM"]
        property real textSize: 16
    }
    MapParameter {
        type: "paint"
        property string layer: "PRCLabels"
        property real textHaloWidth: 10
        property string textHaloColor: "white"
    }

    MapParameter {
        type: "layer"
        
        property string name: "TFCLabels"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["all", ["==", ["get", "CAT"], "PRC"], ["==", ["get", "USE"], "TFC"]]
        property int minzoom: 10
    }
    MapParameter {
        type: "layout"
        
        property string layer: "TFCLabels"
        property string symbolPlacement: "line"
        property var textField: ["get", "NAM"]
        property real textSize: 16
    }
    MapParameter {
        type: "paint"
        property string layer: "TFCLabels"
        property real textHaloWidth: 10
        property string textHaloColor: "white"
    }

    /*************************************
     * Waypoints
     *************************************/

    // Define the necessary images
    
    MapParameter {
        type: "image"
        
        property string name: "AD"
        property string sprite: ":flightMap/sprites/AD"
    }
    MapParameter {
        type: "image"
        
        property string name: "AD-GLD"
        property string sprite: ":flightMap/sprites/AD-GLD"
    }
    MapParameter {
        type: "image"
        
        property string name: "AD-GRASS"
        property string sprite: ":flightMap/sprites/AD-GRASS"
    }
    MapParameter {
        type: "image"
        
        property string name: "AD-INOP"
        property string sprite: ":flightMap/sprites/AD-INOP"
    }
    MapParameter {
        type: "image"
        
        property string name: "AD-MIL"
        property string sprite: ":flightMap/sprites/AD-MIL"
    }
    MapParameter {
        type: "image"
        
        property string name: "AD-MIL-GRASS"
        property string sprite: ":flightMap/sprites/AD-MIL-GRASS"
    }
    MapParameter {
        type: "image"
        
        property string name: "AD-MIL-PAVED"
        property string sprite: ":flightMap/sprites/AD-MIL-PAVED"
    }
    MapParameter {
        type: "image"
        
        property string name: "AD-PAVED"
        property string sprite: ":flightMap/sprites/AD-PAVED"
    }
    MapParameter {
        type: "image"
        
        property string name: "AD-UL"
        property string sprite: ":flightMap/sprites/AD-UL"
    }
    MapParameter {
        type: "image"
        
        property string name: "AD-WATER"
        property string sprite: ":flightMap/sprites/AD-WATER"
    }
    MapParameter {
        type: "image"
        
        property string name: "MRP"
        property string sprite: ":flightMap/sprites/MRP"
    }
    MapParameter {
        type: "image"
        
        property string name: "NDB"
        property string sprite: ":flightMap/sprites/NDB"
    }
    MapParameter {
        type: "image"
        
        property string name: "RP"
        property string sprite: ":flightMap/sprites/RP"
    }
    MapParameter {
        type: "image"
        
        property string name: "VOR"
        property string sprite: ":flightMap/sprites/VOR"
    }
    MapParameter {
        type: "image"
        
        property string name: "VOR-DME"
        property string sprite: ":flightMap/sprites/VORDME"
    }
    MapParameter {
        type: "image"
        
        property string name: "VORTAC"
        property string sprite: ":flightMap/sprites/VORTAC"
    }
    MapParameter {
        type: "image"
        
        property string name: "DVOR"
        property string sprite: ":flightMap/sprites/VOR"
    }
    MapParameter {
        type: "image"
        
        property string name: "DVOR-DME"
        property string sprite: ":flightMap/sprites/VORDME"
    }
    MapParameter {
        type: "image"
        
        property string name: "DVORTAC"
        property string sprite: ":flightMap/sprites/VORTAC"
    }
    
    
    // NavAids - Text. Will be drawn with least priority
    
    MapParameter {
        type: "layer"
        
        property string name: "optionalText"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["==", ["get", "TYP"], "NAV"]
    }
    
    MapParameter {
        type: "layout"

        property string layer: "optionalText"
        property var textField: ["get", "COD"]
        property real textSize: 12
        property string textAnchor: "top"
        property var textOffset: [0, 1]
        property bool textOptional: true
    }
    
    MapParameter {
        type: "paint"
        property string layer: "optionalText"
        property real textHaloWidth: 2
        property string textHaloColor: "white"
    }

    
    // Gliding and UL Airfields
    
    MapParameter {
        type: "layer"
        
        property string name: "WPs"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "AD-GLD"], ["==", ["get", "CAT"], "AD-INOP"], ["==", ["get", "CAT"], "AD-UL"], ["==", ["get", "CAT"], "AD-WATER"]]
    }

    MapParameter {
        type: "layout"

        property string layer: "WPs"
        property var textField: ["get", "NAM"]
        property real textSize: 12
        property string textAnchor: "top"
        property var textOffset: [0, 1]
        property bool textOptional: true
        property var iconImage: ["get", "CAT"]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "WPs"
        property real textHaloWidth: 2
        property string textHaloColor: "white"
    }


    // Reporting points
    
    MapParameter {
        type: "layer"
        
        property string name: "RPs"
        property string layerType: "symbol"
        property string source: "aviationData"
        property int minzoom: 8
        property var filter: ["any", ["==", ["get", "CAT"], "RP"], ["==", ["get", "CAT"], "MRP"]]
    }
    
    MapParameter {
        type: "layout"

        property string layer: "RPs"
        property var textField: ["get", "SCO"]
        property real textSize: 12
        property string textAnchor: "top"
        property var textOffset: [0, 1]
        property bool textOptional: true
        property var iconImage: ["get", "CAT"]
    }
    
    MapParameter {
        type: "paint"
        property string layer: "RPs"
        property real textHaloWidth: 2
        property string textHaloColor: "white"
    }

    
    // GA Airfields with grass runway
    
    MapParameter {
        type: "layer"
        
        property string name: "AD-GRASS"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "AD-GRASS"], ["==", ["get", "CAT"], "AD-MIL-GRASS"]]
    }
    
    MapParameter {
        type: "layout"

        property string layer: "AD-GRASS"
        property var textField: ["get", "NAM"]
        property real textSize: 12
        property string textAnchor: "top"
        property var textOffset: [0, 1]
        property bool textOptional: true
        property var iconImage: ["get", "CAT"]
        property var iconRotate: ["get", "ORI"]
        property string iconRotationAlignment: "map"
    }
    
    MapParameter {
        type: "paint"
        property string layer: "AD-GRASS"
        property real textHaloWidth: 2
        property string textHaloColor: "white"
    }

    
    // NavAids - Icons. Will always be drawn, but might be overdrawn by other stuff.
    
    MapParameter {
        type: "layer"
        
        property string name: "NavAidIcons"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["==", ["get", "TYP"], "NAV"]
    }
    
    MapParameter {
        type: "layout"

        property string layer: "NavAidIcons"
        property var iconImage: ["get", "CAT"]
        property bool iconIgnorePlacement: true
        property bool iconAllowOverlap: true
    }

    
    // GA Airfields with paved or unknown runway
    
    MapParameter {
        type: "layer"
        
        property string name: "AD-PAVED"
        property string layerType: "symbol"
        property string source: "aviationData"
        property var filter: ["any", ["==", ["get", "CAT"], "AD"], ["==", ["get", "CAT"], "AD-PAVED"], ["==", ["get", "CAT"], "AD-MIL"], ["==", ["get", "CAT"], "AD-MIL-PAVED"]]
    }
    
    MapParameter {
        type: "layout"

        property string layer: "AD-PAVED"
        property var textField: ["get", "NAM"]
        property real textSize: 12
        property string textAnchor: "top"
        property var textOffset: [0, 1]
        property bool textOptional: true
        property var iconImage: ["get", "CAT"]
        property var iconRotate: ["get", "ORI"]
        property string iconRotationAlignment: "map"
    }
    
    MapParameter {
        type: "paint"
        property string layer: "AD-PAVED"
        property real textHaloWidth: 2
        property string textHaloColor: "white"
    }
    

} // End of FlightMap
