# Changelog

## [2.32.8] - 2025-02-05

### Changed
- Close waypoint description dialog once waypoint has been (added to|edited
  in|removed from) route or library. (#487)


## [2.32.7] - 2025-01-28

### Fixed
- Fixed problem where Route/Waypoint/Aircraft library pages do not open. (#488, #490)


## [2.32.6] - 2025-01-20

### Fixed
- METAR report is are now showing for airfields not close to current position or
  flight route. (#473)

### Added
- For better user experience on desktop-platforms, several list views now
  support key navigation. This feature is only available on Linux and macOS.

### Changed
- Updated the privacy statement. The server 'enroute-data' now maintains its own
  METAR/TAF database and no longer forwards requests to the aviation weather
  center.


## [2.32.5] - 2025-01-15

### Fixed
- The app no longer hangs on startup.

- Fixed typo in the manual.


### Fixed
- Fixed minor typo in the manual.


## [2.32.4] - 2024-12-24

### Added
- The moving map and waypoint dialogs now show DMEs.


## [2.32.3] - 2024-12-23

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
