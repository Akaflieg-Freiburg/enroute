# QML Code Review Report

**Scope**: files: `src/qml/items/MFM.qml` (current file)
**Files reviewed**: 1 (812 lines)
**Issues found**: 21 (8 from lint, 8 from deep analysis, 5 investigation targets)
**qmllint**: ran (against built module `build/claude/src`)

> **Cross-cutting note:** The qmllint "Member not found on type `Global`" warnings are **false positives**. `Global.qml` defines every flagged member (`currentVAC`, `mapBearingPolicyRect`, `followGPS`, `drawer`, `dialogLoader`, the `*Rect` getters); qmllint cannot fully resolve the singleton's type because of unimportable property types (`vac`, `Drawer`, `Loader`). Runtime impact: none.

---

## Lint findings

### [L-001] `QtQuick.Controls` imported without style qualifier — ✅ WON'T FIX (false positive)
- **File**: `src/qml/items/MFM.qml:27`
- **Rule**: IMP-3
- **Finding**: Plain `import QtQuick.Controls`.
- **Resolution**: **Won't fix — false positive in this codebase.** The app sets the style globally via `QQuickStyle::setStyle("Material")` (`src/main.cpp:217`), so the plain `import QtQuick.Controls` is the correct idiom — it resolves to Material at runtime. Adding a style qualifier (e.g. `QtQuick.Controls.Basic`) would force this screen's controls to a different style and break the Material look. Left as-is by design.

### [L-002] Top-level component lacks `id: root` — ✅ DONE
- **File**: `src/qml/items/MFM.qml:37`
- **Rule**: STY-1
- **Finding**: Root `Item` used `id: page`.
- **Resolution**: **Fixed.** Renamed `id: page` → `id: root` and updated the one internal reference (`page.height` → `root.height` at former line 52). Comment ("page header", line 91) and the user-facing string ("management page", line 476) were left untouched — they are not the id. Verified clean with qmllint.

### [L-003] Unused imports — ✅ DONE
- **File**: `src/qml/items/MFM.qml:22` (`QtCore`), `src/qml/items/MFM.qml:30` (`QtQuick.Shapes`)
- **Rule**: IMP (corroborated by qmllint `unused-imports`)
- **Finding**: Neither module is referenced.
- **Resolution**: **Fixed.** Removed both `import QtCore` and `import QtQuick.Shapes`. qmllint `unused-imports` warnings cleared.

### [L-004] `var` instead of `let`/`const` — ✅ DONE
- **File**: lines 161, 210, 299, 300, 302, 315, 316, 318, 376, 472, 586, 589, 590, 745
- **Rule**: JS-1
- **Finding**: Function-scoped `var` with hoisting hazards throughout the handlers.
- **Resolution**: **Fixed.** All `var` declarations converted: `let` for the reassigned ones (`newZoom` in `onScaleChanged`/`onWheel`, `coordinate` in the center binding), `const` for the rest (`pos`, `posTr`, `wp`, `t`, `resultList`, `airspaceAltitudeLimit`, `airspaceAltitudeLimitString`, `newZoomLevel`). No `var` remains; qmllint clean.

### [L-005] Imperative `=` destroys `zoomLevelBehavior.enabled` binding— ✅ DONE
- **File**: lines 168, 176, 209, 218, 428, 430
- **Rule**: BND-2
- **Finding**: `zoomLevelBehavior.enabled` is toggled imperatively around zoom writes.
- **Mitigation**: **Intentional** (disabling the animation during gesture-driven zoom). See [D-008] for state-correctness analysis.

### [L-006] `Text.RichText` invokes full HTML/CSS parser— ✅ WONTFIX
- **File**: `src/qml/items/MFM.qml:478`
- **Rule**: PRF-5
- **Finding**: `noMapWarning` uses RichText.
- **Mitigation**: RichText is needed here (`<p>`, `<a>`, link activation). Acceptable. See [I-003] for the un-flagged sibling at line 620.

### [L-007] `anchors.*` dot notation (3 properties) — ✅ DONE
- **File**: `src/qml/items/MFM.qml:514`
- **Rule**: STY-3
- **Finding**: Three `anchors.*` assignments using dot notation.
- **Resolution**: **Fixed.** The `Pane` (formerly line 514, now ~519) now uses group notation: `anchors { fill: parent; bottomMargin: menuButton.bottomInset; topMargin: menuButton.topInset }`. STY-3 cleared; qmllint clean.

### [L-008] ~~Loose equality (JS-2)~~ — FALSE POSITIVE, suppressed
- **File**: lines 353, 582, 595, 684
- **Finding**: The linter flagged these as `==`/`!=`, but all four use strict `!==`/`===`. The linter's regex matches `!=` inside `!==`.
- **Mitigation**: None — listed only to prevent a wasted "fix."

---

## Deep analysis findings

### [D-001] Double `vibrateBrief()` call — ✅ DONE
- **File**: `src/qml/items/MFM.qml:559-560`
- **Category**: Performance & Quality
- **Confidence**: 98/100
- **Finding**: `trafficDataReceiverButton.onClicked` calls `PlatformAdaptor.vibrateBrief()` twice in a row. Every other button calls it once.
- **Trace**: All `vibrateBrief()` sites (502, 540, 687, 704, 731, 744, 758) are single calls; only 559-560 is doubled.
- **Resolution**: **Fixed.** Removed the duplicate `PlatformAdaptor.vibrateBrief()` line; the handler now calls it once like every other button.

### [D-002] Unused `id` assignments — ✅ DONE
- **File**: lines 73, 81, 458, 576, 615, 697, 792
- **Category**: Performance & Quality
- **Confidence**: 90/100
- **Finding**: `remainingRoute`, `splitView`, `noMapWarningRect`, `airspaceAltLabel`, `noCopyrightInfo`, `recentFilesInstantiator`, `rawSideView` are never referenced internally or via `objectName`/C++.
- **Trace**: Grepped each id across the project (0 external refs). Confirmed `drag`, `wheel`, `pinch`, `centerItem`, `col2`, `bearingBinding`, `centerBinding`, `panCommitTimer` ARE used (do not remove those).
- **Resolution**: **Fixed.** Removed all seven unused `id:` declarations. Re-verified project-wide before removal — note the `rawSideView` instance id in MFM.qml is distinct from `Sideview.qml`'s own root `id: rawSideView` (28 refs there are internal to that component), so removing the MFM.qml instance id is safe. qmllint clean.

### [D-003] `font.pixelSize` unqualified inside the `center` binding — ✅ DONE
- **File**: `src/qml/items/MFM.qml:385`
- **Category**: Bindings & Properties / Performance
- **Confidence**: 88/100
- **Finding**: The binding's scope object is the QtLocation `Map` (`flightMap`), which has **no** `font` property. `font.pixelSize` resolves up the scope chain to the enclosing `SplitView`'s `font`. It works today but is fragile — and the value is used as a layout dimension (GUI-clearance radius), so a wrong resolution would mis-position the map center.
- **Trace**: qmllint: "font is a member of a parent element," proposes `splitView.font.pixelSize`. Confirmed `Map` has no `font` property.
- **Resolution**: **Fixed.** Replaced `2*font.pixelSize` with `2*GlobalSettings.fontSize`. Verified numerically equivalent: the window sets `font.pixelSize: GlobalSettings.fontSize` (`src/qml/main.qml:46`) and the `SplitView` inherits it unchanged, so the value is identical — but now fully qualified via a singleton (no scope-chain fragility, and immune to the `id: splitView` removal in D-002). qmllint `unqualified` warning at line 385 cleared.

### [D-004] Missing `pragma ComponentBehavior: Bound` — ✅ DONE
- **File**: top of file; affects the Instantiator delegate at `src/qml/items/MFM.qml:696-714`
- **Category**: Bindings & Properties / Delegates
- **Confidence**: 85/100
- **Finding**: The `CheckDelegate` reads outer-scope ids `rasterMenu` (705), `flightMap` (707), and singleton `GeoMapProvider` (700, 706) with no `pragma ComponentBehavior: Bound`. qmllint flags 700/701/705/706/707 as unqualified.
- **Trace**: No pragma in the file. Nuance: **Instantiator does not pool/recycle delegates** (unlike ListView/Repeater), so there is no stale-identity-on-reuse hazard here — the real cost is qmltc compile-compatibility and slower interpreted lookups, plus Qt's direction to make `Bound` the default.
- **Resolution**: **Fixed.** Added `pragma ComponentBehavior: Bound` after the license block (matching the convention in `DecoratedScrollView.qml` etc.) and declared `required property string modelData` in the `CheckDelegate`. qmllint's `unqualified` warnings on the delegate's outer-id accesses are cleared; no new warnings introduced.

### [D-005] `BrightnessContrast` → `MultiEffect` migration opportunity
- **File**: `src/qml/items/MFM.qml:21` (import), `src/qml/items/MFM.qml:449-455`
- **Category**: States & Structure (migration)
- **Confidence**: 88/100
- **Finding**: `Qt5Compat.GraphicalEffects.BrightnessContrast` is the legacy path; Qt 6.5+ recommends `MultiEffect` (QtQuick.Effects), which has `brightness`/`contrast` with the same −1.0…1.0 ranges and no extra overhead. Current values (brightness −0.9/−0.2, contrast 0.6/0.2) port directly.
- **Trace**: Verified against Qt 6.11 docs; the BrightnessContrast page itself recommends MultiEffect; ranges match.
- **Mitigation**: Swap `import Qt5Compat.GraphicalEffects` → `import QtQuick.Effects`, change to `MultiEffect { source: flightMap; brightness: ...; contrast: ...; anchors.fill: flightMap; visible: ... }`. **Caveat**: MultiEffect does not hide its source. Since `flightMap` stays visible (required for gestures), **verify on-device** that the effect overlay still occludes the live map. Defer if device verification isn't possible.

### [D-006] DemoRunner's imperative `flightMap.bearing` write can silently no-op
- **File**: `src/qml/items/MFM.qml:46-48` vs `src/qml/items/MFM.qml:349-355`
- **Category**: Component Loading & Lifecycle
- **Confidence**: 82/100
- **Finding**: `onRequestMapBearing` does `flightMap.bearing = bearing` imperatively, but `Binding on bearing` is active whenever `mapBearingPolicyRect !== UserDefinedBearingUp`. While active, the Binding dominates and immediately overwrites the imperative value — so demo bearing commands only "stick" in UserDefinedBearingUp mode. (`onRequestZoomLevel` is fine — `zoomLevel` has only a `Behavior`, so the demo write animates correctly.)
- **Trace**: Binding precedence — active `Binding` wins over imperative assignment; `RestoreNone` doesn't restore but the active binding clobbers.
- **Mitigation**: If demo bearing commands must always apply, set `Global.mapBearingPolicy = MFM.UserDefinedBearingUp` first, or route through `Global.mapBearing` under the policy rather than writing `flightMap.bearing` directly.

### [D-007] Persisted `mapCenter` is effectively never restored on a normal launch
- **File**: `src/qml/items/MFM.qml:419-432`; `Global.qml:87`
- **Category**: States & Structure
- **Confidence**: 80/100
- **Finding**: `onMapReadyChanged` restores `center = Global.mapCenterRect` only `if (!centerBinding.when)`, i.e. only when `Global.followGPS` is false. But `followGPS` defaults to `true` and isn't persisted as false, so on a normal launch the GPS-follow binding wins immediately and the saved center is ignored. `RestoreNone` itself is **correct and intentional** for both bindings (prevents pan-fighting); this is a design observation, not a code bug. Bearing is symmetric and fine.
- **Trace**: Cross-referenced `centerBinding.when: Global.followGPS` with `Global.followGPS` default `true`.
- **Mitigation**: If honoring persisted `mapCenter` across launches is desired, that's a separate persistence decision. At minimum add a comment that `RestoreNone` is deliberate so it isn't "fixed" to the default later.

### [D-008] `zoomLevelBehavior.enabled` toggle is leak-free today but exception/edit-fragile — ✅ DONE
- **File**: `src/qml/items/MFM.qml:410-413`; toggles at 168/176, 209/218, 428/430
- **Category**: States & Structure
- **Confidence**: 80/100
- **Finding**: Every disable→enable pair was traced; **no path leaves the Behavior stuck disabled** (early returns in `onMapReadyChanged` all occur before line 428). The risk is fragility: any future early-`return`, or a JS exception thrown between a `false` and its `true` (e.g. inside `anchorToCentroid`/`alignCoordinateToPoint` in `onScaleChanged`), would permanently disable the zoom animation with no recovery path.
- **Trace**: Straight-line analysis of all six toggle sites; pinch `onScaleChanged` is the most exposed.
- **Mitigation**: Prefer a declarative `enabled:` expression driven by a state flag, or wrap critical sections in `try { ... } finally { zoomLevelBehavior.enabled = true }`.
- **Resolution**: **Fixed.** All three toggle sites (`onScaleChanged`, `onWheel`, `onMapReadyChanged`) now wrap the body between `enabled = false` and `enabled = true` in `try { ... } finally { zoomLevelBehavior.enabled = true }`, so the Behavior is always re-enabled even if an intervening call (e.g. `anchorToCentroid`) throws or a future early-`return` is added. qmllint clean.

---

## Investigation targets (human verification needed)

### [I-001] `SplitView.minimumHeight` can collapse to 0 when Follow-GPS is on
- **File**: `src/qml/items/MFM.qml:93`
- **Category**: Layout & Anchoring
- **Confidence**: 72/100
- **Finding**: `SplitView.minimumHeight: followGPSButton.height*5`. But `followGPSButton.visible: enabled` where `enabled: !Global.followGPS` (line 537) — so when Follow-GPS is **on**, the button is hidden, and a hidden child in a `ColumnLayout` collapses to `height: 0`, making the minimum **0**. The map pane could then be collapsed to nothing, defeating the documented Qt 6.10.1 workaround.
- **Unverified because**: Whether the layout reports `followGPSButton.height` as 0 when hidden depends on runtime behavior; not confirmable statically.
- **How to verify**: Run the app, turn Follow-GPS **on**, try to drag the SplitView splitter to collapse the map. If it collapses below `followGPSButton.height*5`, confirmed. Fix with a constant floor (`Math.max(const, ...)`) or an always-visible metric.

### [I-002] `Binding on center` runs spherical trig every animation frame while following
- **File**: `src/qml/items/MFM.qml:370-399`
- **Category**: Performance & Quality
- **Confidence**: 75/100
- **Finding**: While Follow-GPS is active, `value` depends on `flightMap.animatedCoordinate` and `animatedTT` (both 1000 ms animations), so it re-evaluates ~60 Hz — running `Math.min`, divisions, `isFinite`, and up to **two** `atDistanceAndAzimuth()` spherical-trig calls per frame, plus per-frame `onCenterChanged` writes to `Global.mapCenter` and `PositionProvider.mapCenter`. A soft re-entrancy via `pixelPer10km` (depends on center/zoom) exists but is damped, not divergent.
- **Unverified because**: Whether this is a measurable bottleneck vs. map-tile rendering is unknown without profiling.
- **How to verify**: Run qmlprofiler during GPS-follow flight; check the binding's evaluation count/time. If hot, cache the radius and pixel offsets as `readonly property real` recomputed only on geometry/zoom change, leaving only the two unavoidable `atDistanceAndAzimuth` calls per frame.

### [I-003] `noCopyrightInfo` Label has implicit `textFormat` with HTML content
- **File**: `src/qml/items/MFM.qml:614-633` (HTML at 620)
- **Category**: Performance & Quality
- **Confidence**: 70/100
- **Finding**: `text` contains `<font>`/`<a>` markup and uses `onLinkActivated`, but no explicit `textFormat`. Default `Text.AutoText` runs the `mightBeRichText()` heuristic on each assignment. Un-flagged sibling of the PRF-5 finding at line 478.
- **Unverified because**: Cost is low (rarely changes); flagged for consistency.
- **How to verify**: Set `textFormat: Text.StyledText` (supports `<a href>`/`<font>`, cheaper than RichText) and confirm the link still activates.

### [I-004] `airspaceAltLabel.text` should be `Text.PlainText`
- **File**: `src/qml/items/MFM.qml:585-598`
- **Category**: Performance & Quality
- **Confidence**: 68/100
- **Finding**: The binding always produces plain text (`join(" • ")`) but defaults to `Text.AutoText`, re-running markup detection on every dependency change. Not on a hot path.
- **Unverified because**: Minor cost; no measured impact.
- **How to verify**: Add `textFormat: Text.PlainText`; confirm the `•` separator and dynamic strings still render correctly.

### [I-005] `CheckDelegate.checked` two-way pattern is correct only by C++ contract
- **File**: `src/qml/items/MFM.qml:700`, `src/qml/items/MFM.qml:706`
- **Category**: ListView & Delegates
- **Confidence**: 72/100
- **Finding**: `checked: modelData === GeoMapProvider.currentRasterMap` plus `onClicked` writing `currentRasterMap`. Mutual exclusivity is achieved by the data layer re-evaluating all delegates' `checked` when `currentRasterMap` changes. `currentRasterMap` is a notifying `BINDABLE` `QProperty` and the C++ setter is idempotent — so it self-heals today. Latent fragility: if `currentRasterMap` ever became non-notifying, the `checked` binding would silently go stale after the first click.
- **Unverified because**: Future-regression risk, not a present bug.
- **How to verify**: Keep `currentRasterMap` notifying. Optionally make the relationship explicit (handle in `onToggled` / use `autoExclusive`).

---

## Summary

| Category | Lint | Deep | Investigate | Total |
|----------|------|------|-------------|-------|
| Bindings & Properties | 1 | 2 | 0 | 3 |
| Layout & Anchoring | 1 | 0 | 1 | 2 |
| Component Loading & Lifecycle | 0 | 1 | 0 | 1 |
| ListView & Delegates | 0 | 1 | 1 | 2 |
| States & Structure | 0 | 3 | 0 | 3 |
| Performance & Quality | 4 | 1 | 3 | 8 |
| Imports/Style (general) | 2 | 0 | 0 | 2 |
| **Total** | **8** | **8** | **5** | **21** |

Findings below confidence 60 were suppressed. The qmllint `Global.*` "missing-property" warnings and the linter's JS-2 "loose equality" flags are **confirmed false positives** and excluded from the actionable counts.

**Top priorities**: [D-001] double-vibrate (trivial, definite bug), [I-001] SplitView min-height collapse (verify at runtime — most likely real defect), [D-003] unqualified `font.pixelSize` in the center calculation. The migration ([D-005]) and binding-precedence note ([D-006]) are the higher-effort items.

---

## Resolution log

- **[L-001]** ✅ Won't fix — false positive; app uses global `QQuickStyle::setStyle("Material")`, plain Controls import is correct.
- **[L-002]** ✅ Done — `id: page` → `id: root` (+ reference at line 52); qmllint clean.
- **[L-003]** ✅ Done — removed unused `import QtCore` and `import QtQuick.Shapes`.
- **[L-004]** ✅ Done — all `var` → `let`/`const`; no `var` remains; qmllint clean.
- **[L-007]** ✅ Done — `Pane` anchors converted from dot notation to group notation; STY-3 cleared.
- **[D-001]** ✅ Done — removed the duplicate `vibrateBrief()` call in `trafficDataReceiverButton`.
- **[D-002]** ✅ Done — removed seven unused `id:` declarations (`remainingRoute`, `splitView`, `noMapWarningRect`, `airspaceAltLabel`, `noCopyrightInfo`, `recentFilesInstantiator`, `rawSideView`).
- **[D-003]** ✅ Done — `2*font.pixelSize` → `2*GlobalSettings.fontSize` (numerically identical, fully qualified).
- **[D-004]** ✅ Done — added `pragma ComponentBehavior: Bound` + `required property string modelData` in the CheckDelegate.
- **[D-008]** ✅ Done — three zoom-Behavior toggle sites wrapped in `try/finally` so the animation is always re-enabled.
