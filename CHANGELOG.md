# Changelog

## [2.34.0] - 2025-07-12

### Added
- Retrieve traffic data from the Open Glider Network (#512).


## [2.33.5] - 2025-07-03

### Changes
- Target Android API level 35.


## [2.33.4] - 2025-05-24

### Fixed
- Fix unexpected exit when editing flight routes. (#526)

- Fix problem renaming visual approach charts.


## [2.33.3] - 2025-05-10

### Fixed
- Fix problem with waypoint selection when no aviation data in installed. (#523)

- Fix problem with CUP file import, where official files provided by
  Austrocontrol could not be imported due to a problem with the parser (#522)


## [2.33.2] - 2025-05-06

### Fixed
- Fix problem with file import under Android.


## [2.33.1] - 2025-04-18

### Fixed
- Fix problem where TCP data connections were not restored on restart of the
  app.


## [2.33.0] - 2025-04-06

### Fixed
- Fix problem where VACs were deleted on every app update on iOS.

### Changed
- Even on very small screens, always show at least four characters of the
  destination waypoint in the remaining route bar. (#446)


## [2.32.14] - 2025-03-04

### Fixed
- Fix problem with the file picker dialog on Android, where files from external
  sources were not opened. (#491)

### Changed
- Increase distance threshold where aircraft is considered to be on-route from 3
  to 5 NM. (#503)


## [2.32.13] - 2025-03-02

### Fixed
- Fix problem with openAIR import. (#501)


## [2.32.12] - 2025-02-23

### Fixed
- Fix problem where irrelevant traffic is shown in the moving map. (#497)


## [2.32.11] - 2025-02-22

### Fixed
- Fix problems importing GeoJSON files produced by third-party software. (#496)


## [2.32.10] - 2025-02-21

### Added
- VACs can now be opened directly from the waypoint info dialog. (#492)

- Added search bar to VAC page. (#493)


## [2.32.9] - 2025-02-16

### Fixed
- The list of available maps was downloaded, but not processed on first startup.
  As a result, new users were shown a message that maps are not available for
  their location. (#485)

- Android users experienced very infrequent, but annoying ANRs.

### Changed
- Show list of all traffic on the page "Information/Traffic Data Receiver".
  Adjust the manual accordingly.


## [2.32.8] - 2025-02-05

### Changed
- Close waypoint description dialog once waypoint has been (added to|edited
  in|removed from) route or library. (#487)


## [2.32.7] - 2025-01-28

### Fixed
- Fix problem where Route/Waypoint/Aircraft library pages do not open. (#488, #490)


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
