/***************************************************************************
 *   Copyright (C) 2019-2022 by Stefan Kebekus                             *
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
    }

    maximumZoomLevel: 13.5
    minimumZoomLevel: 7.0001  // When setting 7 precisely, MapBox is looking for tiles of zoom 6, which we do not have…
    
    
    /*************************************
     * Aviation Data
     *************************************/
    
    /*
    DynamicParameter {
        type: "source"
        
        property string name: "aviationData"
        property string sourceType: "geojson"
        property string data: {
            if (global.dataManager().baseMapsRaster.hasFile)
                return global.geoMapProvider().emptyGeoJSON()
            return global.geoMapProvider().geoJSON
        }
    }
    */


    /*************************************
     * Sprite images
     *************************************/

    DynamicParameter {
        type: "image"
        
        property string name: "AD"
        property string sprite: ":flightMap/sprites/AD"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "AD-GLD"
        property string sprite: ":flightMap/sprites/AD-GLD"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "AD-GRASS"
        property string sprite: ":flightMap/sprites/AD-GRASS"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "AD-INOP"
        property string sprite: ":flightMap/sprites/AD-INOP"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "AD-MIL"
        property string sprite: ":flightMap/sprites/AD-MIL"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "AD-MIL-GRASS"
        property string sprite: ":flightMap/sprites/AD-MIL-GRASS"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "AD-MIL-PAVED"
        property string sprite: ":flightMap/sprites/AD-MIL-PAVED"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "AD-PAVED"
        property string sprite: ":flightMap/sprites/AD-PAVED"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "AD-UL"
        property string sprite: ":flightMap/sprites/AD-UL"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "AD-WATER"
        property string sprite: ":flightMap/sprites/AD-WATER"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "MRP"
        property string sprite: ":flightMap/sprites/MRP"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "NDB"
        property string sprite: ":flightMap/sprites/NDB"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "RP"
        property string sprite: ":flightMap/sprites/RP"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "VOR"
        property string sprite: ":flightMap/sprites/VOR"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "VOR-DME"
        property string sprite: ":flightMap/sprites/VORDME"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "VORTAC"
        property string sprite: ":flightMap/sprites/VORTAC"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "DVOR"
        property string sprite: ":flightMap/sprites/VOR"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "DVOR-DME"
        property string sprite: ":flightMap/sprites/VORDME"
    }
    DynamicParameter {
        type: "image"
        
        property string name: "DVORTAC"
        property string sprite: ":flightMap/sprites/VORTAC"
    }   

} // End of FlightMap
