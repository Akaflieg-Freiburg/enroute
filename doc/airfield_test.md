# Airfield Test Protocol — Enroute Flight Navigation 4.0.0

Pre-release tests that need a real device, a real traffic receiver or a real
flight. Desk-only checks (crafted files, network behaviour) are listed in the
preparation section and must be done before going to the field. Each test
names the change it covers; commit hashes refer to the `review` branch.

Fill in before starting:

* Date: ____________  Tester: ____________  PIC: ____________
* Airfield: ____________  Aircraft: ____________  Second aircraft / FLARM ID: ____________
* Android device + OS: ____________  App version + commit (Info page): ____________
* iOS device + OS: ____________  App version + commit (Info page): ____________
* Traffic receiver model / firmware: ____________

Result convention for every test: `[ ] pass  [ ] fail` plus notes. A test
that could not be run is marked `n/a` with the reason.


## 0. Preparation (at home, the day before)

* Builds
    * Android: release APK of the release candidate, installed **over** the
      previous release. Do not wipe app data: the persistence tests below need
      data from the old version.
    * iOS: TestFlight build of the same commit, installed over the previous
      release.
    * Optional: keep one device on the previous release for A/B comparison.
* Data on both devices
    * Aviation map and terrain data for the test region; one raster map.
    * Aircraft: fuel consumption unit set to **U.S. Gallons**, value 10.0 gal/h.
      Write down the l/h figure the *old* version shows for it: ________
    * Flight route with two legs: home field → a waypoint 10–20 NM away → home
      field. Prefer a waypoint roughly in line with the runway direction so
      that the two legs are close to collinear.
    * One manually imported approach chart (VAC) and one trip kit.
    * A FLARM replay file (lines of the form `<milliseconds> <NMEA sentence>`)
      copied to both devices.
    * A CUB airspace file for the region and an app that can open Garmin FPL
      or MSFS PLN files.
* Hardware
    * Traffic receiver charged; know whether it offers Bluetooth LE, Bluetooth
      Classic or both.
    * Second aircraft with FLARM; brief its pilot on when to taxi, when to
      start up and when to fly (tests G-05, F-04, F-05).
    * Internet on the phone for the OGN test; know how to toggle airplane mode.
* Desk-only checks done and recorded (commit hash: ____________)
    * Crafted TIFF with an oversized tag count imports with an error, no crash
      (TIFF/ZIP hardening).
    * Zip with a 300 MiB entry imports with an error, memory stays flat.
    * Trip kit whose chart name is `../../../pwned` does not write outside the
      VAC directory (trip-kit path sanitising).
    * `lsof -iTCP -sTCP:LISTEN -P | grep enroute` shows `127.0.0.1:<port>`
      only (tile server loopback).
    * `bash -n pull-*.sh` passes (scripts).


## A. On the ground, before engine start

* **G-01 Update persistence** — regression, see `manual_regressiontest.md`
    * Steps: launch the updated app.
    * Expected: release notes shown once; aircraft, route, maps, VACs and
      waypoints from the previous version are all still there.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **G-02 Map zoom and bearing survive a restart** — covers "Restore map
  bearing and zoom persistence"
    * Setup: on the map, zoom to a clearly non-default level (e.g. 9) and
      rotate the map by about 45° (bearing policy "user defined").
    * Steps: kill the app (swipe away), relaunch.
    * Expected: map reopens at the same zoom level and rotation. Repeat on the
      other platform.
    * Result Android: `[ ] pass  [ ] fail`  iOS: `[ ] pass  [ ] fail`  Notes: ________

* **G-03 Bluetooth LE receiver stays Bluetooth LE** — covers "Fix Bluetooth LE
  connections being restored as Classic"
    * Setup: Data Connections → New Connection → Bluetooth, add the receiver
      as a Bluetooth LE device; wait for "connected" and traffic/position data.
    * Steps: kill the app, relaunch, open Data Connections. Repeat the kill
      and relaunch a **second** time (the bug only showed on the third start).
    * Expected: the entry is still shown as Bluetooth LE and reconnects on its
      own without pressing "Reconnect". If the receiver also offers Bluetooth
      Classic, add it that way too and confirm it stays Classic.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **G-04 Gallon display** — covers "Fix U.S. gallon conversion factor"
    * Steps: open the aircraft page; switch the fuel unit between U.S. Gallons
      and Liters.
    * Expected: 10.0 gal/h shows as 37.9 l/h (the old version showed 45.5).
      Route page: fuel totals for the two-leg route are plausible for the
      aircraft.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **G-05 OGN traffic on the ground, no alarm** — covers "Fix OGN alarm level
  for traffic below own aircraft"
    * Setup: internet on; Data Connections → New Connection → Open Glider
      Network (accept the two warnings). Second aircraft parked about 50 m
      away with FLARM switched on.
    * Steps: wait up to two minutes; open the Traffic page.
    * Expected: the second aircraft is listed as traffic with a plausible
      distance; no red or yellow alarm, no aural warning.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **G-06 CUB airspace import** — 4.0.0 feature
    * Steps: open the CUB file with the app (share or file manager).
    * Expected: import dialog, then the airspaces are drawn on the map and
      listed on a waypoint underneath them.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **G-07 Route export as Garmin FPL and MSFS PLN** — 4.0.0 feature
    * Steps: Flight Route page menu → "… to Garmin FPL file" and "… to MSFS
      PLN file", both the share and the save-to-file variants.
    * Expected: the files open in the target app and show the two-leg route;
      on Android "save to file" creates the file where chosen.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **G-08 Pressure-vs-GNSS warning does not fire on the ramp** — 4.0.0 feature
    * Setup: receiver connected and delivering pressure altitude; aircraft
      parked with doors open.
    * Steps: wait two minutes.
    * Expected: no "Inconsistent altitude data" notification.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **G-09 Flight detection idle** — 4.0.0 feature
    * Setup: Flight Log page: "Automatic flight detection", "Record GPS
      track" and "Show live flight trace on map" all on.
    * Expected: status idle, no "Takeoff detected" while parked.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **G-10 FLARM replay smoke test (optional, desk or ramp)**
    * Steps: open the FLARM replay file with the app.
    * Expected: a transient "Simulator file" connection appears in Data
      Connections and traffic is shown. Remove the connection before flight.
    * Result: `[ ] pass  [ ] fail`  Notes: ________


## B. Taxi, takeoff, flight, landing

The PIC flies; the tester observes and writes. No test may influence the
conduct of the flight. Tests F-04 and F-05 need the second aircraft.

* **F-01 Taxi does not trigger takeoff** — 4.0.0 feature
    * Expected: during taxi (below the aircraft's minimum speed) the Flight
      Log status stays idle.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **F-02 Takeoff detection** — 4.0.0 feature
    * Expected: "Takeoff detected — confirming altitude…" on the takeoff roll,
      then "In flight — recording…" within 60 s / 200 ft above the field. The
      live trace is drawn on the map.
    * Result: `[ ] pass  [ ] fail`  Time of takeoff (UTC): ________

* **F-03 Leg following** — covers "Fix dead branch and NaN in point-to-leg
  distance"
    * Setup: the two-leg route is active.
    * Expected: after departure the **first** leg is active (next waypoint is
      the outbound waypoint, not the home field), its distance decreases;
      after passing the waypoint the second leg becomes active. Note any
      moment where the app reports "off route" while on track.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **F-04 OGN alarm in flight** — covers "Fix OGN alarm level for traffic below
  own aircraft"
    * Setup: OGN connection active, internet on. Second aircraft still parked
      with FLARM on.
    * Steps: overfly the field at 1000 ft AGL or higher.
    * Expected: the parked aircraft is shown as traffic with **no** alarm
      (the old version showed a red level-3 alert within 1 km). Then, with
      the second aircraft airborne within 2 km at a similar altitude, an
      alarm appears as before.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **F-05 FLARM traffic display** — regression
    * Setup: second aircraft airborne nearby.
    * Expected: relative bearing and altitude plausible on the map and the
      Traffic page; aural alert when the receiver raises a FLARM alarm.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **F-06 Pressure-vs-GNSS in flight** — 4.0.0 feature
    * Expected: no false "Inconsistent altitude data" notification; the side
      view shows airspace.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **F-07 Backgrounding** — covers "Restore map bearing and zoom persistence"
  and "Bind tile server to loopback" (iOS restart path)
    * Steps: put the app into the background for one minute, bring it back.
    * Expected: map tiles reload, zoom and bearing unchanged, traffic and
      position resume.
    * Result Android: `[ ] pass  [ ] fail`  iOS: `[ ] pass  [ ] fail`  Notes: ________

* **F-08 Landing detection** — 4.0.0 feature
    * Expected: "Landing detected — confirming…" on the rollout; the flight
      ends once below minimum speed; a log entry with the correct departure
      and arrival airfields appears.
    * Result: `[ ] pass  [ ] fail`  Time of landing (UTC): ________

* **F-09 Go-around (optional, PIC decision)** — 4.0.0 feature
    * Expected: a touch-and-go or go-around does not end the flight.
    * Result: `[ ] pass  [ ] fail  [ ] n/a`  Notes: ________


## C. After landing, at home

* **L-01 IGC track** — 4.0.0 feature
    * Steps: Flight Log → the flight → share the IGC track; open it in a
      viewer.
    * Expected: the track matches the flown path, times are UTC, no gaps of
      more than 5 s while recording.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **L-02 Flight log after restart** — 4.0.0 feature
    * Steps: kill and relaunch the app.
    * Expected: the flight is still listed; no "End Flight" button while idle.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **L-03 VAC library housekeeping** — covers "Save VAC library atomically and
  stop the janitor from deleting charts"
    * Steps: relaunch the app and open the VAC list. Then copy an unrelated
      `.webp` file into the app's VAC directory (Android: via a file manager
      with access to the app data, or adb) and relaunch again.
    * Expected: all charts from the preparation are still listed and open;
      the unrelated file ends up in `VAC/unrecognised` and is not deleted.
    * Result: `[ ] pass  [ ] fail  [ ] n/a`  Notes: ________

* **L-04 Gallon round trip** — covers "Fix U.S. gallon conversion factor"
    * Steps: set the fuel consumption to 8.5 gal/h, save the aircraft to the
      library, load it back.
    * Expected: 8.5 gal/h.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **L-05 Exit with airplane mode on** — regression for #544
    * Steps: airplane mode on, quit the app from its menu.
    * Expected: the app exits within two seconds.
    * Result: `[ ] pass  [ ] fail`  Notes: ________

* **L-06 Share Info** — diagnostics channel
    * Steps: Info page → "Share Info".
    * Expected: the shared text contains version, commit and Qt version, and
      on Android the last 300 log lines.
    * Result: `[ ] pass  [ ] fail`  Notes: ________


## D. What to capture when a test fails

* Immediately: Info page → "Share Info" (version, commit; on Android the log
  excerpt). Do this before restarting the app.
* iOS: Settings → Privacy & Security → Analytics & Improvements → Analytics
  Data, look for entries named "enroute"; or attach the device to Xcode and
  export the device log.
* A screenshot of the page in question; for map issues note the zoom level
  and whether a raster map was active.
* Flight detection issues: the IGC file and the takeoff and landing times
  from F-02 / F-08.
* OGN or FLARM issues: own callsign, the second aircraft's FLARM ID and the
  UTC time of the observation.
* Bluetooth issues: receiver model and firmware, and whether the same
  happens on the other platform.
* Note for every failure whether it reproduces on the other platform and on
  the previous release.
