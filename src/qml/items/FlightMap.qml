/***************************************************************************
 *   Copyright (C) 2019-2026 by Stefan Kebekus                             *
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
import QtQml
import QtQuick
import QtQuick.Controls

import MapLibre 3.0
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

    /* Handle changes in zoom level */
    onZoomLevelChanged: {
        var vec1 = flightMap.fromCoordinate(flightMap.center, false)
        var vec2 = flightMap.fromCoordinate(flightMap.center.atDistanceAndAzimuth(10000.0, 0.0), false)
        var dx = vec2.x - vec1.x
        var dy = vec2.y - vec1.y
        pixelPer10km = Math.sqrt(dx*dx+dy*dy);
    }

    onMapReadyChanged: {
        zoomLevelChanged(zoomLevel)
    }

    maximumZoomLevel: 17
    minimumZoomLevel: 7.0001  // When setting 7 precisely, MapBox is looking for tiles of zoom 6, which we do not have…

    property real airspaceAltitudeLimitInFeet: {
        var aAL = GlobalSettings.airspaceAltitudeLimit
        if (aAL.isFinite())
        {
            return aAL.toFeet()
        }
        return 10e6
    }

    property geoCoordinate animatedCoordinate: PositionProvider.lastValidCoordinate
    Behavior on animatedCoordinate { CoordinateAnimation { duration: 1000 } }

    property real animatedTT: PositionProvider.lastValidTT.toDEG()
    Behavior on animatedTT { RotationAnimation {duration: 1000; direction: RotationAnimation.Shortest } }

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

        SourceParameter {
            id: waypointLib

            styleId: "waypointlib"
            type: "geojson"
            property string data: WaypointLibrary.GeoJSON
        }

        SourceParameter {
            id: notams

            styleId: "notams"
            type: "geojson"
            property string data: NOTAMProvider.geoJSON
        }

        SourceParameter {
            id: rasterTiles

            styleId: "rasterTiles"
            type: "raster"
            property string url: GeoMapProvider.serverUrl + "/rasterMap/"
            property int tileSize: GeoMapProvider.currentRasterMapTileSize
        }


        // Map layers, sorted according to importance, from low to high

        LayerParameter {
            id:  airspaceLabels

            styleId: "AirspaceLabels"

            type: "symbol"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet]]
            property string metadata: '{}'

            layout: {
                "symbol-placement": "line",
                "text-allow-overlap": false,
                "text-anchor": "center",
                "text-field": Navigator.aircraft.verticalDistanceUnit === Aircraft.Meters ? '["get", "MLM"]' : '["get", "MLI"]',
                "text-ignore-placement": false,
                "text-justify": "center",
                "text-offset": '[0,1]',
                "text-optional": true,
                "text-size": 0.85*GlobalSettings.fontSize
            }

            paint: {
                "text-halo-width": 2,
                "text-halo-color": "white"
            }
        }

        LayerParameter {
            id: prcLabels

            styleId: "PRCLabels"

            type: "symbol"
            property string source: "aviation-data"
            property var filter: [ "all", ["==", ["get", "CAT"], "PRC"], ["!=", ["get", "USE"], "TFC"] ]
            property real minzoom: 10

            layout: {
                "symbol-placement": "line",
                "text-field": ["get", "NAM"],
                "text-size": 1.14*GlobalSettings.fontSize,
                "symbol-spacing": 140
            }

            paint: {
                "text-color": "black",
                "text-halo-width": 10,
                "text-halo-color": "white"
            }
        }

        LayerParameter {
            id: tfcLabels

            styleId: "TFCLabels"

            type: "symbol"
            property string source: "aviation-data"
            property var filter: [ "all", ["==", ["get", "CAT"], "PRC"], ["==", ["get", "USE"], "TFC"] ]
            property real minzoom: 10

            layout: {
                "symbol-placement": "line",
                "text-field": ["get", "NAM"],
                "text-size": 1.14*GlobalSettings.fontSize,
                "symbol-spacing": 140
            }

            paint: {
                "text-color": "black",
                "text-halo-width": 10,
                "text-halo-color": "white"
            }
        }

        LayerParameter {
            id: optionalText

            styleId: "optionalText"

            type: "symbol"
            property string source: "aviation-data"
            property var filter: ["==", ["get", "TYP"], "NAV"]

            layout: {
                "text-field": ["get", "COD"],
                "text-size": 0.85*GlobalSettings.fontSize,
                "text-anchor": "top",
                "text-offset": [0, 1],
                "text-optional": true
            }

            paint: {
                "text-color": "black",
                "text-halo-width": 2,
                "text-halo-color": "white"
            }
        }

        LayerParameter {
            id: wps

            styleId: "WPs"

            type: "symbol"
            property string source: "aviation-data"
            property var filter: ["any", ["==", ["get", "CAT"], "AD-GLD"], ["==", ["get", "CAT"], "AD-INOP"], ["==", ["get", "CAT"], "AD-UL"], ["==", ["get", "CAT"], "AD-WATER"]]

            layout: {
                "icon-image": ["get", "CAT"],
                "text-field": ["get", "NAM"],
                "text-size": 0.85*GlobalSettings.fontSize,
                "text-anchor": "top",
                "text-offset": [0, 1],
                "text-optional": true
            }

            paint: {
                "text-color": "black",
                "text-halo-width": 2,
                "text-halo-color": "white"
            }
        }

        LayerParameter {
            id: rps

            styleId: "RPs"

            type: "symbol"
            property string source: "aviation-data"
            property var filter: ["any", ["==", ["get", "CAT"], "RP"], ["==", ["get", "CAT"], "MRP"]]

            layout: {
                "icon-image": ["get", "CAT"],
                "text-field": ["get", "NAM"],
                "text-size": 0.85*GlobalSettings.fontSize,
                "text-anchor": "top",
                "text-offset": [0, 1],
                "text-optional": true
            }

            paint: {
                "text-color": "black",
                "text-halo-width": 2,
                "text-halo-color": "white"
            }
        }

        LayerParameter {
            id: adGrass

            styleId: "AD-GRASS"

            type: "symbol"
            property string source: "aviation-data"
            property var filter: ["any", ["==", ["get", "CAT"], "AD-GRASS"], ["==", ["get", "CAT"], "AD-MIL-GRASS"]]

            layout: {
                "icon-image": ["get", "CAT"],
                "icon-rotate": ["get", "ORI"],
                "icon-rotation-alignment": "map",
                "text-field": ["get", "NAM"],
                "text-size": 0.85*GlobalSettings.fontSize,
                "text-anchor": "top",
                "text-offset": [0, 1],
                "text-optional": true
            }

            paint: {
                "text-color": "black",
                "text-halo-width": 2,
                "text-halo-color": "white"
            }
        }

        LayerParameter {
            id: navAidIcons

            styleId: "NavAidIcons"

            type: "symbol"
            property string source: "aviation-data"
            property var filter: ["==", ["get", "TYP"], "NAV"]

            layout: {
                "icon-image": ["get", "CAT"],
                "icon-ignore-placement": true,
                "icon-allow-overlap": true
            }
        }

        LayerParameter {
            id: adPaved

            styleId: "AD-PAVED"

            type: "symbol"
            property string source: "aviation-data"
            property var filter: ["any", ["==", ["get", "CAT"], "AD"], ["==", ["get", "CAT"], "AD-PAVED"], ["==", ["get", "CAT"], "AD-MIL"], ["==", ["get", "CAT"], "AD-MIL-PAVED"]]

            layout: {
                "icon-image": ["get", "CAT"],
                "icon-rotate": ["get", "ORI"],
                "icon-rotation-alignment": "map",
                "text-field": ["get", "NAM"],
                "text-size": 0.85*GlobalSettings.fontSize,
                "text-anchor": "top",
                "text-offset": [0, 1],
                "text-optional": true
            }

            paint: {
                "text-color": "black",
                "text-halo-width": 2,
                "text-halo-color": "white"
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

        LayerParameter {
            id: waypointLibParam

            styleId: "waypoint-layer"

            type: "symbol"
            property string source: "waypointlib"

            layout: {
                "icon-image": '["get", "CAT"]',
                "text-field": '["get", "NAM"]',
                "text-size": 0.85*GlobalSettings.fontSize,
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

        LayerParameter {
            id: notamParam

            styleId: "notam-layer"

            type: "symbol"
            property string source: "notams"

            layout: {
                "icon-ignore-placement": true,
                "icon-image": ["get", "CAT"],
                "text-field": ["get", "NAM"],
                "text-size": 0.85*GlobalSettings.fontSize,
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

        LayerParameter {
            id: fis
            styleId: "FIS"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["any", ["==", ["get", "CAT"], "FIR"], ["==", ["get", "CAT"], "FIS"]]]

            paint: {
                "line-color": "green",
                "line-width": 1.5
            }
        }

        LayerParameter {
            id: sua
            styleId: "SUA"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["==", ["get", "CAT"], "SUA"]]

            paint: {
                "line-color": "red",
                "line-width": 2,
                "line-dasharray": [4.0, 3.0]
            }
        }

        LayerParameter {
            id: glidingSector
            styleId: "glidingSector"
            type: "fill"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["==", ["get", "CAT"], "GLD"]]

            paint: {
                "fill-color": "yellow",
                "fill-opacity": 0.1
            }
        }

        LayerParameter {
            id: glidingSectorOutLines
            styleId: "glidingSectorOutLines"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["==", ["get", "CAT"], "GLD"]]

            paint: {
                "line-color": "yellow",
                "line-width": 2,
                "line-opacity": 0.8
            }
        }

        LayerParameter {
            id: rmz
            styleId: "RMZ"
            type: "fill"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["any", ["==", ["get", "CAT"], "ATZ"], ["==", ["get", "CAT"], "RMZ"], ["==", ["get", "CAT"], "TIZ"], ["==", ["get", "CAT"], "TIA"]]]

            paint: {
                "fill-color": "blue",
                "fill-opacity": 0.2
            }
        }

        LayerParameter {
            id: rmzOutline
            styleId: "RMZoutline"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["any", ["==", ["get", "CAT"], "ATZ"], ["==", ["get", "CAT"], "RMZ"], ["==", ["get", "CAT"], "TIZ"], ["==", ["get", "CAT"], "TIA"]]]

            paint: {
                "line-color": "blue",
                "line-width": 2,
                "line-dasharray": [3.0, 3.0]
            }
        }

        LayerParameter {
            id: tmz
            styleId: "TMZ"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["==", ["get", "CAT"], "TMZ"]]

            paint: {
                "line-color": "black",
                "line-width": 2,
                "line-dasharray": [4.0, 3.0, 0.5, 3.0]
            }
        }

        LayerParameter {
            id: pje
            styleId: "PJE"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["==", ["get", "CAT"], "PJE"]]

            paint: {
                "line-color": "red",
                "line-width": 2,
                "line-dasharray": [4.0, 3.0]
            }
        }

        LayerParameter {
            id: abcdOutlines
            styleId: "ABCDOutlines"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["any", ["==", ["get", "CAT"], "A"], ["==", ["get", "CAT"], "B"], ["==", ["get", "CAT"], "C"], ["==", ["get", "CAT"], "D"]]]

            paint: {
                "line-color": "blue",
                "line-width": 2
            }
        }

        LayerParameter {
            id: abcds
            styleId: "ABCDs"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["any", ["==", ["get", "CAT"], "A"], ["==", ["get", "CAT"], "B"], ["==", ["get", "CAT"], "C"], ["==", ["get", "CAT"], "D"]]]

            paint: {
                "line-color": "blue",
                "line-opacity": 0.2,
                "line-width": 7,
                "line-offset": 3.5
            }
        }

        LayerParameter {
            id: efgOutlines
            styleId: "EFGOutlines"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["any", ["==", ["get", "CAT"], "E"], ["==", ["get", "CAT"], "F"], ["==", ["get", "CAT"], "G"]]]

            paint: {
                "line-color": "blue",
                "line-width": 2
            }
        }

        LayerParameter {
            id: ctr
            styleId: "CTR"
            type: "fill"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["==", ["get", "CAT"], "CTR"]]

            paint: {
                "fill-color": "red",
                "fill-opacity": 0.2
            }
        }

        LayerParameter {
            id: ctrOutline
            styleId: "CTRoutline"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["==", ["get", "CAT"], "CTR"]]

            paint: {
                "line-color": "blue",
                "line-width": 2,
                "line-dasharray": [4.0, 3.0]
            }
        }

        LayerParameter {
            id: nraOutlines
            styleId: "NRAoutlines"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["==", ["get", "CAT"], "NRA"]]

            paint: {
                "line-color": "green",
                "line-width": 2
            }
        }

        LayerParameter {
            id: nra
            styleId: "NRA"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["==", ["get", "CAT"], "NRA"]]

            paint: {
                "line-color": "green",
                "line-opacity": 0.2,
                "line-width": 7,
                "line-offset": 3.5
            }
        }

        LayerParameter {
            id: dangerZonesOutlines
            styleId: "dangerZonesOutlines"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["any", ["==", ["get", "CAT"], "DNG"], ["==", ["get", "CAT"], "R"], ["==", ["get", "CAT"], "P"]]]

            paint: {
                "line-color": "red",
                "line-width": 2,
                "line-dasharray": [4.0, 3.0]
            }
        }

        LayerParameter {
            id: dangerZones
            styleId: "dangerZones"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "TYP"], "AS"], ["<=", ["get", "SBO"], flightMap.airspaceAltitudeLimitInFeet], ["any", ["==", ["get", "CAT"], "DNG"], ["==", ["get", "CAT"], "R"], ["==", ["get", "CAT"], "P"]]]

            paint: {
                "line-color": "red",
                "line-opacity": 0.2,
                "line-width": 7,
                "line-offset": 3.5
            }
        }

        LayerParameter {
            id: prcDep
            styleId: "PRC_DEP"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "CAT"], "PRC"], ["==", ["get", "USE"], "DEP"]]
            property real minzoom: 10.0

            paint: {
                "line-color": ["get", "GAC"],
                "line-width": 3.0,
                "line-dasharray": [3.0, 3.0]
            }
        }

        LayerParameter {
            id: prcArr
            styleId: "PRC_ARR"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "CAT"], "PRC"], ["==", ["get", "USE"], "ARR"]]
            property real minzoom: 10.0

            paint: {
                "line-color": ["get", "GAC"],
                "line-width": 3.0,
                "line-dasharray": [9.0, 3.0]
            }
        }

        LayerParameter {
            id: prcOth
            styleId: "PRC_OTH"
            type: "line"
            property string source: "aviation-data"
            property var filter: ["all", ["==", ["get", "CAT"], "PRC"], ["!=", ["get", "USE"], "ARR"], ["!=", ["get", "USE"], "DEP"]]
            property real minzoom: 10.0

            paint: {
                "line-color": ["get", "GAC"],
                "line-width": 3.0
            }
        }

        LayerParameter {
            id: rasterTileLayer

            styleId: "rasterTileLayer"
            type: "raster"
            property string source: "rasterTiles"

            layout: {
                "visibility": 'visible', // GeoMapProvider.currentRasterMap !== "" ? 'visible' : 'none'
                "raster-resampling": 'linear'
            }
        }
    }


    //
    // Additional Map Items
    //

    MapCircle { // Circle for nondirectional traffic warning
        center: PositionProvider.lastValidCoordinate

        radius: Math.max(500, TrafficDataProvider.trafficObjectWithoutPosition.hDist.toM())
        Behavior on radius {
            NumberAnimation { duration: 1000 }
            enabled: TrafficDataProvider.trafficObjectWithoutPosition.animate
        }

        color: TrafficDataProvider.trafficObjectWithoutPosition.color
        Behavior on color {
            ColorAnimation { duration: 400 }
            enabled: TrafficDataProvider.trafficObjectWithoutPosition.animate
        }
        opacity: 0.1
        visible: TrafficDataProvider.trafficObjectWithoutPosition.relevant
    }

    MapQuickItem {
        id: mapCircleLabel

        property real distFromCenter: 0.5*Math.sqrt(lbl.width*lbl.width + lbl.height*lbl.height) + 28

        coordinate: PositionProvider.lastValidCoordinate
        Behavior on coordinate {
            CoordinateAnimation { duration: 1000 }
            enabled: TrafficDataProvider.trafficObjectWithoutPosition.animate
        }

        visible: TrafficDataProvider.trafficObjectWithoutPosition.relevant

        Connections {
            // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
            // is not updated when the height of the map changes. It does get updated when the
            // width of the map changes. We use the undocumented method polishAndUpdate() here.
            target: flightMap
            function onHeightChanged() { mapCircleLabel.polishAndUpdate() }
        }

        Control { id: fontGlean }

        sourceItem: Label {
            id: lbl

            x: -width/2
            y: mapCircleLabel.distFromCenter - height/2

            text: TrafficDataProvider.trafficObjectWithoutPosition.description
            textFormat: Text.RichText

            font.pixelSize: 0.8*fontGlean.font.pixelSize

            leftInset: -4
            rightInset: -4
            bottomInset: -1
            topInset: -2

            background: Rectangle {
                border.color: "black"
                border.width: 1
                color: Qt.lighter(TrafficDataProvider.trafficObjectWithoutPosition.color, 1.9)

                Behavior on color {
                    ColorAnimation { duration: 400 }
                    enabled: TrafficDataProvider.trafficObjectWithoutPosition.animate
                }
                radius: 4
            }
        }
    }

    MapItemView { // Labels for traffic opponents
        model: TrafficDataProvider.trafficObjects
        delegate: Component {
            TrafficLabel {
                trafficInfo: modelData
            }
        }
    }

    MapPolyline {
        id: flightPath
        line.width: 4
        line.color: "#ff00ff"
        path: {
            var array = []
            //Looks weird, but is necessary. geoPath is an 'object' not an array
            Navigator.flightRoute.geoPath.forEach(element => array.push(element))
            return array
        }
    }

    MapPolyline {
        id: toNextWP
        visible: PositionProvider.lastValidCoordinate.isValid &&
                 (Navigator.remainingRouteInfo.status === RemainingRouteInfo.OnRoute)
        line.width: 2
        line.color: 'darkred'
        path: visible ? [PositionProvider.lastValidCoordinate, Navigator.remainingRouteInfo.nextWP.coordinate] : []
    }

    MapQuickItem {
        id: ownPosition

        coordinate: PositionProvider.lastValidCoordinate
        Behavior on coordinate { CoordinateAnimation { duration: 1000 } }

        Connections {
            // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
            // is not updated when the height of the map changes. It does get updated when the
            // width of the map changes. We use the undocumented method polishAndUpdate() here.
            target: flightMap
            function onHeightChanged() { ownPosition.polishAndUpdate() }
        }

        sourceItem: Item {

            rotation: flightMap.animatedTT-flightMap.bearing

            FlightVector {
                pixelPerTenKM: flightMap.pixelPer10km
                groundSpeedInMetersPerSecond: PositionProvider.positionInfo.groundSpeed().toMPS()
                visible: {
                    if (!PositionProvider.positionInfo.trueTrack().isFinite())
                        return false
                    if (!PositionProvider.positionInfo.groundSpeed().isFinite())
                        return false
                    if (PositionProvider.positionInfo.groundSpeed().toMPS() < 2.0)
                        return false
                    return true
                }
            }

            Image {
                id: imageOP

                x: -width/2.0
                y: -height/2.0

                source: {
                    var pInfo = PositionProvider.positionInfo

                    if (!pInfo.isValid()) {
                        return "/icons/self-noPosition.svg"
                    }
                    if (!pInfo.trueTrack().isFinite()) {
                        return "/icons/self-noDirection.svg"
                    }

                    return "/icons/self-withDirection.svg"
                }

                sourceSize.width: 50
                sourceSize.height: 50
            }
        }
    }

    MapItemView { // Traffic opponents
        model: TrafficDataProvider.trafficObjects
        delegate: Component {
            Traffic {
                map: flightMap
                trafficInfo: modelData
            }
        }
    }

    Component {
        id: waypointComponent

        MapQuickItem {
            id: midFieldWP

            anchorPoint.x: image.width/2
            anchorPoint.y: image.height/2
            coordinate: model.modelData.coordinate

            Connections {
                // This is a workaround against a bug in Qt 5.15.2.  The position of the MapQuickItem
                // is not updated when the height of the map changes. It does get updated when the
                // width of the map changes. We use the undocumented method polishAndUpdate() here.
                target: flightMap
                function onHeightChanged() { midFieldWP.polishAndUpdate() }
            }

            sourceItem: Item{
                Image {
                    id: image

                    source:  "/icons/waypoints/WP-map.svg"
                    sourceSize.width: 10
                    sourceSize.height: 10
                }
                Label {
                    anchors.verticalCenter: image.verticalCenter
                    anchors.left: image.right
                    anchors.leftMargin: 5
                    text: model.modelData.extendedName
                    color: "black" // Always black, independent of dark/light mode
                    visible: (flightMap.zoomLevel > 11.0) && (model.modelData.extendedName !== "Waypoint")
                    leftInset: -4
                    rightInset: -4
                    topInset: -2
                    bottomInset: -2
                    background: Rectangle {
                        opacity: 0.8
                        border.color: "black"
                        border.width: 0.5
                        color: "white"
                    }
                }
            }
        }

    }

    MapItemView {
        id: midFieldWaypoints
        model: Navigator.flightRoute.midFieldWaypoints
        delegate: waypointComponent
    }

} // End of FlightMap
