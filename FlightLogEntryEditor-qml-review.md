# QML Code Review Report — FlightLogEntryEditor.qml

Date: 2026-11-17
Scope: [src/qml/dialogs/FlightLogEntryEditor.qml](src/qml/dialogs/FlightLogEntryEditor.qml) (404 lines) at commit `8d1d75e22` on `feature/cleanup`. All line numbers refer to that commit.
Method: deterministic lint, qmllint (Qt 6.12, project import paths), and six parallel deep-analysis passes (bindings, layout, lifecycle and C++ boundary, delegates, structure, performance). Findings were deduplicated and checked against the source.
Verification: by reading the app sources and the Qt 6.12 sources. Nothing was built or run, and the app was not launched. Where a finding has a "To reproduce" line, that recipe has not been tried yet.

Issues found: 19 (5 lint groups, 12 deep analysis, 2 investigation targets); 8 resolved and removed since, 11 listed below (4 lint groups, 6 deep analysis, 1 investigation target), of which D-001 and I-002 are marked resolved
qmllint: no warnings on this file

**TL;DR** — The dialog is structurally sound: no binding loops, correct types at the C++ boundary, correct UTC and midnight handling. There are two ways it can lose flight data:

1. Adding several similar flights in a row can save the new flight under the previous flight's UUID and overwrite it (D-001).
2. Pressing OK after a landing was detected while the editor was open erases the detected landing (D-002).

The remaining findings are a usability issue (required/optional marking) and cleanup.

**If you only fix two things**: D-001, D-002. D-001 comes first, because it loses flights in ordinary use (back-filling a log, or several local flights in a row).

---

## Context: how the dialog is used

[FlightLogPage.qml](src/qml/pages/FlightLogPage.qml) holds two static instances of the dialog, both reused for every flight:

- **Edit** (`flightEditor`, line 833). The edit button of a list entry sets `flightUuid` and `editFlight = modelData`, then calls `open()` (lines 567-572). On accept, the page calls `FlightLog.updateFlight(flightEditor.flightUuid, flightEditor.resultFlight())`.
- **Add** (`addFlightEditor`, line 844). The "Add Flight" button builds a template with `FlightLog.createFlight(lastArrival, nearestAirfield, …, aircraftName, "")`, assigns it to `editFlight` and calls `open()` (line 737). On accept, the page calls `FlightLog.addFlight(addFlightEditor.resultFlight())`.

Inside the dialog, `onAboutToShow` copies `editFlight` into the fields once. `resultFlight()` builds a flight from the field texts with `FlightLog.createFlight()` and returns `FlightLog.applyEdits(editFlight, thatFlight)`. `applyEdits()` starts from its first argument, so the result carries the UUID, track file and coordinates of `editFlight`.

---

## Decisions to make

These findings need a design decision before they can be fixed:

| Finding | Question |
|---------|----------|
| D-001 | Fix in `Flight::operator==`, in `FlightLog::addFlight()`, in the dialog, or in more than one place? |
| D-002 | Merge only the fields the user changed, follow the live entry while the dialog is open, or do not offer editing for the flight being recorded? |
| D-006 | How are optional fields marked: which fields, and with which wording? (The departure is optional now.) |
| D-007 | `flightUuid` can go, but only after D-001 is fixed (it protects the edit path today). |
| Side note | Lift the qmllint exclusion for this file in [src/CMakeLists.txt:630](src/CMakeLists.txt#L630)? |

Fix status: [ ] = open. Mark items as you resolve them.

---

## Deep analysis findings

### [x] [D-001] Consecutive "Add Flight" can reuse the previous flight's UUID and overwrite it
- **Status**: Resolved: `Flight::operator==` compares the UUID, so QML stores every new template, and `FlightLog::addFlight()` ignores a flight whose UUID is already in the log. Tested end to end on 2026-11-21 in the app (offscreen, fresh data directory, position fix at EDTF), by clicking "Add Flight" and accepting the editor for four flights. Before the fix, the first three flights were saved under one UUID and only the last of them was left after a restart. With the fix, all four flights have their own UUID and are in the database after a restart.
- **File**: [FlightLogEntryEditor.qml:124](src/qml/dialogs/FlightLogEntryEditor.qml#L124) (`resultFlight`), [:39](src/qml/dialogs/FlightLogEntryEditor.qml#L39) (`property flight editFlight`)
- **Category**: Component Loading & Lifecycle — **Confidence**: 85/100
- **Finding**: The flight saved by the Add path takes its identity from `editFlight`. Under the conditions below, `editFlight` still holds the template of the previous "Add Flight", so the new flight is saved under the previous flight's UUID. The database row of the previous flight is overwritten. Both flights show in the list until the app restarts; after that the earlier one is gone.
- **Mechanism**:
  1. Each "Add Flight" builds a new template with a new UUID and assigns it to `addFlightEditor.editFlight` ([FlightLogPage.qml:737](src/qml/pages/FlightLogPage.qml#L737)).
  2. QML does not store an assignment to a value-type property when the new value compares equal to the stored one (Qt 6.12, `qtdeclarative/src/qml/qml/qqmlvmemetaobject.cpp`, around line 1080: the value is replaced only if `!propType.equals(...)`).
  3. Equality is `Flight::operator==`, which compares every member except the UUID ([Flight.cpp:71-86](src/flightlog/Flight.cpp#L71-L86)). A template with the same content as the previous one is therefore dropped, and the dialog keeps the previous template and its UUID.
  4. `applyEdits()` starts from the original and keeps its UUID ([FlightLog.cpp:486](src/flightlog/FlightLog.cpp#L486)).
  5. `addFlight()` prepends to the list without a duplicate check ([FlightLog.cpp:244-257](src/flightlog/FlightLog.cpp#L244-L257)), and the storage upserts on `uuid` ([FlightLogStorage.cpp:102](src/flightlog/FlightLogStorage.cpp#L102)).
- **When it triggers**: The template consists of last arrival, nearest airfield and aircraft name. It repeats when the same aircraft is selected, the device has not moved, and the flight added before did not change the last arrival. That is the case when the previous flight
  - arrived where the one before it arrived (local flights, A→A),
  - is older than the newest flight in the log (back-filling), or
  - has an empty arrival.

  On an empty log, the third such flight is the first to be affected.
- **The Edit path is safe today**: `updateFlight()` looks the entry up by `flightUuid` and re-applies the edits onto the stored entry ([FlightLog.cpp:419-430](src/flightlog/FlightLog.cpp#L419-L430)), so the identity of `editFlight` does not matter there.
- **Mitigation**: Include the UUID in `Flight::operator==`, so that a flight with another identity is a different value to QML. In addition, have `addFlight()` reject or re-key a UUID that is already in the log. Alternatively, do not let the Add path take its identity from `editFlight` at all.
- **To reproduce**: In a `FLIGHTLOG=ON` build with at least one earlier flight arriving at EDTF, add EDTF→EDTF 10:00-11:00, then add EDTF→EDTF 14:00-15:00 for the same aircraft. Restart the app and count the entries.

### [ ] [D-002] A landing detected while the editor is open is erased on OK
- **File**: [FlightLogEntryEditor.qml:109-125](src/qml/dialogs/FlightLogEntryEditor.qml#L109-L125) (`resultFlight`), [:127-160](src/qml/dialogs/FlightLogEntryEditor.qml#L127-L160) (`onAboutToShow`)
- **Category**: Component Loading & Lifecycle — **Confidence**: 85/100
- **Finding**: The dialog copies `editFlight` into its fields once and hands back every editable field on accept, touched or not. If the flight being recorded is open in the editor when the detector completes it, pressing OK writes the stale fields over the detected values:
  - the arrival code becomes empty,
  - the landing time becomes invalid,
  - the landing count goes back to 1,
  - the GPS-derived arrival coordinate is discarded.

  Nothing repairs the entry afterwards, because the detector has already finished with it. The track file survives.
- **Trace**:
  - The in-progress flight is in the list and has an edit button ([FlightLogPage.qml:562-572](src/qml/pages/FlightLogPage.qml#L562-L572)).
  - `onLandingDetected` writes arrival, coordinate, landing time and landing count into the stored entry ([FlightLog.cpp:950-992](src/flightlog/FlightLog.cpp#L950-L992)). A modal dialog does not stop this.
  - On OK, `updateFlight()` applies the dialog's result onto the stored entry ([FlightLog.cpp:419-438](src/flightlog/FlightLog.cpp#L419-L438)). In `applyEdits()` the empty arrival differs from the detected one, so the coordinate is cleared; `keepSeconds()` returns the invalid edited time; the landing count is overwritten ([FlightLog.cpp:471-510](src/flightlog/FlightLog.cpp#L471-L510)).
  - OK stays enabled, because an empty landing time and arrival are acceptable input (lines 56-62).
- **When it triggers**: The landing is confirmed when ground speed drops below the minimum speed, or 60 s after entering the landing phase ([AirplaneFlightDetector.cpp:174-196](src/flightlog/AirplaneFlightDetector.cpp#L174-L196)). The editor has to be open across that moment, for example when the pilot opens it during the landing roll to add a comment.
- **Mitigation**: One of these:
  - Apply only what the user changed: pass the snapshot along and take a field from the edit only where it differs from the snapshot (a three-way merge).
  - Follow the live entry: while open, react to `FlightLog.flightsChanged`, refresh the fields the user has not touched, or close with a notice.
  - Do not offer editing for the flight that is being recorded.
- **To reproduce**: With flight detection enabled and a simulated position source, open the editor on the in-progress entry, trigger a landing, and press OK without touching Landing or Arrival.

### [ ] [D-006] Required and optional fields are not communicated consistently
- **Status**: Partly addressed in the date/time redesign: one translatable placeholder for all four times, and a status line that lists errors (which block saving) or else warnings, separated by " • ". Arrival, Aircraft and the remaining "(optional)" placeholders are unchanged.
- **File**: [FlightLogEntryEditor.qml:254-296](src/qml/dialogs/FlightLogEntryEditor.qml#L254-L296) (time fields), also lines 222, 236, 316, 327, 338
- **Category**: States & Structure — **Confidence**: 92/100
- **Finding**: `acceptableInput` requires Departure, Date, Start, and a valid-or-empty Landings count; everything else may be empty.
  - "(optional)" marks Off-Block, On-Block, Pilot and Comments, but not Arrival, Landing or Aircraft, which are optional too.
  - Mandatory Start and optional Landing show the same placeholder "HH:mm".
  - The Start field's colour uses `isValidOrEmptyTime` (line 270), while OK uses `isValidTime` (line 58). An empty mandatory field therefore never looks wrong, and "Add Flight" can open with OK disabled and no field marked.
  - `"HH:mm"` is translated in Off-Block and On-Block (`qsTr("HH:mm (optional)")`) and hard-coded in Start and Landing. Polish already shows "GG:mm (opcja)" next to "HH:mm".
- **Trace**: Placeholders compared against lines 56-62. Polish: `3rdParty/enrouteText/assets/enroute_pl.ts`, line 2582.
- **Mitigation**: Pick one convention (mark every optional field, or mark the required ones) with one wording. Mark empty mandatory fields the same way as invalid ones. Treat the time format the same way in all four fields: either translatable everywhere or nowhere.

### [ ] [D-007] Public API that nothing uses
- **File**: [FlightLogEntryEditor.qml:38-51](src/qml/dialogs/FlightLogEntryEditor.qml#L38-L51), also lines 30, 68, 75-104
- **Category**: States & Structure — **Confidence**: 95/100
- **Finding**:
  - The ten `new*` properties (lines 42-51) carry the comment "Output values read by the caller", but no caller reads them; they only feed `resultFlight()`. The set is also incomplete: the landing count is read straight from the field (line 123).
  - `codeOrName`, `isValidOrEmptyTime`, `formatTimeInput`, `utcTime` and `utcDate` are public functions but purely internal.
  - The dialog never reads `flightUuid`. The page parks the UUID there for the Edit instance; the Add instance never sets it, although the header comment (line 30) tells every user to. If it is missing or wrong, `updateFlight()` returns silently while the page still shows "Flight updated".
  - The default `title` (line 68) is overridden by both instances and only gives translators a second "Edit Flight" string.
- **Trace**: A search over `src/` finds no reader of the `new*` properties or the helpers outside this file. Callers: [FlightLogPage.qml:569-571](src/qml/pages/FlightLogPage.qml#L569-L571), [:737-738](src/qml/pages/FlightLogPage.qml#L737-L738), [:836-852](src/qml/pages/FlightLogPage.qml#L836-L852).
- **Mitigation**: Make `resultFlight()` the only output and move the rest into a private object. Fix the comments at lines 30 and 41 and remove the default title. Remove `flightUuid` only after D-001 is fixed: today it is what keeps the Edit path from the same UUID mix-up.

### [ ] [D-008] Summary label has a dependency it does not capture
- **File**: [FlightLogEntryEditor.qml:368-383](src/qml/dialogs/FlightLogEntryEditor.qml#L368-L383)
- **Category**: Bindings & Properties — **Confidence**: 85/100
- **Finding**: The "Distance" part of the summary comes from an airfield lookup inside `FlightLog.applyEdits()`. The binding does not depend on the map data, so if maps finish loading or are updated while the dialog is open, the label keeps the old value until the next keystroke. The saved flight is correct, because `resultFlight()` runs again on accept. This is the situation the CLAUDE.md rule "bindings must consume every dependency" describes. All other dependencies are consumed properly.
- **Trace**: `applyEdits()` → `resolveCoordinates()` ([FlightLog.cpp:817-842](src/flightlog/FlightLog.cpp#L817-L842)) → `GeoMapProvider::findByID()`.
- **Mitigation**: Refresh the summary from a `Connections` handler on `GeoMapProvider.waypointsChanged` with an imperative assignment. Do not read the waypoint list inside the binding; that would convert the whole list to JavaScript.

### [ ] [D-009] Duplicated logic and dead guards
- **Status**: Partly addressed in the date/time redesign: one `TimeField` component replaces the four copies, the dead `getTime &&` guards are gone in the editor, and `utcDate()` is replaced by `DateInput.isoDate()` / `todayIsoDate()`. The summary still duplicates the list delegate, and code normalisation still lives in `codeOrName()`.
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
- **`applyEdits()` against the header comment**: they agree. `updateFlight()` applies it a second time, which is idempotent as long as the stored entry has not changed (see D-002).
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
| Component Loading & Lifecycle | 0 | 2 | 1 | 3 |
| ListView & Delegates | 0 | 0 | 0 | 0 |
| States & Structure | 2 | 2 | 0 | 4 |
| Performance & Quality | 1 | 1 | 0 | 2 |
| **Total** | **4** | **6** | **1** | **11** |

Findings below confidence 60 are suppressed.

---

## Side notes outside the file

- **qmllint exclusion**: [src/CMakeLists.txt:627-630](src/CMakeLists.txt#L627-L630) sets `QT_QML_SKIP_QMLLINT` for this file and for `FlightLogPage.qml` "until they are clean". This file is clean when qmllint is run on it directly, so the exclusion can be lifted for it. `FlightLogPage.qml` still produces about 166 messages and has to stay excluded.
- **FlightLogPage.qml**: its list delegates use `modelData` (for example line 569), which goes against the CLAUDE.md convention of typed `required property` roles. The page was not reviewed here.
