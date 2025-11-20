# Regression Test Enroute Flight Navigation

## General

###
* App Launch
    * First Install
        * Ask For Permission
            * Bluetooth
            * Network
            * Location
        * Download Map Data
    * Updated App
        * Should display releasenotes (optional)
        * Show working Map
        * Data was preserved
            * Airplane
            * Route
            * Map
            * VAC
            * Waypoints
    * Display stays on and does not go to standby

* MapView
    * Base data, Terrain and Airspaces
    * Aerodromes and other waypoints
    * Traffic pattern (when available)
    * Notams
    * Gestures for Zoom and Rotation
    * Waypoints
        * Open Waypoint with long and double tap
        * Show airspaces, elevation and NOTAM
        * when Aerodrome:
            * Show ID, COM, RWY and METAR, if available
        * Sat view
        * Route:
            * insert
            * append
            * delete waypoint


# Data
* Aircraft
    * Edit aircraft
    * Load aircraft
    * Save aircraft
    * Switch units

* Route and Wind
    * All menu items
    * Waypoints
        * Add
        * Remove
        * Move up / down
        * Tap to show waypoint information
    * Edit wind
        * Correct calculation
    * Calculation
        * Distance, ETE and consumption total
        * Distance, ETE TC and TH between waypoints

* Visual Approach Charts
    * File import
    * Import of Trip Kit
    * Search
    * Order by Distance to currentlocation
    * Move Map to selected VAC

* Nearby Waypoints
    * Show nearest weather information and Sunset
    * Show AD, WP and NAV
        * Details
        * Search

* Weather
    * Show nearest weather information and Sunset
    * List Stations with METAR information, sorted by distance


* Aircraft Library
    * List aircraft
    * Rename aircraft
    * Delete aircraft
    * Overwrite current aircraft
    * Search

* Flight Route Library
    * List
    * Search
    * Rename
    * Delete
    * Share (GPX and GeoJSON)
    * Open in Other App (not Apple)
    * Load

* Map and Data Library
    * All menu Items
    * Map
        * Show installed Maps
        * Uninstall Map
        * Install Map
        * Raster map
            * Install raster map
            * Uninstall raster map
    * VAC
        * All Menu Items


## Settings
* Night mode
    * Turn on and off, check all pages in main menu for contrasts and readability
