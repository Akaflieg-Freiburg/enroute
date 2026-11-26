# QML Code Review Report — FlightLogEntryEditor.qml

Date: 2026-11-17
Scope: [src/qml/dialogs/FlightLogEntryEditor.qml](src/qml/dialogs/FlightLogEntryEditor.qml) (404 lines) at commit `8d1d75e22` on `feature/cleanup`. All line numbers refer to that commit.
Method: deterministic lint, qmllint (Qt 6.12, project import paths), and six parallel deep-analysis passes (bindings, layout, lifecycle and C++ boundary, delegates, structure, performance). Findings were deduplicated and checked against the source.
Verification: by reading the app sources and the Qt 6.12 sources. Nothing was built or run, and the app was not launched. Where a finding has a "To reproduce" line, that recipe has not been tried yet.

Issues found: 19 (5 lint groups, 12 deep analysis, 2 investigation targets); 11 resolved and removed since, 8 listed below (4 lint groups, 3 deep analysis, 1 investigation target), of which I-002, D-007, D-008 and D-009 are marked resolved
qmllint: no warnings on this file

**TL;DR** — The dialog is structurally sound: no binding loops, correct types at the C++ boundary, correct UTC and midnight handling. Two findings that could lose flight data have been fixed and removed from this report:

1. D-001: adding several similar flights in a row could save the new flight under the previous flight's UUID and overwrite it.
2. D-002: pressing OK after a landing was detected while the editor was open erased the detected landing. The flight list no longer offers editing while a flight is being recorded.

The remaining findings are cleanup.

---

## Context: how the dialog is used

[FlightLogPage.qml](src/qml/pages/FlightLogPage.qml) holds two static instances of the dialog, both reused for every flight:

- **Edit** (`flightEditor`, line 833). The edit button of a list entry sets `editFlight = modelData`, then calls `open()` (lines 567-572). On accept, the page calls `FlightLog.updateFlight()` with the result of `flightEditor.resultFlight()` and its UUID.
- **Add** (`addFlightEditor`, line 844). The "Add Flight" button builds a template with `FlightLog.createFlight(lastArrival, nearestAirfield, …, aircraftName, "")`, assigns it to `editFlight` and calls `open()` (line 737). On accept, the page calls `FlightLog.addFlight(addFlightEditor.resultFlight())`.

Inside the dialog, `onAboutToShow` copies `editFlight` into the fields once. `resultFlight()` builds a flight from the field texts with `FlightLog.createFlight()` and returns `FlightLog.applyEdits(editFlight, thatFlight)`. `applyEdits()` starts from its first argument, so the result carries the UUID, track file and coordinates of `editFlight`.

---

## Decisions to make

These findings need a design decision before they can be fixed:

| Finding | Question |
|---------|----------|
| S-001 | Once the flight list is an item model: does `narrowSelection()` get the visible flights from the proxy model, or does the selection move into C++? |

Fix status: [ ] = open. Mark items as you resolve them.

---

## Deep analysis findings

### [x] [D-007] Public API that nothing uses
- **Status**: Resolved on 2026-11-26. `flightUuid`, the ten `new*` properties and the default title are gone; `resultFlight()` reads the fields directly, and the header comment describes the contract (`editFlight` in, `resultFlight()` out). The Edit path takes the UUID from the result: `FlightLog.updateFlight(edited.uuidString, edited)`. This is safe since D-001 was fixed by comparing the UUID in `Flight::operator==`: assigning another flight to `editFlight` now always replaces it. Not done: a private object for the helpers (`codeOrName`, `utcTime`, `followTakeoff`, `preview`, `errors`, `warnings`, `autoDate`). QML has no private members, and the prefix would touch about twenty lines for little gain. The toast "Flight updated" after a failed database write remains; that needs `updateFlight()` to report failure. Checked by build and qmllint; not run in the app.
- **File**: [FlightLogEntryEditor.qml:38-51](src/qml/dialogs/FlightLogEntryEditor.qml#L38-L51), also lines 30, 68, 75-104
- **Category**: States & Structure — **Confidence**: 95/100
- **Finding**:
  - The ten `new*` properties (lines 42-51) carry the comment "Output values read by the caller", but no caller reads them; they only feed `resultFlight()`. The set is also incomplete: the landing count is read straight from the field (line 123).
  - `codeOrName`, `isValidOrEmptyTime`, `formatTimeInput`, `utcTime` and `utcDate` are public functions but purely internal.
  - The dialog never reads `flightUuid`. The page parks the UUID there for the Edit instance; the Add instance never sets it, although the header comment (line 30) tells every user to. If it is missing or wrong, `updateFlight()` returns silently while the page still shows "Flight updated".
  - The default `title` (line 68) is overridden by both instances and only gives translators a second "Edit Flight" string.
- **Trace**: A search over `src/` finds no reader of the `new*` properties or the helpers outside this file. Callers: [FlightLogPage.qml:569-571](src/qml/pages/FlightLogPage.qml#L569-L571), [:737-738](src/qml/pages/FlightLogPage.qml#L737-L738), [:836-852](src/qml/pages/FlightLogPage.qml#L836-L852).
- **Mitigation**: Make `resultFlight()` the only output and move the rest into a private object. Fix the comments at lines 30 and 41 and remove the default title. Remove `flightUuid` only after D-001 is fixed: today it is what keeps the Edit path from the same UUID mix-up.

### [x] [D-008] Summary label has a dependency it does not capture
- **Status**: Resolved on 2026-11-26. A `Connections` handler on `GeoMapProvider.waypointsChanged` re-creates the binding of `preview` with `Qt.binding()`, which evaluates it again; `preview` is no longer `readonly`. Reproduced and verified end to end on the desktop (Qt 6.12.1, offscreen, copy of a real data directory, QML test driver loaded at start-up): the editor opened 22 ms after start with EDTF→EDFE, before the maps had loaded, and the maps arrived after about 0.6 s. Without the fix the distance stayed missing for 20 s, until a simulated keystroke; with the fix it appeared on the signal (120.7 NM). After two more emissions of the signal the binding still followed its inputs.
- **File**: [FlightLogEntryEditor.qml:368-383](src/qml/dialogs/FlightLogEntryEditor.qml#L368-L383)
- **Category**: Bindings & Properties — **Confidence**: 85/100
- **Finding**: The "Distance" part of the summary comes from an airfield lookup inside `FlightLog.applyEdits()`. The binding does not depend on the map data, so if maps finish loading or are updated while the dialog is open, the label keeps the old value until the next keystroke. The saved flight is correct, because `resultFlight()` runs again on accept. This is the situation the CLAUDE.md rule "bindings must consume every dependency" describes. All other dependencies are consumed properly.
- **Trace**: `applyEdits()` → `resolveCoordinates()` ([FlightLog.cpp:817-842](src/flightlog/FlightLog.cpp#L817-L842)) → `GeoMapProvider::findByID()`.
- **Mitigation**: Refresh the summary from a `Connections` handler on `GeoMapProvider.waypointsChanged` with an imperative assignment. Do not read the waypoint list inside the binding; that would convert the whole list to JavaScript.

### [x] [D-009] Duplicated logic and dead guards
- **Status**: Resolved on 2026-11-26. The date/time redesign had brought one `TimeField`, removed the dead `getTime &&` guards in the editor and replaced `utcDate()`. The rest:
  - Code normalisation moved into `Flight::setDepartureICAO()` / `setArrivalICAO()`, via the new `Flight::normalizedAirfield()` (trimmed; up to 4 characters upper case). It applies to every path: editor, "Add Flight" template, flight detection, JSON import and loading from the database. `codeOrName()` is gone.
  - Duration / Block / Distance come from the new `Flight::calculatedValues(aircraft)`, translated in the context `Flightlog::Flight`; the editor summary and the list delegate both use it. In the list, "Landings" now follows the distance. The existing translations of the list were carried over to the new context in all six `.ts` files (subrepo, uncommitted).
  - The two dead `getTime` guards in the list delegate are gone as well.
  - Verified on the desktop (Qt 6.12.1, offscreen, German, copy of a real data directory, QML test driver): `createFlight(" edtf ")` gives "EDTF", names longer than 4 characters are kept; a JSON import with "edtf"→"edfe" gives upper-case codes and a distance of 121 NM. Control run without the setter change: the import kept "edtf"/"edfe" and had no distance. Screenshots of the list and the editor show the German values.
- **File**: [FlightLogEntryEditor.qml:128-152](src/qml/dialogs/FlightLogEntryEditor.qml#L128-L152), also lines 94-104, 132, 250-300, 368-383
- **Category**: Performance & Quality — **Confidence**: 90/100
- **Finding**:
  - The four load blocks in `onAboutToShow` repeat `x.getTime && !isNaN(x.getTime())`. The first half is dead: a `QDateTime` always arrives as a Date object (an invalid one as an Invalid Date), so `!isNaN(x.getTime())` alone is enough.
  - The four time fields (lines 250-300) differ only in id and placeholder. The copies have already diverged, see the colour predicate in D-006.
  - `utcDate()` and line 132 re-implement `isoDate()` and `todayIsoDate()` of [DateInput.qml:69-81](src/qml/items/DateInput.qml#L69-L81).
  - The summary repeats the Duration / Block / Distance formatting of the list delegate in [FlightLogPage.qml](src/qml/pages/FlightLogPage.qml), including three `qsTr` strings in a second translation context.
  - The rule "codes are stored in upper case, so that they are found in the maps" exists only in `codeOrName()`. C++ stores what it is given, so a lower-case code from another path (JSON import) stays unresolved until the entry is saved from this dialog.
- **Mitigation**: One reusable time field with a "required" flag that drives colour and validity. One helper that returns "" for an invalid time. `dateField.todayIsoDate()` for the default date. Code normalisation in `createFlight()` / `applyEdits()`.

---

## Investigation targets (human verification needed)

### [x] [I-002] Double-click on OK may add the flight twice
- **Status**: Resolved by the duplicate check in `FlightLog::addFlight()`, see D-001: a second call with the same UUID is ignored. The toast "Flight added" can still appear twice. Tested end to end on 2026-11-21 by calling `accept()` twice on the open editor: before the fix the list showed the flight twice, with the fix once.
- **File**: [FlightLogEntryEditor.qml:396-401](src/qml/dialogs/FlightLogEntryEditor.qml#L396-L401)
- **Category**: Component Loading & Lifecycle — **Confidence**: 65/100
- **Finding**: `Dialog.done()` emits `accepted()` and then starts the exit transition (220 ms in the Material style), during which the popup stays visible and enabled. A second click on OK would call `addFlight()` again with the same UUID: one database row, but two list entries until restart. In the Edit path it would be harmless.
- **Unverified because**: Whether the second click still lands on the shrinking button depends on timing and pointer position.
- **How to verify**: In a `FLIGHTLOG=ON` build, fill "Add Flight" and double-click OK quickly; look for two toasts or two rows.
- **Possible mitigation**: Also bind the OK button's `enabled` to the dialog's `opened`. The duplicate check in `addFlight()` proposed for D-001 covers this as well.

---

## Lint findings

The linter reported 51 raw hits in five groups. The `var` group (L-002) has been resolved and removed. None of the remaining groups needs action beyond style.

### [ ] [L-001] Root id is not `root`
- **File**: [FlightLogEntryEditor.qml:35](src/qml/dialogs/FlightLogEntryEditor.qml#L35)
- **Rule**: STY-1
- **Finding**: The root id is `flightEditorDialog`.
- **Mitigation**: None needed. Descriptive root ids are the project's own convention (`centeringDialog`, `dateInput`, `textField`).

### [ ] [L-003] Imperative assignment to field text
- **File**: lines 130-159 (14 hits)
- **Rule**: BND-2
- **Finding**: `onAboutToShow` assigns the field texts.
- **Mitigation**: None. The fields have no bindings to destroy; filling them on open is the design.

### [ ] [L-004] `clip: true`
- **File**: line 166
- **Rule**: PRF-3
- **Finding**: Clipping on the scroll view.
- **Mitigation**: None. A scroll area inside a dialog needs it.

### [ ] [L-005] Attached `Layout.*` properties before plain properties
- **File**: lines 203-400 (24 hits)
- **Rule**: ORD-1
- **Finding**: `Layout.*` lines precede `text`, `placeholderText` and similar.
- **Mitigation**: Style only. Reorder if the file should follow the Qt ordering convention.

**False positives**: the linter reported loose equality (JS-2) on lines 178, 372 and 375. Those lines use `===` / `!==`; the file contains no loose comparison.

---

## Checked and found sound

- **Binding loops**: none. The handlers that write (`onAboutToShow`, `onTextEdited`, `onHeightChanged`) do not feed back.
- **C++ boundary**: `createFlight()` gets ten strings, `applyEdits()` two `flight` values, the validators strings. The write `edited.landingCount = …` on line 123 is effective: a value type returned from an invokable is a copy of its own, and `landingCount` is writable.
- **`applyEdits()` against the header comment**: they agree. `updateFlight()` applies it a second time, which is idempotent as long as the stored entry has not changed. Only the flight detector changes an entry while the editor is open, and only the entry of the flight being recorded. The flight list therefore hides the edit button unless `FlightLog.detectionState` is `Idle`.
- **UTC and midnight**: `getUTC*` works on the epoch value. `createFlight()` places the other times around the start time, so overnight flights survive a round trip, seconds included.
- **Input-method pre-edit text**: it is committed when the OK button takes focus, so testing `displayText` and saving `text` does not lose input.
- **Layout**: only `anchors.fill: parent`; the implicit-height chain from grid to scroll view to dialog is loop-free; the empty separator rows and `Qt.AlignBaseline` behave as the comments say (DateInput is a TextField and has a baseline); the scroll handler uses the right coordinate system and clamp.
- **Performance**: one evaluation of the summary binding costs a few regular-expression matches and at most two hash lookups; no map scan, SQL or file I/O. It runs once per keystroke, which is fine.
- **`codeOrName()` and its `length <= 4` rule**: checked against the installed maps for Germany, France and Switzerland. All airfield codes have four characters and all short airfield names are already upper case, so the function does not alter stored names.
- **Not applicable**: the file has no Loader, Image, Connections, ListView, delegate, inline component, state or transition, and uses no deprecated Qt 5 API.

---

## Summary

| Category | Lint | Deep | Investigate | Total |
|----------|------|------|-------------|-------|
| Bindings & Properties | 1 | 1 | 0 | 2 |
| Layout & Anchoring | 0 | 0 | 0 | 0 |
| Component Loading & Lifecycle | 0 | 0 | 1 | 1 |
| ListView & Delegates | 0 | 0 | 0 | 0 |
| States & Structure | 2 | 1 | 0 | 3 |
| Performance & Quality | 1 | 1 | 0 | 2 |
| **Total** | **4** | **3** | **1** | **8** |

Findings below confidence 60 are suppressed.

---

## Side notes outside the file

### [ ] [S-001] FlightLogPage.qml: feed the flight list from an item model
- **Status**: Open. Recommended as a change of its own, before `FLIGHTLOG` is enabled by default. Noted on 2026-11-21, after commit `b1b95be49` made `FlightLogPage.qml` clean under qmllint and removed the lint exclusion in `src/CMakeLists.txt`. Line numbers in this note refer to that commit, not to the commit named at the top.
- **File**: [FlightLogPage.qml:64](src/qml/pages/FlightLogPage.qml#L64) (`visibleFlights`), [:401](src/qml/pages/FlightLogPage.qml#L401) (`Binding on model`), [:434](src/qml/pages/FlightLogPage.qml#L434) (delegate), [FlightLog.h:94](src/flightlog/FlightLog.h#L94) (`flights`)
- **Finding**: The model of the flight list is a JavaScript array. `visibleFlights()` copies `FlightLog.flights` into an array and filters it with `Librarian.matches`; a delayed `Binding` assigns the result to the list view. [Librarian.h:255](src/Librarian.h#L255) names this as the pattern for ad-hoc lists. The flight log is library data: it is stored, the user edits it, and it grows over the years. For such data, CLAUDE.md asks for a list that binds once to an item model, and for delegates with a typed role (`required property flight flight`). The delegate has been typed since 2026-11-21, but its role is still called `modelData`, because an array offers no other role name.
- **Consequences**:
  - Every `flightsChanged` gives the list view a new model. This should rebuild all delegates and move the list back to the top after each edit, removal or detected landing. That is what `ListView` does with a new model; it has not been observed in the app.
  - Every keystroke in the filter field calls `Librarian.matches` from JavaScript, once per flight.
- **Scope**: `FlightLog.flights` is read in one place, `visibleFlights()`. Nine functions in [FlightLog.cpp](src/flightlog/FlightLog.cpp) change the list: `addFlight`, `removeFlight`, `removeFlights`, `clearFlights`, `updateFlight`, `importFromJSON`, `removeTrack`, `load` and `onLandingDetected`. Each of them copies the list, changes the copy and stores it with `m_flights.setValue()`.
- **Mitigation**:
  1. Add a `QAbstractListModel` that `FlightLog` owns and exposes as `FlightLog.flightsModel`, as `Librarian` does with `aircraftModel` ([Librarian.h:50](src/Librarian.h#L50)). `FlightLog` cannot be the model itself, because it derives from `GlobalObject`.
  2. Give the model two roles: `flight`, and `name` with the text that the filter searches (departure, arrival, pilot name, callsign). `NameFilterProxyModel` filters on the role `name` by default and works unchanged.
  3. In the page, set `model: NameFilterProxyModel { sourceModel: FlightLog.flightsModel; filter: flightFilter.filter }` and declare `required property flight flight` in the delegate. `visibleFlights()` and the `Binding` go away.
  4. Emit row-granular signals from the nine functions. `addFlight`, `updateFlight` and `importFromJSON` sort the list after the change, so the row of a flight is known only after sorting. `load` and `clearFlights` replace the whole list and are resets.
- **Decision needed**: `narrowSelection()` ([FlightLogPage.qml:76](src/qml/pages/FlightLogPage.qml#L76)) deselects the flights that the filter hides, and takes the visible UUIDs from `visibleFlights()`. It needs another source. Either the proxy model reports which UUIDs are visible, or the selection moves into the C++ model.
- **Why a change of its own**: It changes the data path that the flight detector writes to during a flight. It needs its own review and a test in the app.
- **To verify**: In a `FLIGHTLOG=ON` build with more flights than fit on the screen, scroll down, then edit one flight and remove another: the list must stay where it is. Select several flights, then type in the filter field: the count in the header must equal the number of selected flights that are still visible.
