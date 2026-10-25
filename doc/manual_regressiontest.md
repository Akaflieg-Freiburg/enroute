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
* Aircraft (main menu)
    * Menu entry shows the active aircraft, or "Select Aircraft…"
    * Select an aircraft from the library
    * "Library…" opens the aircraft library

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
    * List aircraft, active aircraft highlighted
    * Tap to make aircraft active
    * Add aircraft (first aircraft becomes active)
    * Edit aircraft, including units; changes to the active aircraft apply immediately
    * Rename aircraft (active aircraft stays active)
    * Delete aircraft (deleting the active aircraft leaves none active)
    * Search
* Upgrade from a version with a separate current aircraft: it is imported into the library and active

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
