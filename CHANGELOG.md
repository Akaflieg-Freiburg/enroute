# Changelog

## [Unreleased]

### Added
- Airspace data can now be imported in the binary CUB format used by Naviter
  and LXNav products, such as SeeYou and the LX9000. (#601)

- Waypoints in the flight route can now be reordered by drag-and-drop. (#52)

- Flight routes can now be exported in Garmin FPL and MSFS PLN formats, for
  transfer to Garmin avionics and flight simulators. (#443, #640)

- On Android, flight routes, the waypoint library, flight logs and system
  information can now be saved directly to a file, in addition to sharing.
  (#363)

- Warn the user when pressure altitude and GNSS altitude differ by unrealistic
  amounts, which happens when the device does not measure static pressure, for
  instance in a pressurized cabin or when a flight simulator is used. In this
  situation, the side view no longer shows unreliable airspace data. (#570)

- OpenAIR import now understands the AY record (airspace type) and the
  airspace classes E, F and G. Gliding sectors and similar zones are now drawn
  with their proper airspace category, instead of appearing as generic special
  use airspace.

### Changed

- The safe-area handling that keeps the user interface clear of display
  cutouts, system bars and the virtual keyboard now uses Qt's SafeArea
  support instead of custom platform code. (#584)

### Fixed

- GDL90 Traffic: suppress "No GPS reception" warning when phone GNSS is primary
  source #673

- Fixes several issues with OpenAIR import.

- OpenAIR files that specify activation times now warn that these times are not
  evaluated, so that seasonal airspace is not mistaken for permanent airspace.

- The app no longer hangs on exit when there is no network connection. It used
  to wait for pending host name lookups, which take the full resolver timeout
  to fail when the network is unreachable (#544).

- Fixed a crash at startup on systems without any positioning backend, where
  the app dereferenced a missing satellite position source.

- Fixed the conversion between liters and U.S. gallons, which used the
  imperial gallon (4.546 l) instead of the U.S. gallon (3.785 l). Users who
  entered fuel consumption in gallons per hour should re-check the value in
  their aircraft settings, as the displayed figure will now differ.

- Fixed the OGN traffic alarm, which raised an alert for any aircraft flying
  well below the own position because the vertical distance was not taken as
  an absolute value.

- Fixed the computation of the distance between the aircraft and a route leg.
  Positions behind the start of a leg were attributed to the leg, and
  positions exactly on the leg could be reported as far away due to rounding.

- Fixed a hang when the weather server returned a truncated or malformed
  METAR/TAF response; the app no longer loops forever while parsing the data.

- Fixed an error where the map zoom level and rotation were not saved between
  sessions, so the map always reopened at the default zoom level.

- Fixed an error where a Bluetooth Low Energy traffic receiver was restored as
  a Bluetooth Classic connection after an app restart, so that it could no
  longer connect automatically.

- Fixed the internal map tile server, which listened on all network interfaces
  instead of the local loopback address only, and which did not validate tile
  coordinates in incoming requests.

- Fixed an error where chart names from a trip kit were used unchanged as file
  names, so that a malicious trip kit could write files outside the app's
  chart directory.

- Fixed the TIFF and ZIP file readers, which could crash or allocate unbounded
  memory when opening malformed or oversized files, such as corrupt GeoTIFF
  charts or trip kits.

- Fixed an error where the chart library could be lost if the app was
  interrupted while saving it, and where the library housekeeping deleted
  chart files that it had just re-registered. Unreadable chart files are now
  moved to a folder 'unrecognised' instead of being deleted.

- Fixed the helper scripts that update bundled third-party data; they no
  longer run 'git reset --hard' on the main repository when the target
  directory is missing.

- Fixed an error where the cached NOTAM data could be corrupted if the app was
  interrupted while saving it; the cache is now written atomically and a
  damaged cache is ignored instead of being loaded.

- Fixed an error where the current aircraft, the current flight route and
  saved routes or aircraft could be lost if the app was interrupted while
  writing them; these files are now written atomically and write errors are
  reported in the save dialogs.

- Fixed an error where the waypoint library could be emptied if the app was
  interrupted while saving it; the library file is now written atomically.

- Fixed an error where the aviation data cache and imported airspace files
  could be left half-written if the app was interrupted; they are now written
  atomically, and an import failure no longer deletes a previously imported
  file of the same name.

- Fixed an error where importing a chart or trip kit could leave a damaged
  chart file behind if the import was interrupted; chart files are now written
  atomically.

- Fixed an error where the list of traffic receiver connections and the stored
  Wi-Fi passwords could be lost if the app was interrupted while saving them;
  both files are now written through a shared atomic write helper.

- Fixed an error where exporting a file on Linux or macOS could leave a
  truncated file behind when the write failed, and where the weather cache
  write did not report failures.

- Fixed an error where flight routes exported as GPX contained unescaped
  waypoint names, so that names with characters such as '&' or '<' produced a
  file that other apps could not read.

- Fixed an error where the route summary lost its time and fuel figures when
  the route contained two waypoints less than 100 m apart anywhere except at
  the start.

- Fixed an error where the warning about a missing aviation map for the
  current location stayed visible after the map had been installed, until the
  aircraft moved.

- Fixed an error where the app did not notice a date change after sleeping
  across midnight, so that date-dependent displays such as the NOTAM list
  showed the previous day.

- Fixed an error where TAF validity periods ending at midnight (hour 24) were
  shown without an end time, and a remark in decoded METARs that read
  literally '%1 observed.' instead of naming the phenomenon.

- Fixed an error where the app requested the same METAR and TAF data several
  times in a row at startup and when returning to the foreground.

- Fixed an error where NOTAM texts containing characters such as '<' were cut
  off in the NOTAM list, and the grouping of NOTAMs that take effect more than
  90 days ahead, which now appear under their own heading.

- Fixed an error where the connection to the Open Glider Network could never
  be established on slow mobile links, because a watchdog aborted every
  connection attempt that took longer than one second. The watchdog now leaves
  attempts in progress alone, respects a manual disconnect and retries at most
  every ten seconds.

- Fixed several errors in TCP connections to traffic receivers: a Wi-Fi
  password that a device requested while it was already delivering data was
  stored as an empty password; disconnecting from a TCP traffic receiver was
  immediately undone by an automatic reconnect; and the keep-alive and low-
  delay socket options were requested before the socket existed and therefore
  never applied. The Open Glider Network connection had the same socket-option
  error.

- Fixed an error where only one Bluetooth Low Energy traffic receiver could be
  added on iOS, because every further device was mistaken for a duplicate of
  the first.

- Fixed an error where traffic that had disappeared from the map stayed
  invisible for a while after the device clock was corrected backwards,
  because expired traffic entries refused position reports with older
  timestamps.

- Fixed an error where a traffic receiver that sends data without sentence
  delimiters could make the app use ever more memory; the input buffer for
  FLARM data and the line reads from Bluetooth Classic and serial devices are
  now bounded.

- Fixed an error where traffic reports without an identification, such as
  Mode-C transponder targets reported by a PowerFLARM, were all treated as one
  and the same aircraft, so that the display animated between different
  aircraft as if one of them were moving.

- Fixed an error where the file holding stored Wi-Fi passwords for traffic
  receivers was created with default permissions; it is now readable by the
  owner only.

- Fixed an error in the ranking of traffic targets: when two targets were
  otherwise equal, only their horizontal distance was compared although the
  vertical distance was checked; the distance in space is now used.

- Fixed an error where an aviation map containing an airspace with a malformed
  coordinate could crash the app while the aviation data was loaded; such
  airspaces are now rejected.

- Fixed an error where a map download that could not be saved was silently
  treated as successful; the app now reports the failure and, before a map
  file is replaced or deleted, closes the map so that the update also works on
  Windows.

- Fixed an error where a map path received from the map server was used to
  build a local file name without checking that it stays inside the map
  directory; such entries are now ignored.

- Fixed an error where the tile server advertised zoom levels 6 to 10 for
  every map even if the map files covered a different range, which made the
  map renderer request tiles that do not exist instead of scaling the existing
  ones.

- Fixed an error on iOS where the map could stop loading tiles after the app
  returned from the background: when the tile server had to move to a new
  port, the tile descriptions still pointed to the old one.

- Fixed an error where an OpenAir airspace file containing the words nan or
  inf in place of a number was accepted and produced airspaces with invalid
  geometry; such numbers are now rejected.

- Fixed an error where importing a CUP waypoint file failed as a whole if it
  contained a blank line or a waypoint without elevation, and where the
  reported line number was one too small.

- Fixed a start-up race on Linux where the speech engine was wired into the
  app from a worker thread; only the slow construction now runs there, and
  queued voice notifications are spoken as soon as the engine is ready instead
  of after a polling delay.

- Fixed an error on iOS where a file whose name contains spaces or special
  characters could not be opened from another app, because the file URL was
  not decoded.

- Fixed an error where importing an approach chart handed over as a file URL
  by a desktop file manager failed with a misleading georeferencing error.

- Fixed an error on Android where generating a bug report could freeze the app
  for up to 30 seconds while collecting the system log.

- Fixed an error on Android where changing the system language restarted the
  app so abruptly that settings changed shortly before could be lost.

- Fixed an error where editing a coordinate between 1° west or south and 0° in
  degrees-and-minutes or degrees-minutes-seconds notation lost the sign, so
  that a waypoint near the equator or the Greenwich meridian flipped
  hemisphere.

- Fixed an error where entering a waypoint elevation in meters was ignored
  unless the feet field happened to be valid as well.

- Fixed an error where the OK button of the vector-map import dialog did not
  react to the map name being typed.

## [3.4.1] - 2026-08-06

### Fixed
- The app no longer crashes at startup when an unreasonably large aviation map
  is installed. Oversized maps are now skipped, and a warning asks the user to
  update the map data (#676).

## [3.4.0] - 2026-07-14

### Added
- Support for VAC collections distributed together with map sets.

- Add close button to the side view.

### Changed
- Improved pinch-to-zoom behavior on the moving map.

- Improved readability of NavBar on small displays.


### Fixed
- Fixes potential crashes on Android when picking files, on WiFi network
  changes, and when opening files without a name-

- Fixes receiving files shared by other Android apps ("Share with enroute").

- Fixes connection to USB serial devices that do not report a product name.

- User-defined waypoints no longer obscure airfields (#667)

- Fix night mode that was broken on some Android devices (#665)


## [3.3.3] - 2026-06-08

### Fixed
- Fixes sign error that makes OGN traffic always appear above the ownship position.


## [3.3.2] - 2026-05-28

### Fixed
- Fixes sporadic crash on Android when connecting devices via USB

- Fixes minor text layout issue on "Traffic" page

### Added
- Location sharing with the app "mapy.com"

### Removed
- Removed support for location sharing with Google Earth
 

## [3.3.1] - 2026-05-23

### Fixed
- Reporting Point shows Name (NAM) instead of Radio Code (SCO) (#648)

- Fixes crash on Android when receiving geo URLs

### Added
- Animate traffic on moving map


## [3.3.0] - 2026-05-11

### Added
- In moving map, use different for different types of traffic.

- Avoid showing traffic twice.


## [3.2.2] - 2026-04-22

### Added
- Improve NOTAM classification. (#638)


## [3.2.1] - 2026-04-19

### Added
- Improve NOTAM classification. (#629)

- Display magnetic variation of waypoints. (#633)


## [3.2.0] - 2026-04-11

### Added
- Added new icons to moving maps, for obstacle NOTAMs and NOTAMs about UAVs. (#625)


## [3.1.7] - 2026-04-03

### Fixed
- Issues with Bluetooth connections on macOS.

- Minor GUI graphics issue in connection info dialogs.

- Filter improbably high ground speed. (#623)


## [3.1.6] - 2026-03-16

### Fixed
- GeoTIFF import problems (#620)


## [3.1.5] - 2026-03-11

### Fixed
- Airspaces imported from OpenAIR files are not shown.


## [3.1.4] - 2026-02-09

### Fixed
- Maintain zoom, bearing, map center and display modes when toggling raster maps

- Visual approach charts now drawn on top of aviation map, not underneath

- Inconsistent behavior of slider when setting airspace altitude limit

- Simplified logic when setting map display modes, added toast messages for
  clarity

- Fix error in GUI of the aircraft page when loading aircraft from the library.

- Fix $PGRMZ message interpretation issue (#612)

- Ownship shown twice when using OGN data (#581)

- Warning "QML Loader cannot create delegate" (#578)

- Map is reset when airspace altitude limit changes (#551)

### Added
- Android libraries are now 16kb-aligned (#552)

- Tooltips on several buttons on the moving maps


## [3.1.0] - 2026-01-17

### Added
- Support serial port USB devices on Android (#411, #532, #537)

- Support external GPS devices (#564)

### Fixed
- Suppress confusing log messages

- Make links to manual more robust (#555)

- Wrong font size in scale items of moving map (#576)


## [3.0.51] - 2025-12-31

### Fixed
- Fixes problems with accessibility features under Android.


## [3.0.12] - 2025-12-26

### Fixed
- Scroll gesture scrolls airspace side view instead of dialog content.


## [3.0.11] - 2025-12-15

### Fixed
- Sharing System Information was inoperational (#595)

- Android SSL libraries not distributed (#594)


## [3.0.10] - 2025-12-14

### Added
- Serial port connections can now be configured on Linux and macOS platforms
  (#587)

- Serial ports (via USB) are now fully supported on the macOS platform (#591)

- Update manual to reflect improvements in serial port handling (#590)

- Support NMEA positioning devices, such as external GPS receivers (#564)

- Enroute now safely handle multiple connections to the same serial port device
  (#589)

### Fixed
- GUI issue where the aircraft symbol in the moving map appears too close to
  edge of screen (#573)

- Serial port connection issues (#564)

- Add proper privileges to flatpak, in order to access serial ports on Linux
  platforms (#586)

- Monitor serial connections and react when connections become (un)available
  (#585)


## [3.0.8] - 2025-11-27

### Fixed
- Erroneous "off route" messages (#572)


## [3.0.7] - 2025-11-25

### Fixed
- Ownship symbol too close to edge of screen (#573)

- No File Extension when exporting files (#565)

- GPS Height off in some Android devices (#549)


## [3.0.5] - 2025-11-17

### Fixed
- Issues in side view of airspaces with fault vertical borders (#568)


## [3.0.4] - 2025-11-15

### Fixed
- Fixed blurred display of raster maps with tiles of size 256x256 (#566)


## [3.0.3] - 2025-11-02

### Changed
- Added button to the moving map that can be used to open the side view.


## [3.0.1] - 2025-09-27

### Changed
- Android: Allow import of image files containing VACs, where bounding box
  coordinates are specified as part of the file name (#398)

- Reduce number of warnings when opening aircraft or routes from the library
  (#444)

- Hide warnings on traffic data receiver errors once the errors are gone (#406)

- Give meaningful error message when users try to import more than one file at a
  time via drag-and-drop. (#371)


## [3.0.0] - 2025-09-21

### Added
- Added side view of the airspace situation. (#345)

- Added option to use cabin pressure as static pressure, in order to compute
  pressure altitude and vertical distance to airspaces (#546, #454)


## [2.34.6] - 2025-08-07

### Fixed

- Fixed error where many data connections appear twice in the data connection
  dialog.


## [2.34.5] - 2025-08-05

### Changed

- Changed behavior of the "North Arrow" button in the moving map to make usage
  more intuitive.


## [2.34.4] - 2025-08-03

### Changed
- Changed behavior of "Direct To" in the waypoint description dialog to make it
  more useful. (#325, #449)

- Android binaries are now available for sideloading at GitHub
  https://github.com/Akaflieg-Freiburg/enroute/releases/latest


## [2.34.2] - 2025-07-18

### Fixed
- Fixed infrequent freeze on startup in Android.

### Added
- Support more Bluetooth Low Energy UART characteristics. (#529)


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


## [2.31.16] - 2024-11-04

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
