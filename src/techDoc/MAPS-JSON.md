# The file "maps.json"

At regular intervals, the app **Enroute** downloads a JSON file "maps.json" from the internet, from an address that is hardcoded into the binary (see "MapManager.cpp" for details). The content of this file describes aviation-maps and base-maps that are available for download available. This is used to…

* Show a list of available maps to the user on the page "Download Maps"

* Warn the user when map updates become available.

This document describes the content of "maps.json".

## The Top-Level Object

The top-level object in **maps.json** is a dictionary with the following entries.

* **url** - the base URL where all maps can be downloaded. At the time of writing this document, this entry was https://cplx.vm.uni-freiburg.de/storage/enroute-GeoJSONv001

* **maps** - an array of dictionaries, one for every available map. These "map objects" are described below.

## The Map Objects

These are standard JSON dictionaries with the following entries.

* **path** - Path of the downloadable file, relative to the **url** that was specified in the top-level object

* **size** - Size of the map file on the server

* **time** - A string that describes the file creation day for the map file. This string is of the form *YYYYMMDD*.


