# Changelog

## [2.32.3] - ??

### Added
- Zoom buttons no longer change places at max/min zoom level, one typo fixed in
  the GUI (#480)

- When opening a satellite view from the waypoint description dialog, the
  Android version of this app will use Google Earth if available. Otherwise,
  Google Maps is used as a fallback. (#448)

- In the Library "Maps and Data", show installed maps first (#323)


### Chores
- Substantial cleanup in the C++ code.


## [2.32.2] - 2024-11-26

### Added
- The METAR dialog shows a warning when density altitude substantially impairs
  aircraft performance (#463)


## [2.32.1] - 2024-11-21

### Added
- Enroute allows installation of several raster maps, which can then be switched
  on/off on the fly (#390)

- Upgrade underlying library to Qt 6.8

### Fixed
- Minor problems in the Italian translation


## [2.32.0] - 2024-11-11

### Added
- Enroute can now calculate density altitudes from METAR data (#408)

### Fixed
- Icons for reporting points no longer rotate along with the moving map (#453)


## [2.31.16] - 2024-11-4

### Added
- Enroute can now import PLN and FPL files as flight routes (#443, #342)


## [2.31.15] - 2024-10-27

### Fixed
- Icons for gliding and microlight sites no longer rotate along with the moving
  map (#453)


## [2.31.14] - 2024-10-22

### Added
- Setting "Font Size" now affects aviation-related items in the moving map
  (#409, #283)

### Fixed
- Enroute honors per-app language settings in Android 13+ (#442)
