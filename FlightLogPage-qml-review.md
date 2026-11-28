# Review: FlightLogPage.qml

Reviewed file: [src/qml/pages/FlightLogPage.qml](src/qml/pages/FlightLogPage.qml), branch `feature/cleanup` at `9c69743d1`, 2026-11-27.

## Summary

The page is clean at the type level: the project's `all_qmllint` target reports no warnings for it, and every call into C++ matches its signature. The problems are behavioural. Four of them affect selection mode and swipe-to-delete and are worth fixing before the flight log is enabled in default builds (F-01 to F-04).

| ID | Finding | Severity | Verified by | Status |
|----|---------|----------|-------------|--------|
| F-01 | Long press selects the flight, release deselects it again | High | Reproduced in an offscreen test | **Done** |
| F-02 | Empty selection in selection mode exports the whole log | High | Code read | Done |
| F-03 | Row stays swiped open after "No" in the remove dialog | High | Code read, sibling pages | Done |
| F-04 | "Save as…" leaves selection mode before the save has a result | Medium | Code read | Done |
| F-05 | System Back pops the page in selection mode | Medium | Code read | Done |
| F-06 | Success toasts are unconditional although the operations can fail | Medium | Analysis trace | Done |
| F-07 | "Show live flight trace" shows off while the map still draws the trace | Medium | Code read | Open |
| F-08 | Detection is off by default, but the page behaves as if it were on | Medium | Code read | Open |
| F-09 | Hard-coded colours ignore the theme | Medium | Code read | Open |
| F-10 | Counted strings without plural handling | Low | Analysis trace | Open |
| F-11 | Long comments break mid-word; time line has loose ends | Low | Code read | Open |
| F-12 | Ten copy-pasted export handlers that have drifted | Low | Partly code read | Open |
| F-13 | ScrollIndicator override | Low | Code read | Open |
| F-14 | Row has no `clicked()` contract and no primary action | Low | Code read | Open |
| F-15 | Dead code | Low | Analysis trace | Open |
| F-16 | Missing `readonly` on platform flags | Low | Analysis trace | Open |

"Code read" means I checked the decisive lines myself. "Analysis trace" means the finding rests on a traced reading of the project and Qt 6.12.1 sources that I did not repeat. The app itself was not launched for this review.

Four further points need a run to settle and are listed under [To verify](#to-verify). Points that were checked and found correct are under [Checked and cleared](#checked-and-cleared).

---

## Findings

### F-01 Long press selects the flight, release deselects it again — done

- **Where**: [FlightLogPage.qml:550-566](src/qml/pages/FlightLogPage.qml#L550-L566)
- **What happens**: Long-pressing a row enters selection mode with nothing selected. The header reads "Select flights" and the phone vibrates twice. Inside selection mode a long press does nothing.
- **Why**: The row has both `onClicked` (toggle when in selection mode) and a child `TapHandler.onLongPressed` (enter selection mode, then toggle). The `TapHandler` only takes a passive grab, and a `SwipeDelegate` suppresses `clicked` after a hold only when its own `pressAndHold` signal is connected. So the long press toggles the flight on, and the click on release toggles it off.
- **Evidence**: A standalone Qt Quick Test mirroring the delegate (see [Appendix](#appendix-reproducing-f-01)) ends with an empty selection after press, 1.1 s wait and release, for mouse and for touch.
- **Status**: Done. Fixed on 2026-11-27.
- **Fix**: The `TapHandler` is replaced by the delegate's own `onPressAndHold`. Once that signal is connected, the button marks the press as held and skips `clicked` on release, so there is one recogniser and no state to keep in sync.
  - The hold interval is the platform style hint: 800 ms in Qt 6.12.1, with no Android or iOS override. That is the same as the former `longPressThreshold: 0.8`.
  - A long press in selection mode toggles the row, the same as a tap.
- **Tested** in an offscreen Qt Quick Test mirroring the row (not in the app), with mouse and touch:
  - Long press outside selection mode: enters selection mode, flight selected, no click.
  - Tap in selection mode: toggles once per tap.
  - Long press in selection mode: toggles once.
  - Tap outside selection mode: click only, nothing selected.
  - Slow swipe lasting longer than 800 ms: swipe completes, no long press, no click.
- **Still open**: F-02 stays reachable by deselecting the last row or by filtering.

### F-02 Empty selection in selection mode exports the whole log — done

- **Where**: [FlightLogPage.qml:181-238](src/qml/pages/FlightLogPage.qml#L181-L238); comment at [61-65](src/qml/pages/FlightLogPage.qml#L61-L65)
- **What happens**: The four header export items ("Share/Export as ForeFlight CSV…", "… Flightlog JSON…", and the two Android "Save as…" items) pass `page.selectedUuids` and are enabled by `FlightLog.count > 0` only. With nothing selected they send every flight, including flights hidden by the filter, toast "Flight log shared" and leave selection mode.
- **How the state is reached**:
  - Formerly on every long press; that path is closed by the F-01 fix.
  - Deselecting the last selected row.
  - Typing a filter that hides the selected rows: the `Connections` at [66-83](src/qml/pages/FlightLogPage.qml#L66-L83) then empties the selection silently.
- **Why**: `flightsForUuids` returns all flights for an empty list ([FlightLog.cpp:658-662](src/flightlog/FlightLog.cpp#L658-L662)). The "Remove Selected Flights…" item at [310](src/qml/pages/FlightLogPage.qml#L310) guards this state correctly; the export items do not.
- **Also**: The comment at 61-65 says the export and remove actions always operate on the flights on screen. That is not true for export, in or out of selection mode.
- **Also**: A non-empty selection is exported in the order the flights were ticked, not in log order.
- **Status**: Done. Fixed on 2026-11-28.
- **Fix**:
  - A page property `hasActionTarget` (non-empty selection in selection mode, non-empty log otherwise) now enables the four export items and "Remove Selected Flights…" / "Clear Flight Log".
  - The comment now says that export and clear operate on the whole log, regardless of the filter, outside selection mode.
  - `flightsForUuids` walks the log and keeps the flights whose UUID is in the selection, so exports are in log order. This also replaces the quadratic lookup with a linear one.
- **Tested**: Build and `all_qmllint` with `-DFLIGHTLOG=ON`. Not run in the app.
- **Still open**: Should "Export" outside selection mode follow the filter? It currently exports the whole log, and the comment documents that.

### F-03 Row stays swiped open after "No" in the remove dialog — done

- **Where**: [FlightLogPage.qml:568-572](src/qml/pages/FlightLogPage.qml#L568-L572), empty handler at [811-812](src/qml/pages/FlightLogPage.qml#L811-L812)
- **What happens**: After a full swipe and "No", the row shows only the trash icon until the user swipes it back by hand. A tap on the stuck row does not emit `clicked`, so in selection mode it cannot be toggled.
- **Why**: `swipe.onCompleted` opens the dialog but never closes the swipe. `SwipeToDeleteDelegate` has no auto-close.
- **Evidence**: All three sibling pages call `iDel.swipe.close()` right after opening the dialog, e.g. [AircraftLibrary.qml:102-108](src/qml/pages/AircraftLibrary.qml#L102-L108) with the comment "Otherwise it stays, so return it to its normal position".
- **Status**: Done. Fixed on 2026-11-28.
- **Fix**: `swipe.onCompleted` calls `iDel.swipe.close()` right after opening the dialog, with the same comment as the sibling pages. Closing at once, rather than in `onRejected`, keeps the shared dialog free of a reference to a delegate that may be recycled while it is open.
- **Tested** on the Android emulator (API 36, x86_64, Debug build with `-DFLIGHTLOG=ON`), with three flights:
  - Full swipe, then "No": the row is back in its normal position while the dialog is still open, and stays there.
  - In selection mode, full swipe, then "No", then a tap on the row: the row is selected.
  - Full swipe, then "Yes": the flight is removed from the list and the database, the toast shows, and the row that moves up is not swiped.
  - No QML warnings in logcat.

### F-04 "Save as…" leaves selection mode before the save has a result — done

- **Where**: [FlightLogPage.qml:224-225](src/qml/pages/FlightLogPage.qml#L224-L225), [240-241](src/qml/pages/FlightLogPage.qml#L240-L241)
- **What happens**: On Android, cancelling the system file dialog or a write failure loses the selection, which then has to be rebuilt. The Share handlers on the same page keep the selection on abort and on error.
- **Why**: `FileExchange.saveContent` returns `void` and is asynchronous on Android; the result arrives through `saveContentResult` ([FileExchange_Abstract.h:126-140](src/platform/FileExchange_Abstract.h#L126-L140)). The page calls `exitSelectionMode()` on the next line. The only consumer of the result is `ImportManager.qml`, which knows nothing about the page.
- **Status**: Done. Fixed on 2026-11-28.
- **Fix**: The two header "Save as…" items set `page.savePending` and no longer call `exitSelectionMode()`. A page-level `Connections` on `FileExchange.saveContentResult` clears the flag and leaves selection mode only for an empty (success) result. Abort and errors keep the selection. `ImportManager.qml` still shows the toast or error dialog.
  - The flag is set before `saveContent()`, because two errors are emitted synchronously from inside it.
  - The flag keeps the page from reacting to saves started elsewhere in the app; the signal is shared.
  - The per-flight "Save as…" items are unchanged: their menu is hidden in selection mode.
- **Tested** on the Android emulator (API 36, x86_64, Debug build with `-DFLIGHTLOG=ON`), with two of three flights selected:
  - "Save as Flightlog JSON…", then Back in the system file dialog: toast "Aborted", both flights still selected.
  - "Save as ForeFlight CSV…", then Save: toast "File saved", selection mode ends. The file holds the two selected flights in log order, not in the order they were ticked (confirms the F-02 change).
  - No QML warnings in logcat.
  - Not tested: the synchronous error paths ("Another file save operation is already in progress", "Unable to open the system file dialog"); they cannot be triggered from the UI.

### F-05 System Back pops the page in selection mode — done

- **Where**: [FlightLogPage.qml:100-107](src/qml/pages/FlightLogPage.qml#L100-L107)
- **What happens**: The header arrow leaves selection mode. The Android back key or gesture closes the page instead and discards the selection. Escape on desktop does not leave selection mode either.
- **Why**: [main.qml:917-930](src/qml/main.qml#L917-L930) pops the StackView unconditionally, and this page has no `Keys` handler.
- **Tried, does not work** (2026-11-28): a page-level `Keys.onReleased` that accepts Back and Escape in selection mode. On the Android emulator (API 36, back gesture), Back in selection mode still popped the page.
  - **Cause**: the page never has active focus when it was opened from the drawer. `StackView` focuses the pushed page, but the drawer, closing right after, restores focus to the item that had it before it opened: the drawer button on the map page. Back then travels map page → StackView, and the handler in `main.qml` pops. Temporary logging showed `activeFocusItem` = `MapButton` and no call of the page handler.
  - The same applies to every page opened from the drawer, including the WebView back handling in [Manual.qml:113](src/qml/pages/Manual.qml#L113).
  - Side finding: `adb shell input keyevent 4` reaches `MobileAdaptor.onBackInvoked` twice on API 36 (pop, then quit at depth 1). The back gesture and the 3-button navigation bar deliver once. Test Back with those, not with `input keyevent`.
- **Status**: Done. Fixed on 2026-11-28.
- **Fix**: A page hook instead of a page key handler.
  - `FlightLogPage` defines `function handleBack(): bool`. In selection mode it leaves selection mode and returns true; otherwise it returns false.
  - `main.qml` asks the current page first, through `stackView.currentPageHandlesBack()`: in the StackView's `Keys.onReleased` for Back and Escape, and in the Android `onClosing` fallback. It pops only if the page did not handle the key. Escape never pops.
  - This works wherever focus is, so it also answers the question below for this page.
- **Tested** on the Android emulator (API 36, x86_64, Debug build with `-DFLIGHTLOG=ON`), with the back gesture:
  - Selection mode, Back: selection mode ends, the page stays. Back again: back to the map, the app keeps running.
  - Selection mode with the filter field focused and the keyboard open: the first Back closes the keyboard and keeps the selection; the second leaves selection mode.
  - Settings page (no hook), Back: back to the map.
  - No QML warnings in logcat.
  - Not tested: Escape on the desktop.
- **Still open**: [Manual.qml:113](src/qml/pages/Manual.qml#L113) has the same focus problem for its WebView "go back" and could move to `handleBack()`.
- **To discuss**: The `onClosing` fallback in `main.qml` bypasses page-level key handlers when the window rather than the StackView has focus. Is a page-level "can go back" hook worth adding for both paths?

### F-06 Success toasts are unconditional although the operations can fail — done

- **Where**: [790-793](src/qml/pages/FlightLogPage.qml#L790-L793), [807-810](src/qml/pages/FlightLogPage.qml#L807-L810), [822-825](src/qml/pages/FlightLogPage.qml#L822-L825), [835-839](src/qml/pages/FlightLogPage.qml#L835-L839), [863-867](src/qml/pages/FlightLogPage.qml#L863-L867), [875-878](src/qml/pages/FlightLogPage.qml#L875-L878)
- **What happens**: When a storage write fails, the user sees "Flights removed" (or "Flight added", "Flight updated", "Track deleted", "Flight log cleared") together with the app-wide error dialog, while the list is unchanged. For the multi-remove the selection is discarded as well. "Show on Map" does nothing, with no feedback, when the track file is unreadable.
- **Why**: The mutating invokables are `void` and return early on failure, e.g. [FlightLog.cpp:369-371](src/flightlog/FlightLog.cpp#L369-L371). `addFlight` also returns silently on a duplicate UUID.
- **Status**: Done. Fixed on 2026-11-28.
- **Fix**:
  - `addFlight`, `removeFlight`, `removeFlights`, `clearFlights`, `updateFlight`, `removeTrack` and `showTrack` return `bool`: true if the change was made (for `showTrack`: the track is displayed). Storage failures still emit `saveError()`, so `main.qml` keeps showing the error dialog with the reason; a `bool` is enough for the page.
  - The page toasts, and leaves selection mode after a multi-remove, only on true. On failure the selection is kept.
  - "Show on Map" toasts "Track data could not be read." on false, the string the per-flight "Save to IGC…" already uses.
  - Not `[[nodiscard]]`: `DemoRunner` and the flight detector call `addFlight()` and ignore the result.
- **Tested** on the Android emulator (API 36, x86_64, Debug build with `-DFLIGHTLOG=ON`). Storage failures were provoked by making the app's `files` directory read-only (`chmod 555`), so that SQLite cannot create its journal:
  - Swipe-remove, "Remove Selected Flights…", "Clear Flight Log", "Delete IGC Track…", editing a flight, "Add Flight": each shows the error dialog and no success toast; the list, the track file and (for the multi-remove) the selection are unchanged.
  - Editing without changes succeeds and toasts "Flight updated" even with the directory read-only: SQLite does not write a page whose content is unchanged. The stored flight matches the list, so the toast is correct.
  - Writable again: "Flight removed" and "Track deleted" are toasted, the track file is deleted.
  - "Show on Map" with the track file unreadable (`chmod 000`): toast "Track data could not be read."; with it readable: the track is shown, no toast.
  - No QML warnings in logcat.
- **Side findings** (not fixed):
  - The error dialog in [main.qml:1127](src/qml/main.qml#L1127) says "Recent changes will be lost when the app closes." Since the operations persist first, a failed change is not made at all; the text is misleading.
  - The flight editor closes on OK even when the update or add fails, so the user's input is lost.
  - If the flight detector's `addFlight()` fails at takeoff ([FlightLog.cpp:931](src/flightlog/FlightLog.cpp#L931)), `m_currentFlightUuid` points to a flight that is not in the log, and the landing is lost silently.

### F-07 "Show live flight trace" shows off while the map still draws the trace

- **Where**: [FlightLogPage.qml:166-175](src/qml/pages/FlightLogPage.qml#L166-L175)
- **What happens**: In flight with recording on and the trace shown, unticking "Record GPS track" turns the third item unchecked and disabled. The map keeps drawing the frozen partial trace until landing, and the user cannot hide it: the item is disabled, and "Hide Track from Map" only applies to saved tracks.
- **Why**: The menu models a chain (detection → recording → live trace) through `enabled` and `checked`. C++ implements only the first link: `displayedTrackPath()` tests `showCurrentFlightTrace()` alone ([FlightLog.cpp:776-790](src/flightlog/FlightLog.cpp#L776-L790)), and `setTrackRecording` only writes the setting ([FlightLog.cpp:271-278](src/flightlog/FlightLog.cpp#L271-L278)).
- **To discuss**: Which side should be authoritative? Either C++ honours the chain (no live path while recording is off, or the in-memory track is discarded when recording is switched off), or the menu item depends only on what C++ actually tests.

### F-08 Detection is off by default, but the page behaves as if it were on

- **Where**: [FlightLogPage.qml:425-427](src/qml/pages/FlightLogPage.qml#L425-L427), [745-753](src/qml/pages/FlightLogPage.qml#L745-L753)
- **What happens**:
  - The empty-list text says flights "will be automatically recorded when takeoff and landing are detected". A first-time user is told flights will appear by themselves, and nothing on the page indicates that detection is off.
  - "Add Flight" prefills the arrival airfield "if not in flight", tested as `detectionState === Idle`. With detection off the detector is never fed and stays Idle, so adding a flight shortly after takeoff prefills the arrival with the departure field. The editor does not warn about it.
- **Why**: `autoFlightDetection` defaults to `false` ([GlobalSettings.cpp:82](src/GlobalSettings.cpp#L82)), and the detector is skipped while it is off ([FlightLog.cpp:1021-1027](src/flightlog/FlightLog.cpp#L1021-L1027)).
- **Proposed fix**: Make the empty-state text depend on the setting (say that detection can be switched on in the menu, or offer a button). For the prefill, also require `Navigator.flightStatus !== Navigator.Flight`; the reason given at the edit button for not using `flightStatus` does not apply to a prefill heuristic.

### F-09 Hard-coded colours ignore the theme

- **Where**: [FlightLogPage.qml:343-347](src/qml/pages/FlightLogPage.qml#L343-L347), [482-488](src/qml/pages/FlightLogPage.qml#L482-L488)
- **What happens**:
  - The "has track" indicator is a plain `Image` of a black SVG at 0.55 opacity. In night mode it is black on a dark background.
  - The status line uses literal `#4CAF50` / `#2196F3` / `#FF9800` at 0.8 opacity and a fixed 14 px. On the light background the contrast is estimated at about 2.2:1 to 3.1:1 before the opacity reduction. This is the line a pilot reads in daylight.
- **Proposed fix**: Use the project's [Icon.qml](src/qml/items/Icon.qml), which tints with `Material.foreground`, for the indicator. Take the status colours from the Material palette, drop the extra opacity, and size the font relative to the control font as the rest of the page does.

### F-10 Counted strings without plural handling

- **Where**: [122](src/qml/pages/FlightLogPage.qml#L122) ("%1 selected"), [288](src/qml/pages/FlightLogPage.qml#L288) ("%1 flight(s) imported"), [837](src/qml/pages/FlightLogPage.qml#L837) ("Flights removed")
- **What happens**: The strings inflect by count, and the shipped translations show the workarounds, e.g. "%1 Flug/Flüge importiert" in German and a Polish form that is wrong for 1 and for 2-4.
- **Proposed fix**: `qsTr("%n …", "", count)` with numerus translations. The import toast is duplicated in `ImportManager.qml` under a second context and should share one source.
- **To discuss**: There is no `%n` form anywhere in the project yet, so this would be the first use and needs numerus forms in all six languages.

### F-11 Long comments break mid-word; time line has loose ends

- **Where**: [542-544](src/qml/pages/FlightLogPage.qml#L542-L544), [493-516](src/qml/pages/FlightLogPage.qml#L493-L516)
- **What happens**:
  - The summary line replaces every space with a no-break space, including in the free-text comment and pilot name. A field wider than the row (about 35-40 characters on a 360 px phone) has no break opportunity, so `Text.Wrap` breaks it at an arbitrary character. The label has no line cap, so a long comment adds unbounded lines.
  - A landing on an earlier UTC day than the takeoff (possible through JSON import) gets no marker; the editor shows "−1".
  - The comment "as the flight editor does" is inaccurate: the editor shows "+1", the list "⁺¹".
- **Proposed fix**: Apply the no-break treatment only to the short computed fields (callsign, duration, distance, landings), and cap the label with `maximumLineCount` plus `elide`.
- **To discuss**: Move both strings into `Flight`, next to `calculatedValues`, and use them from the page and the editor? That would also settle one day-offset notation.

### F-12 Ten copy-pasted export handlers that have drifted

- **Where**: header menu [183-242](src/qml/pages/FlightLogPage.qml#L183-L242), row menu [624-701](src/qml/pages/FlightLogPage.qml#L624-L701), error dialog [777](src/qml/pages/FlightLogPage.qml#L777)
- **What happens**: The same block (export, empty check, share, abort check, error dialog, toast) appears ten times. Besides F-04, the copies differ or share these defects:
  - All pass `"text/*"`, which is not a MIME type. Sibling pages pass exact types such as `"application/geo+json"`.
  - Import failures appear in `shareErrorDialog` under the title "Error Sharing Data…".
  - `qsTr("FlightLog")` is the file-name stem, so exports get translated file names, and every single-flight IGC export gets the same generic name.
  - Three spellings of one term ("Flight Log", "Flightlog", "FlightLog"), and "Share to IGC…" next to "Share as …". Translators read "to IGC" as a destination ("Mit IGC teilen…").
  - "Clear Flight Log" lacks the ellipsis that its twin "Remove Selected Flights…" has.
  - Sentence case and Title Case mixed in the same menu.
- **Proposed fix**: One page-level function taking data, suffix, MIME type and success message, called from all ten places. Fix the strings in the same pass.
- **To discuss**: Which spelling, and Title Case or sentence case for menu entries? A fixed untranslated file-name stem, or a per-flight name built from date and airfields?

### F-13 ScrollIndicator override

- **Where**: [FlightLogPage.qml:410](src/qml/pages/FlightLogPage.qml#L410)
- **What happens**: The line replaces the indicator that `DecoratedListView` installs with `visible: !isDesktop`. On desktop the list shows both the scrollbar and a touch-style indicator.
- **Evidence**: [DecoratedListView.qml:158-167](src/qml/items/DecoratedListView.qml#L158-L167): "lists using DecoratedListView should NOT set their own".
- **Proposed fix**: Delete the line.

### F-14 Row has no `clicked()` contract and no primary action

- **Where**: [434-440](src/qml/pages/FlightLogPage.qml#L434-L440), [550-555](src/qml/pages/FlightLogPage.qml#L550-L555)
- **What happens**: Outside selection mode a tap on a row vibrates and does nothing. The delegate root does not expose `signal clicked()` as the sibling delegates do, so Return/Enter in `DecoratedListView` and Return in the filter field are silent no-ops on this page.
- **To discuss**: Should a tap open the editor (under the same condition as the edit button)? If not, drop the vibration for the no-op.

### F-15 Dead code

- Four empty `onRejected` handlers: [794](src/qml/pages/FlightLogPage.qml#L794), [811](src/qml/pages/FlightLogPage.qml#L811), [840](src/qml/pages/FlightLogPage.qml#L840), [854](src/qml/pages/FlightLogPage.qml#L854). `clearDialog` has none, which shows they are not needed.
- Ids never referenced: `lbl`, `backButton`, `detectionStatusLabel`, `editButton`, `cptMenuButton`.
- `cascade: true` on a menu without submenus ([143](src/qml/pages/FlightLogPage.qml#L143)).

### F-16 Missing `readonly` on platform flags

- **Where**: [FlightLogPage.qml:37-39](src/qml/pages/FlightLogPage.qml#L37-L39)
- `isAndroid`, `isIos` and `isAndroidOrIos` are bound once and never assigned. The sibling pages carry the same non-readonly pattern, so this is a codebase-wide nit rather than a defect of this page.

---

## To verify

These are suspicions that need a run to settle.

| ID | Suspicion | How to verify |
|----|-----------|---------------|
| V-01 | The per-row menu with nine entries ([604-724](src/qml/pages/FlightLogPage.qml#L604-L724)) is roughly half of each delegate's objects; three entries are Android-only but created everywhere. It may cost frames while flicking a long log. | qmlprofiler with a few hundred flights. If it shows, use one page-level menu, as the dialogs already do. |
| V-02 | The status label ([332-363](src/qml/pages/FlightLogPage.qml#L332-L363)) can neither wrap nor elide. The French and Italian "Takeoff detected…" strings are estimated at 325-350 px against about 332 px available on a 360 px screen. | Run in French at 360 px in TakeoffPhase. Adding `wrapMode: Text.WordWrap` is harmless either way. |
| V-03 | `"text/*"` goes unchanged into the Android create-document intent used by "Save as…", which expects a concrete type. | Save CSV, JSON and IGC into Downloads on the emulator and check name and extension. Settled anyway if F-12 switches to concrete types. |
| V-04 | Status label, End Flight button and filter field take about 160 px above the list. On a 360 px-high landscape phone an estimated 40 px remain for the list while in flight. | Run at 760x360 with the detector in flight. |

---

## Lint

The style linter of the Qt review skill reports 58 hits in 9 rule groups. None is a defect.

| Rule | Hits | Assessment |
|------|------|------------|
| JS-1 `var` instead of `let`/`const` | 33 | Worth converting when the handlers are touched; most disappear with F-12. |
| ORD-1 attribute ordering | 12 | Cosmetic; sibling pages use the same order. |
| BND-2 imperative `highlighted = false`, `shareErrorDialog.text = …` | 8 | No binding exists on either property. Intentional. |
| STY-3 `anchors.*` dot notation | 2 | Cosmetic. |
| BND-1 `property var selectedUuids` ([43](src/qml/pages/FlightLogPage.qml#L43)) | 1 | `property list<string>` would type it. |
| IMP-3 unqualified `QtQuick.Controls` import | 1 | Needed anyway if F-09 uses the Material palette. |
| STY-1 `id: page` rather than `id: root` | 1 | Project convention. Ignore. |
| PRF-3 `clip: true` on the ListView | 1 | Acceptable. |
| PRF-5 `Text.RichText` | 1 | App-wide convention. Ignore. |

---

## Checked and cleared

- **Row height**: The delegate binds `height: iDel.height` on a `RowLayout` that itself sizes `iDel`. One analysis pass claimed the row could not grow when the third line wraps. An offscreen test refutes this: the row grew from 160 to 180 px when the text re-wrapped. Binding to `implicitHeight` would be more direct but is not a defect.
- **Empty state**: The list and the empty-state label both fill the height, but they do not split the space. In the test the list got 9 px and the label the rest.
- **Selection sync**: `UuidRole`, `flight.uuidString` and `displayedTrackUuid` all use the brace-less UUID format. Every way a row leaves the proxy (filter change, remove, clear, a data change that fails the filter) reaches one of the two `Connections` handlers.
- **C++ boundary**: All call signatures and argument orders match, including the ten-argument `createFlight`. `importFromJSON` handles the `file://` URL from the file dialog.
- **`checked` bindings**: The checkable menu items and the CheckBox keep their bindings when the control toggles itself.
- **Edit-button comment** ([578-585](src/qml/pages/FlightLogPage.qml#L578-L585)): Its claims hold. The button is hidden for all rows while a flight is in progress, which is more restrictive than necessary but not wrong.
- **FileDialog inside a MenuItem**, **`Global.dialogLoader` usage**: Both follow the established pattern of the sibling pages.

---

## Appendix: reproducing F-01

This reproduces the behaviour before the fix. Save as `tst_longpress.qml` and run with `QT_QPA_PLATFORM=offscreen qmltestrunner -input tst_longpress.qml`. The test fails as written. Replacing the `TapHandler` with `onPressAndHold` on the `SwipeDelegate`, as the fix does, makes it pass.

```qml
import QtQuick
import QtQuick.Controls.Material
import QtTest

Item {
    id: page
    width: 400; height: 600

    property bool selectionMode: false
    property var selectedUuids: []

    function toggleSelection(uuid) {
        const arr = page.selectedUuids.slice()
        const pos = arr.indexOf(uuid)
        if (pos >= 0) arr.splice(pos, 1); else arr.push(uuid)
        page.selectedUuids = arr
    }

    SwipeDelegate {
        id: iDel
        width: parent.width
        text: "flight"
        swipe.enabled: true
        swipe.right: Item { anchors.fill: parent }

        onClicked: if (page.selectionMode) page.toggleSelection("a")

        TapHandler {
            longPressThreshold: 0.8
            onLongPressed: {
                page.selectionMode = true
                page.toggleSelection("a")
            }
        }
    }

    TestCase {
        name: "LongPress"
        when: windowShown

        function test_longPress() {
            mousePress(iDel, 50, 20)
            wait(1100)
            compare(page.selectedUuids.length, 1)   // selected by the long press
            mouseRelease(iDel, 50, 20)
            compare(page.selectedUuids.length, 1)   // fails: the click deselected it
        }
    }
}
```
