## Architecture

**enroute flight navigation** is a fairly standard Qt application. The user interface is written in QML, the hard functionality is written in C++.

This document describes the C++ part of the program.  The documentation is continuously generated from the master branch of the enroute GIT repository.

### A. Moving map

The moving map is a standard QML Map object that uses the **mapboxgl** plugin. As mapboxgl expects the map data to come over the internet, there is a GeoMapProvider object that runs an internal http-server to provide the data.

### B. C++ classes exposed to QML

The main() function constructs a number of C++ objects and exposes them to QML.

#### B.1 C++ classes providing core functionality

- Instances of the classes GeoMapProvider and MapManager, under the names "mapManager" and "geoMapProvider". These classes allow access to the library of geographic maps, and provide the map data in formats suitable for use by QML.

- An instance of the class MobileAdaptor, under the name "MobileAdaptor". This class provides QML with access to platform-specific functionality, such as "vibrate device", "disable screen lock" and others

- An instance of the class SatNav, under the name "satNav". This class contains the satellite navigaton functionality and is really a thin wrapper around QGeoPositionInfoSource, that provides data in formats suitable for aviation purposes.

#### B.2 Other C++ classes

- An instance of the class GlobalSettings, under the name "globalSettings". This simple wrapper around QSetting allows QML retrieve and store settings.

- An instance of the class Library, under the name "library". This class helps to maintain a library of flight routes.

- Instances of the classes Aircraft, FlightRoute and Wind, under the name "aircraft", "flightRoute" and "wind". These classes are used for route and wind computation.

- An instance of the class Meteorologist, under the name "meteorologist".  This class provides weather info.






