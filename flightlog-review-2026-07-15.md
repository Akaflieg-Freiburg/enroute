# Qt Code Review Report — src/flightlog

Date: 2026-07-15
Scope: all C++ sources and headers in `src/flightlog/` (10 files)
Method: deterministic lint + six parallel deep-analysis agents (model contracts,
ownership/lifecycle, thread safety, API/C++ correctness, error handling,
performance/quality), findings deduplicated and verified against source.

Issues found: 37 (2 lint, 25 deep analysis, 10 investigation targets)

**TL;DR** — The code is cleanly single-threaded and mostly idiomatic, but there
are three genuine design-level problems: (1) **persistence is not crash-safe** —
both `flightlog.json` and IGC tracks are written with truncating, unchecked
`QFile` writes, and failures silently destroy data; (2) **the in-progress
flight is tracked by list index** across a sorted, user-editable list, which is
corruptible through normal UI actions; (3) **the flight-leg lifecycle is
smeared across three classes** (detector, recorder, FlightLog) held together by
comments and synchronous signal-ordering assumptions, including a fabricated
transient `Idle` state during touch-and-go.

**If you only fix five things**: D-001/D-008 (crash-safe persistence with
`QSaveFile`), D-004 (don't discard the flown track on a failed save), D-006
(stop tracking the in-progress flight by list index), D-002 (don't lose the
track when a flight is edited), D-003 (delete IGC files when their flight is
removed). The architectural item worth a think before the feature branch merges
is D-014 — the detector/recorder/FlightLog lifecycle coupling — since D-006,
D-010, and I-010 are all symptoms of it.

Fix status: [ ] = open. Mark items as you resolve them.

---

## Deep analysis findings

### [x] [D-001] save() truncate-writes with unchecked QFile — whole-log data loss
- **File**: `src/flightlog/FlightLog.cpp:684-691`
- **Category**: Error Handling — **Confidence**: 92/100
- **Finding**: `save()` opens `flightlog.json` with `WriteOnly` (truncates immediately) and ignores both the `open()` failure and the `write()` return. On disk-full or permission errors, the previously saved log is truncated or half-written and no error is surfaced anywhere. It is the sole persistence path, called from every CRUD operation.
- **Trace**: `save()` ← `addFlight`/`removeFlight`/`updateFlight`/`removeTrack`/`onLandingDetected`; no return value, no signal; QML has no persistence-error UI.
- **Mitigation**: Use `QSaveFile` with a checked `commit()` so the old file is never truncated on failure; report failures via a signal and surface them in the UI.

### [x] [D-002] updateFlight() silently discards trackFile, orphaning the IGC file
- **File**: `src/flightlog/FlightLog.cpp:249-273`
- **Category**: Model Contracts — **Confidence**: 90/100
- **Finding**: `trackFile` is a read-only Q_PROPERTY, so the QML edit dialog builds its replacement Flight via `createFlight()` which never sets it. `updateFlight()` preserves only coordinates from the old entry, so editing any flight that has a track sets `hasTrack` to false, orphans the IGC file, and — if that track was displayed — leaves `m_displayedTrackFile` naming a file no flight owns, with `displayedTrackIndex` flipping to −1 without its NOTIFY firing.
- **Trace**: `Flight.h:78` (READ-only property) → `createFlight()` → `FlightLogEntryEditor.qml` `resultFlight()` → `FlightLogPage.qml:683` → `updateFlight()` (preserves coordinates only).
- **Mitigation**: Carry the old entry's `trackFile` over in `updateFlight()` the same way coordinates are preserved, and re-validate `m_displayedTrackFile` after any list mutation.

### [x] [D-003] removeFlight() never deletes the flight's IGC file — permanent orphans
- **File**: `src/flightlog/FlightLog.cpp:217-246`
- **Category**: Ownership & Lifecycle — **Confidence**: 90/100
- **Finding**: All QML deletion paths (single remove, "clear all" loop, remove-selected) go through `removeFlight()`, which drops the entry but never calls `m_recorder.removeTrack()`. The IGC file stays in `AppDataLocation/tracks` forever with nothing referencing it and no orphan sweep anywhere — a storage leak on mobile devices.
- **Trace**: `FlightRecorder::removeTrack()` is reachable only via the separate delete-track dialog (`FlightLogPage.qml:595`); grep confirms no other deletion of files under the tracks directory.
- **Mitigation**: Delete the associated track file inside `removeFlight()` before erasing the entry, or add a startup sweep removing IGC files not referenced by any flight.

### [x] [D-004] saveTrack() ignores I/O errors; caller destroys the in-memory track regardless
- **File**: `src/flightlog/FlightRecorder.cpp:137-160`, `src/flightlog/FlightLog.cpp:840-847`
- **Category**: Error Handling — **Confidence**: 88/100
- **Finding**: `mkpath()` and `write()` returns are unchecked. On a failed open the method silently returns; on a short write, `setTrackFile()` is still called, leaving the flight referencing a corrupt file. Either way `onLandingDetected()` then calls `clearTrack()`, permanently discarding the just-flown GPS track from RAM with no notification — an unrecoverable data-loss path.
- **Trace**: `onLandingDetected`: `saveTrack(flight)` → `trackGeoPath()` → `clearTrack()`; no status flows back.
- **Mitigation**: Write via `QSaveFile`, have `saveTrack()` return success, only call `setTrackFile()`/`clearTrack()` on success, and notify the user on failure (the notification machinery already exists in this class).

### [x] [D-005] Full waypoint scan + O(n log n) distance-sort on every position update in flight
- **File**: `src/flightlog/FlightLog.cpp:341-347`; callers `src/flightlog/AirplaneFlightDetector.cpp:112,141,163`
- **Category**: Performance & Quality — **Confidence**: 88/100
- **Finding**: `nearestAirfield()` calls `nearbyWaypoints()`, which filters every aeronautical waypoint plus the library, then `std::sort`s the full list with haversine `distanceTo()` recomputed on both operands per comparison — and the caller uses only `.first()`. The detector runs this on every position fix (~1 Hz) for the entire `InFlight` and `LandingPhase` duration. Sustained per-second CPU cost on mobile for the whole flight.
- **Trace**: `onPositionUpdated` → `processPositionUpdate` → `case InFlight` → `nearestAirfield` → full filter + full sort (`GeoMapProvider.cpp:486-518`).
- **Mitigation**: Replace sort with a single-pass minimum computing each distance once; additionally throttle the in-flight landing check (re-query only after moving a few hundred meters, or skip while well above any plausible field elevation).

### [x] [D-006] In-progress flight tracked by fragile list index — corruptible via normal UI use
- **File**: `src/flightlog/FlightLog.cpp:794-797, 169-177, 649-677`
- **Category**: Ownership & Lifecycle — **Confidence**: 85/100
- **Finding**: Two corruption paths for `m_currentFlightIndex`. **(a) Verified:** `addFlight()` prepends without adjusting the index, then `sortFlights()` re-anchors by reading `m_flights[m_currentFlightIndex]` — now the wrong (shifted) element. Manually adding a flight from QML while a recording is in progress makes the manual flight become "the current flight", and on landing the arrival data and track are written into it while the real flight stays incomplete. **(b) Conditional:** `onTakeoffDetected` sets `m_currentFlightIndex = 0` with the comment "always at index 0 due to prepending", but `addFlight()` sorts by startTime — any existing future-dated entry (typo'd manual date) sorts above it, so landing data hits the wrong flight. `sortFlights()`'s re-identification key `(startTime, trackFile)` is also non-unique for in-progress flights (empty trackFile).
- **Trace**: `addFlight` prepend → `sortFlights` reads stale index; comment at line 796 contradicts the sort. Three independent review passes converged; path (a) verified directly against the source.
- **Mitigation**: Track the in-progress flight by stable identity (e.g. a UUID member on Flight) instead of a list index, or at minimum increment the index on prepend and have `addFlight` return the actual post-sort insertion index.

### [x] [D-007] showTrack() mutates displayed-track state before its failure check
- **File**: `src/flightlog/FlightLog.cpp:595-611`
- **Category**: Model Contracts — **Confidence**: 88/100
- **Finding**: `m_displayedTrackPath` is assigned before the `isEmpty()` early-return. Showing a track whose IGC file is missing/unreadable clobbers the currently displayed track's cached path to empty while `m_displayedTrackFile` still names the old track and no NOTIFY is emitted — the property value changes silently and the UI state becomes internally inconsistent.
- **Trace**: `loadTrackPath()` returns `{}` on open failure → early return after the member assignment, before `emit`.
- **Mitigation**: Load into a local, return early on failure without touching members, and assign both members together on success before emitting.

### [x] [D-008] load() silently discards a corrupt log, which the next save() then overwrites
- **File**: `src/flightlog/FlightLog.cpp:694-722`
- **Category**: Error Handling — **Confidence**: 82/100
- **Finding**: A parse failure or content-tag mismatch leaves `m_flights` empty with no diagnostics (no `QJsonParseError` consulted) and no user notification. The first subsequent CRUD operation calls `save()`, truncating the possibly recoverable file with a near-empty log. Combined with D-001, one bad write cycle silently destroys the entire logbook.
- **Trace**: Constructor calls `load()`; any CRUD then calls `save()` over the same `m_fileName`.
- **Mitigation**: On parse failure, rename the damaged file aside (e.g. `flightlog.json.corrupt`) before any save can run, log via `QJsonParseError`, and inform the user.

### [x] [D-009] Export failure indistinguishable from success — UI shares a 0-byte file with a success toast
- **File**: `src/flightlog/FlightLog.cpp:355-361`, `src/flightlog/FlightRecorder.cpp:189-200`
- **Category**: Error Handling — **Confidence**: 83/100
- **Finding**: `exportToIGC()` returns `{}` identically for bad index, no track, and read failure. QML passes the bytes straight to `FileExchange.shareContent`, which writes a 0-byte file and reports success — the user sees "shared" for an empty .igc even though the menu was enabled by `hasTrack`. Same pattern in the CSV/JSON exports.
- **Trace**: `FlightLogPage.qml:505-509` → `shareContent` (`FileExchange_Linux.cpp:52-89` — only destination I/O checked).
- **Mitigation**: Distinguish failure from legitimately empty content (error out-parameter or signal), or have QML check for empty bytes and show the existing share-error dialog.

### [x] [D-010] Stale live track retained in RAM and on the map when settings toggle mid-flight
- **File**: `src/flightlog/FlightLog.cpp:882-905, 840-847`
- **Category**: Ownership & Lifecycle — **Confidence**: 80/100
- **Finding**: The recorder's track is cleared only on entering `TakeoffPhase` and in `onLandingDetected` — the latter only when `trackRecording()` is true. If auto-detection is disabled mid-flight, `onPositionUpdated` resets the detector but never touches the recorder; if track recording is toggled off before landing, neither save nor clear happens. Either way, thousands of points stay in RAM and `displayedTrackPath()` keeps drawing the frozen live trace on the map until the next takeoff. The stale points can even be prepended to the next flight's saved track in some toggle sequences, since only the `TakeoffPhase` transition clears them.
- **Trace**: Early return at `FlightLog.cpp:885-891` bypasses the recorder; the recorder's documented contract ("call on every position update") is broken by the conditional forwarding at line 901.
- **Mitigation**: Always forward detection state to the recorder (let it decide whether to append), clear the recorder whenever detection is reset/disabled, and clear on landing regardless of the recording setting — emitting `displayedTrackPathChanged`.

### [x] [D-011] Unbounded track growth when the detector is stuck InFlight
- **File**: `src/flightlog/FlightRecorder.cpp:74-84`, `src/flightlog/AirplaneFlightDetector.cpp:105-130`
- **Category**: Ownership & Lifecycle — **Confidence**: 80/100
- **Finding**: Landing detection requires being within 5 km of a mapped "AD" waypoint. After an off-field landing or a landing at an unmapped field, the detector stays `InFlight` indefinitely and — with the Android foreground service keeping GPS alive — the recorder appends points (~1 per 10 s) with no cap, for days if unattended. `endFlight()` requires manual action.
- **Trace**: `shouldRecord()` bounds the rate, not the total; no code path bounds `m_track.size()`.
- **Mitigation**: Cap the point count (decimate or coarsen the sampling interval past a threshold), or add a stale-InFlight timeout/plausibility check in the detector.

### [x] [D-012] trackRecording / showCurrentFlightTrace proxies break the NOTIFY contract in both directions
- **File**: `src/flightlog/FlightLog.cpp:179-214`
- **Category**: Model Contracts — **Confidence**: 82/100
- **Finding**: These properties' real storage is `GlobalSettings`, which exposes the same values as QML-writable properties with its own NOTIFY signals — but FlightLog never connects them, so a write through the GlobalSettings facade leaves every `FlightLog.trackRecording` binding stale (two competing sources of truth; latent today since QML currently writes only via FlightLog). Conversely, FlightLog's setters emit unconditionally even when the guarded GlobalSettings setter early-returned on no change — NOTIFY without a value change (`setShowCurrentFlightTrace` also emits `displayedTrackPathChanged` every time).
- **Trace**: `deferredInitialization()` connects only `autoFlightDetectionChanged`; `GlobalSettings.cpp:256-275` has equality-guarded setters.
- **Mitigation**: Connect the GlobalSettings NOTIFY signals to FlightLog's in `deferredInitialization()` and remove the direct emits from FlightLog's setters, making the guarded setter the single emission source — or drop the proxies and bind QML to GlobalSettings directly.

### [x] [D-013] displayedTrackIndex value changes on list mutations without its NOTIFY firing
- **File**: `src/flightlog/FlightLog.h:113`, `src/flightlog/FlightLog.cpp:169-177`
- **Category**: Model Contracts — **Confidence**: 82/100
- **Finding**: `displayedTrackIndex` is a function of flight-list order but is declared `NOTIFY displayedTrackPathChanged`. `addFlight()` (a takeoff while a saved track is displayed shifts the displayed flight from i to i+1) and `removeFlight()` of an earlier flight both change the getter's value while emitting only `flightsChanged`. Currently partly masked by wholesale model rebuilds, but the property contract is broken and any non-delegate binding sees stale values.
- **Trace**: Property declaration vs. mutation paths; QML bindings at `FlightLogPage.qml:182` and `:462`.
- **Mitigation**: Emit `displayedTrackPathChanged` from every path that mutates `m_flights` while a track is displayed.

### [x] [D-014] Flight-leg lifecycle split across three classes, held together by signal-ordering assumptions and a fabricated Idle state
- **File**: `src/flightlog/AirplaneFlightDetector.cpp:161-195`, `src/flightlog/FlightRecorder.h:46-51`, `src/flightlog/FlightLog.cpp:840-847`
- **Category**: Design — **Confidence**: 80/100
- **Finding**: Ending a recording depends on the detector emitting `landingDetected` synchronously inside `processPositionUpdate` so that FlightLog saves and clears the recorder *before* FlightLog feeds the same position update to the recorder. The go-around branch even transitions `LandingPhase → Idle → InFlight` within one call, with a comment admitting the fake Idle exists to satisfy FlightLog's internal ordering ("ensures onLandingDetected saves the track while m_flights[0] is still the current leg") — a FlightLog invariant leaking into detector state-machine logic. Observers see a fabricated momentary Idle and `detectionStateChanged` fires twice per fix. Any change to connection type (queued), detector implementation, or slot ordering silently breaks track attachment.
- **Trace**: Comments at `AirplaneFlightDetector.cpp:168-171` and `FlightRecorder.h:46-51` explicitly document the cross-class coupling; call order in `FlightLog.cpp:897-904`.
- **Mitigation**: Give one component ownership of the leg lifecycle: have the detector emit semantic events only (takeoff, touchAndGo, landing) without staging fake states, and let FlightLog orchestrate recorder save/clear from those events. If the direct-connection invariant must remain, document or assert it.

### [ ] [D-015] FlightLog is a god class (seven-plus responsibilities, ~980 lines)
- **File**: `src/flightlog/FlightLog.cpp` (whole file)
- **Category**: Design — **Confidence**: 82/100
- **Finding**: One class covers JSON persistence, flight CRUD + sorting + coordinate resolution, a ~170-line inline ForeFlight CSV formatter, JSON export, Android JNI notification/foreground-service plumbing, an Android no-GPS watchdog, displayed-track selection state, and settings proxying. The CSV formatter is pure serialization with no FlightLog-state dependency; the platform plumbing is unrelated to logbook management. Hard to test; every concern shares one notification surface.
- **Trace**: CSV at lines 364-533; persistence 684-748; platform code 46-77, 799-815, 859-874, 908-979; settings proxy 179-214.
- **Mitigation**: Extract the ForeFlight CSV writer (parallel to `FlightRecorder::toIGC`) and move the Android notification/service/watchdog code into a platform helper; FlightLog remains the model/coordinator.

### [x] [D-016] Dead constant airfieldProximityM; the real thresholds are magic numbers in other places
- **File**: `src/flightlog/AirplaneFlightDetector.h:89`, `src/flightlog/FlightLog.cpp:347`, `src/flightlog/AirplaneFlightDetector.cpp:86,135`
- **Category**: API & C++ Correctness — **Confidence**: 95/100
- **Finding**: `airfieldProximityM = 5000.0` is declared and documented but never referenced — the actual 5 km cutoff is a hardcoded `5000.0` inside `FlightLog::nearestAirfield()`, a different class. Tuning the constant would silently do nothing. The two 60-second confirmation timeouts are likewise bare `> 60` literals while every other detection threshold has a named, documented constexpr.
- **Trace**: Grep confirms the declaration is the only occurrence; all detector branches delegate proximity to `nearestAirfield`.
- **Mitigation**: Pass the radius into `nearestAirfield()` as a parameter defaulted from the named constant (it is detector policy, not FlightLog policy), and name the two timeout constants.

### [x] [D-017] endFlight() documented contract contradicts the implementation, in two headers
- **File**: `src/flightlog/FlightDetector.h:86-94`, `src/flightlog/FlightLog.h:193-199`, vs `src/flightlog/AirplaneFlightDetector.cpp:205`
- **Category**: API & C++ Correctness — **Confidence**: 90/100
- **Finding**: Both docs say "Does nothing if the detection state is not InFlight", but the implementation deliberately also acts in `LandingPhase` (it uses the accumulated `m_landingCount` there). A future detector subclass implementing the documented narrower contract would diverge from the existing behavior.
- **Trace**: `landingCount = (m_detectionState == LandingPhase) ? m_landingCount : 1` proves LandingPhase handling is intentional; the docs are wrong.
- **Mitigation**: Update both doc comments to the actual contract (InFlight or LandingPhase).

### [x] [D-018] Seven-line detector state-reset block triplicated, plus a diverging partial variant
- **File**: `src/flightlog/AirplaneFlightDetector.cpp:149-156, 230-237, 248-255` (partial variant 175-177)
- **Category**: API & C++ Correctness — **Confidence**: 85/100
- **Finding**: The full pending-state reset appears identically three times (landing confirmation, `endFlight`, `resetDetection`), plus a deliberate partial variant in the go-around branch. `resetDetection()` can't be reused due to its Idle early-return. When a new pending member is added, some path will silently miss resetting it.
- **Mitigation**: Factor a private `clearPendingState()` helper (no signal, no early return); callers emit signals themselves.

### [x] [D-019] Duplicated export index-collection and duration-formatting logic
- **File**: `src/flightlog/FlightLog.cpp:370-384` vs `538-551`; `src/flightlog/Flight.cpp:44-56` vs `59-68`
- **Category**: API & C++ Correctness — **Confidence**: 85/100
- **Finding**: The 14-line "collect flights to export" block is copy-pasted byte-for-byte between `exportToForeFlight` and `exportToJSON`. The H:MM formatting is likewise duplicated between `blockTime()` and `flightTime()`.
- **Mitigation**: Extract `flightsForIndices(const QVariantList&)` in FlightLog and a file-local duration formatter in Flight.cpp.

### [x] [D-020] flights property copies the full list per read; delete-all is O(n²) with n disk writes
- **File**: `src/flightlog/FlightLog.h:88-89`, `src/flightlog/FlightLog.cpp:169-246`
- **Category**: Performance & Quality — **Confidence**: 85/100
- **Finding**: The list is a by-value `QList<Flight>` property with no granular change notification, so every mutation rebuilds the entire QML model and rewrites the entire JSON file. The QML "clear all" path loops `removeFlight(0)` — n full-file writes and n model resets. `removeTrack()` also saves and emits even when it was a no-op.
- **Mitigation**: Tolerable at logbook scale, but provide batch operations (`removeFlights(indices)`, `clear()`) that save/emit once; longer-term, a `QAbstractListModel` gives row-level updates.

### [x] [D-021] Live track path rebuilt from scratch on every recorded point, read twice by QML
- **File**: `src/flightlog/FlightLog.cpp:578-592`, `src/flightlog/FlightRecorder.cpp:126-134`
- **Category**: Performance & Quality — **Confidence**: 82/100
- **Finding**: Each appended point emits a change; `displayedTrackPath()` then reconstructs the whole `QList<QGeoCoordinate>` from `m_track`, and `FlightMap.qml` binds the property in two places (one doing a JS per-element copy) — so each fix costs two O(n) rebuilds plus a JS array copy, growing linearly over a multi-hour flight, all on the UI thread.
- **Mitigation**: Maintain the geo path incrementally in the recorder so `trackGeoPath()` returns a cheap COW copy; in QML, read the property once into a local. Longer term, an appended-point signal lets the map extend the polyline instead of replacing it.

### [x] [D-022] Missing const / [[nodiscard]] on non-mutating export methods
- **File**: `src/flightlog/FlightLog.h:232, 242, 253`, `src/flightlog/FlightRecorder.h:114`
- **Category**: API & C++ Correctness — **Confidence**: 90/100
- **Finding**: `exportToIGC`, `exportToForeFlight`, and `exportToJSON` only read state but are non-const, while the analogous `lastArrivalICAO()` is const. Root blocker: `FlightRecorder::exportToIGC` is non-const and lacks `[[nodiscard]]`, though its sibling `loadTrackPath` (same kind of file read) has both.
- **Mitigation**: Mark `FlightRecorder::exportToIGC` const + `[[nodiscard]]`, then const-qualify the three FlightLog exports (Q_INVOKABLE works on const methods).

### [x] [D-023] Inconsistent PositionInfo passing: detector takes by value, recorder by const-ref
- **File**: `src/flightlog/FlightDetector.h:84` vs `src/flightlog/FlightRecorder.h:76-78`
- **Category**: API & C++ Correctness — **Confidence**: 80/100
- **Finding**: `FlightDetector::processPositionUpdate(PositionInfo info)` copies on every GPS fix (`QGeoPositionInfo` is a non-implicitly-shared d-pointer class), while the sibling recorder API and the upstream signal use const-ref. Both are called side by side with the same object.
- **Mitigation**: Change the virtual (base and override) to `const Positioning::PositionInfo&`.

### [x] [D-024] Abstract FlightDetector has one implementation and no swap API despite docs promising one
- **File**: `src/flightlog/FlightDetector.h:45`, `src/flightlog/FlightLog.cpp:90`
- **Category**: Ownership & Lifecycle — **Confidence**: 82/100
- **Finding**: `AirplaneFlightDetector` is the sole subclass; `m_detector` is assigned exactly once and there is no swap method, yet the FlightLog class doc claims the detector "can be swapped at runtime". The repeated `m_detector != nullptr` guards protect a pointer that is never null after construction — while the one place it actually matters, `onDetectionStateChanged()` at `FlightLog.cpp:772`, dereferences unguarded. Low severity — the paraglider intent is documented, so this may be accepted design.
- **Mitigation**: Either add the promised swap API (with disconnect of the old detector) or trim the doc claim; make the null-guarding consistent either way.

### [x] [D-025] Dangling comment for a removed member
- **File**: `src/flightlog/FlightLog.h:369-370`
- **Category**: Performance & Quality — **Confidence**: 90/100
- **Finding**: "// Whether track recording is enabled" is followed by a blank line and the unrelated `m_detector` declaration — leftover from the field that moved into GlobalSettings.
- **Mitigation**: Delete the stale comment.

---

## Investigation targets (human verification needed)

### [ ] [I-001] aircraftCallsign is populated with Aircraft::name(), which is not a callsign
- **File**: `src/flightlog/AirplaneFlightDetector.cpp:99`, `src/flightlog/Flight.h:65-66`
- **Category**: API & C++ Correctness — **Confidence**: 72/100
- **Finding**: `takeoffDetected` feeds `aircraft().name()` into a field documented as "e.g. D-KEBE" that flows into the ForeFlight `AircraftID` column and the `lastArrivalICAO` lookup key. `Navigation::Aircraft` has no registration property; users naming their aircraft "Cessna club plane" get non-registration logbook IDs.
- **Unverified because**: May be an accepted app convention (users instructed to use the registration as the name).
- **How to verify**: Check the aircraft settings UI/manual; if free-form, rename to `aircraftName` or document the assumption.

### [x] [I-002] landingCount increments on LandingPhase entry — go-arounds without touchdown count as landings
- **File**: `src/flightlog/AirplaneFlightDetector.cpp:126`
- **Category**: Domain semantics — **Confidence**: 70/100
- **Finding**: `m_landingCount++` fires on descending below 100 ft AGL near a field. A balked landing with no touchdown still increments the count and the go-around branch reports it as a completed leg.
- **How to verify**: Decide the intended invariant; if "landings = touchdowns", gate the count on the speed threshold instead of the altitude band.

### [x] [I-003] IGC filename has minute resolution — same-minute saves silently overwrite
- **File**: `src/flightlog/FlightRecorder.cpp:148-159`
- **Category**: Error Handling — **Confidence**: 70/100
- **Finding**: `track_yyyyMMdd_HHmm.igc` with no existence check: two saves in the same UTC minute overwrite each other, and two flights then share one trackFile — deleting one flight's track destroys the other's.
- **Unverified because**: Whether two legs can complete within the same minute depends on detector timing; the touch-and-go path sets the new leg's startTime to the previous landing time with no minimum leg duration.
- **How to verify**: Simulate a sub-minute leg; or simply add a seconds field and a collision counter to the filename.

### [x] [I-004] Detector signals carry both a QDateTime and its pre-formatted "HH:mm" string
- **File**: `src/flightlog/FlightDetector.h:115-136`
- **Category**: API & C++ Correctness — **Confidence**: 70/100
- **Finding**: Presentation formatting is embedded in the detection-layer signal at four emit sites; every future subclass must replicate the exact format.
- **How to verify**: Confirm no receiver needs a different format, then drop `timeStr` and format once in FlightLog.

### [x] [I-005] Flight::operator== ignores coordinates and trackFile
- **File**: `src/flightlog/Flight.cpp:152-164`
- **Category**: API & C++ Correctness — **Confidence**: 68/100
- **Finding**: Equality covers 10 of 13 members; two entries differing only in attached track compare equal. As a `QML_VALUE_TYPE` this defines value equality in QML.
- **How to verify**: Grep for Flight comparisons (`contains`/`indexOf`, QML `===`); either include the fields or document the exclusion as intentional metadata-only equality.

### [x] [I-006] FlightRecorder::clearTrack() changes trackGeoPath without emitting trackGeoPathChanged
- **File**: `src/flightlog/FlightRecorder.cpp:116-123, 92-97`
- **Category**: Model Contracts — **Confidence**: 65/100
- **Finding**: `clearTrack()` and the TakeoffPhase reset mutate the track silently; FlightLog currently compensates externally, but any second consumer of the signal sees a stale trace.
- **How to verify**: Emit the signal from both sites and confirm no binding-loop regression.

### [x] [I-007] Case-sensitive callsign matching in lastArrivalICAO and CSV aircraft dedup
- **File**: `src/flightlog/FlightLog.cpp:314, 415`
- **Category**: API & C++ Correctness — **Confidence**: 65/100
- **Finding**: "d-kebe" vs "D-KEBE" fails the last-arrival lookup and produces duplicate Aircraft Table rows in the export.
- **How to verify**: Check whether the edit dialog lets users type the callsign per flight; if so, compare case-insensitively.

### [ ] [I-008] CSV formula injection not neutralized in the ForeFlight export
- **File**: `src/flightlog/FlightLog.cpp:387-392`
- **Category**: Error Handling — **Confidence**: 62/100
- **Finding**: `csvField()` quotes only comma/quote/CR/LF; free-text fields beginning with `=`, `+`, `-`, `@` execute as formulas when opened in Excel/LibreOffice.
- **Unverified because**: Data is authored by the device's own user; risk materializes only if flight data can arrive from external sources.
- **How to verify**: Confirm the threat model (planned import/sync); if fields can be foreign-sourced, prefix leading formula characters.

### [x] [I-009] trackFromIGC(): unchecked toInt() and altitude 0 conflated with "unknown"
- **File**: `src/flightlog/FlightRecorder.cpp:289-364`
- **Category**: Error Handling — **Confidence**: 62/100
- **Finding**: B-record fields parse without ok-flags — garbage becomes a valid-looking (0°, 0°) point that passes the validity gate; hemisphere chars aren't validated; `gpsAlt==0`/`pressAlt==0` (legitimate sea-level fixes) are treated as missing.
- **Unverified because**: The parser currently only reads files this app wrote; impact depends on a future external-IGC import feature.
- **How to verify**: Check whether foreign IGC import is planned; feed a corrupted B-record and watch for the bogus (0,0) point.

### [x] [I-010] Go-around path emits signals mid-transition — latent reentrancy hazard
- **File**: `src/flightlog/AirplaneFlightDetector.cpp:179`
- **Category**: Thread Safety — **Confidence**: 60/100
- **Finding**: The touch-and-go branch emits `detectionStateChanged`/`landingDetected` while half-transitioned, then continues mutating state — breaking the "reset before emitting" convention the other paths follow. A synchronous receiver calling back into `resetDetection()`/`endFlight()` would have its reset clobbered. No current receiver re-enters; everything is main-thread.
- **How to verify**: If a handler for the takeoff/landing signals is ever added that calls back into the detector synchronously, step through lines 161-193.

---

## Summary

| Category | Lint | Deep | Investigate | Total |
|----------|------|------|-------------|-------|
| Model Contracts | 0 | 4 | 1 | 5 |
| Ownership & Lifecycle | 0 | 5 | 0 | 5 |
| Thread Safety | 0 | 0 | 1 | 1 |
| API & C++ Correctness | 2 | 6 | 4 | 12 |
| Error Handling | 0 | 4 | 3 | 7 |
| Performance & Quality | 0 | 6 | 1 | 7 |
| **Total** | **2** | **25** | **10** | **37** |

Findings below confidence 60 were suppressed. Thread safety came back clean —
the module is correctly single-threaded throughout (verified against the JNI
entry points, `QtConcurrent` usage in GeoMapProvider, and the position-update
path).
