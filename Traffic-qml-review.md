# QML Code Review Report — Traffic.qml & TrafficLabel.qml

**Scope**: files: `src/qml/items/Traffic.qml`, `src/qml/items/TrafficLabel.qml`
**Files reviewed**: 2 (107 + 119 lines)
**Issues found**: 24 (8 from lint, 8 from deep analysis, 8 investigation targets)
**qmllint**: ran (against built module `build/claude/src`) — **clean** for both files

> **Context from the C++ trace (de-risks several findings):** these two components are delegates of `MapItemView { model: TrafficDataProvider.trafficObjects }` (`FlightMap.qml:791-799` and `879-888`). The model is a **`CONSTANT` fixed pool of 20 pre-allocated, `CppOwnership` `TrafficFactor_WithPosition*` objects** built once in the provider's constructor — it never grows/shrinks, so there is **no delegate churn** and `trafficInfo` is **never null** on a live item. `trafficInfo.icon` is a **base64 `data:image/svg+xml` URI** (recolored from a bundled ~135×135 SVG), not a resource path.

---

## Lint findings

### [L-001] Top-level component lacks `id: root`
- **File**: `Traffic.qml:28`, `TrafficLabel.qml:26`
- **Rule**: STY-1
- **Finding**: Roots use `id: traffic1MapItem` / `id: trafficLabel`.
- **Mitigation**: Convention-only; rename to `root` for consistency if desired. Not a defect.

### [L-002] `property var trafficInfo`
- **File**: `Traffic.qml:33`, `TrafficLabel.qml:29`
- **Rule**: BND-1
- **Finding**: Untyped `var` for the central model object.
- **Mitigation**: See [D-001] — type as `TrafficFactor_WithPosition`.

### [L-003] Attribute ordering
- **File**: `Traffic.qml:47,59,67`, `TrafficLabel.qml:44,84,92`
- **Rule**: ORD-1
- **Finding**: Property assignments/declarations interleaved with child objects (`coordinate`/`visible` after the `Behavior`; `id` after assignments).
- **Mitigation**: Reorder to id → properties → assignments → children. Cosmetic.

### [L-004] `Image` without `sourceSize`
- **File**: `Traffic.qml:94`
- **Rule**: IMG-1
- **Finding**: Traffic icon Image has no `sourceSize`.
- **Mitigation**: See [D-006] — real impact (135×135 SVG data-URI drawn at 40px).

### [L-005] `QtQuick.Controls` without style qualifier — WON'T FIX
- **File**: `TrafficLabel.qml:24`
- **Rule**: IMP-3
- **Finding**: Plain `import QtQuick.Controls`.
- **Mitigation**: Same false positive as MFM's L-001 — the app sets `QQuickStyle::setStyle("Material")` globally, so the plain import is correct. No action.

### [L-006] `var` instead of `let`/`const` — ✅ DONE
- **File**: `TrafficLabel.qml:63-68` (`s`, `c`, `halfW`, `halfH`, `edge` in `distFromCenter`)
- **Rule**: JS-1
- **Finding**: Function-scoped `var`.
- **Resolution**: **Fixed** as part of [D-002] — the `distFromCenter` body was rewritten using `const` throughout (and the `edge` temporary was eliminated). No `var` remains.

### [L-007] `Text.RichText` invokes full HTML parser
- **File**: `TrafficLabel.qml:98`
- **Rule**: PRF-5
- **Finding**: Label uses RichText for `trafficInfo.description`.
- **Mitigation**: See [D-005] — `description` is `<br>`-only, so `Text.StyledText` is the correct, cheaper tier.

### [L-008] ~~Loose equality (JS-2)~~ — FALSE POSITIVE
- **File**: `Traffic.qml:53,54`, `TrafficLabel.qml:87`
- **Finding**: All use strict `!==`; the linter regex matches `!=` inside `!==`. No action.

---

## Deep analysis findings

### [D-001] Type `trafficInfo` as `TrafficFactor_WithPosition` — ✅ DONE
- **File**: `Traffic.qml:33`, `TrafficLabel.qml:29`
- **Category**: Bindings & Properties
- **Confidence**: 90/100
- **Finding**: The model element type `TrafficFactor_WithPosition` is `QML_ELEMENT` and importable. Typing the property lets qmlsc compile the hot chained accesses (`positionInfo.trueTrack().toDEG()`, `uncertaintyRadius.toM()`, `.icon`, `.color`) to C++ instead of interpreted `var` lookups, and catches contract drift at lint time.
- **Trace**: `TrafficFactor_WithPosition.h:40-42` (`QML_ELEMENT`, derives `TrafficFactor_Abstract`); model is `QList<TrafficFactor_WithPosition*>` (`TrafficDataProvider.h:143`). Verified every member used resolves on the type (own + inherited from `TrafficFactor_Abstract`) before changing it.
- **Resolution**: **Fixed.** `property var trafficInfo` → `property TrafficFactor_WithPosition trafficInfo` in both files (the `import akaflieg_freiburg.enroute` needed by TrafficLabel.qml was already added in [D-004]). Dropped TrafficLabel's `({})` default — a typed object property defaults to `null`, which **subsumes [I-003]** (the `{}` default was dead and would itself have thrown on chained access). Both files compile via qmlsc; qmllint clean.

### [D-002] Label-to-symbol gap is not constant across rotation angles — ✅ DONE
- **File**: `TrafficLabel.qml:61-72`
- **Category**: Layout & Anchoring
- **Confidence**: 88/100
- **Finding**: The comment promises a constant `symbolGap` (26px) "independent of direction t," but `edge = min(halfW/s, halfH/c)` measures along the offset ray, not to the nearest corner of the axis-aligned label. The gap is exactly 26 only at cardinal angles; on diagonals it shrinks to roughly half (e.g. ~13px for an 80×24 label at t≈60°), so the label can crowd/overlap the 40px icon at intermediate headings.
- **Trace**: Numerically evaluated true rect-to-origin distance vs. the formula across t.
- **Resolution**: **Fixed.** `distFromCenter` now expands each half-extent by `symbolGap` and intersects the offset ray with the enlarged box: `min((0.5*width+symbolGap)/s, (0.5*height+symbolGap)/c)` (s/c guards kept). Verified numerically: the nearest label edge now stays ≥26px from the symbol at every angle (was 13.4px at t≈60° before). Rewritten with `const` (clears [L-006]).

### [D-003] Repeated `trueTrack()` / `toM()` metacalls in bindings — ✅ DONE
- **File**: `Traffic.qml:55,60,91` (+ `uncertaintyRadius.toM()` at 77,87,103); `TrafficLabel.qml:37,38,47,48`
- **Category**: Performance & Quality
- **Confidence**: 88/100
- **Finding**: `positionInfo.trueTrack()` returns a `Units::Angle` gadget by value via `Q_INVOKABLE`, and `.isFinite()`/`.toDEG()` are a *second* metacall on it — none cacheable by the engine. Traffic.qml re-derives the same `isFinite` 3×; TrafficLabel 4×. These re-run on every `bearing` change (i.e. per map-rotation frame) × N targets.
- **Trace**: `PositionInfo.h:147`, `Angle.h:85,172` — all `Q_INVOKABLE`.
- **Resolution**: **Fixed.** Added cached `readonly` root properties in both files — `trafficPositionInfo` (one `positionInfo` snapshot), `trafficTrueTrack` (one `trueTrack()` call), `trafficTrueTrackValid` (one `isFinite()`), and in Traffic.qml `trafficUncertaintyM` (one `toM()`). All the bindings now reference these instead of re-invoking. In Traffic.qml the boundary crossings drop from ~7 `trueTrack()`+3 `toM()` per re-eval to one each; the single snapshot also keeps derived values consistent. `groundSpeed()` (called once) now reads the cached `trafficPositionInfo`.

### [D-004] `Control { id: fontGlean }` instantiated per label just to read a font size — ✅ DONE
- **File**: `TrafficLabel.qml:89` (used at 100)
- **Category**: Performance & Quality
- **Confidence**: 85/100
- **Finding**: A full Qt Quick `Control` (style/palette/font/background machinery) is created per label solely to read `fontGlean.font.pixelSize`. It's the heaviest avoidable per-instance object in either file (and the pattern is duplicated in FlightMap.qml).
- **Resolution**: **Fixed.** Removed the per-label `Control { id: fontGlean }` and changed `font.pixelSize: 0.8*fontGlean.font.pixelSize` → `0.8*GlobalSettings.fontSize`. Numerically identical (the window sets `font.pixelSize: GlobalSettings.fontSize` in `main.qml:46`, which is exactly what the probe Control inherited), but removes one Control allocation per label. Added `import akaflieg_freiburg.enroute` to TrafficLabel.qml (needed for the `GlobalSettings` singleton; also a prerequisite for [D-001]).

### [D-005] `description` is `<br>`-only HTML → use `Text.StyledText` — ✅ DONE
- **File**: `TrafficLabel.qml:98`
- **Category**: Performance & Quality
- **Confidence**: 82/100
- **Finding**: The only markup `description` ever contains is `<br>` (the rest is plain text + Unicode arrows). `Text.RichText` spins up the full QTextDocument HTML engine per label; `StyledText` handles `<br>` with a far lighter parser. (`PlainText` would wrongly show literal `<br>`.)
- **Trace**: `TrafficFactor_WithPosition.cpp:118-196` and `TrafficFactor_DistanceOnly.cpp:34-94` both `join("<br>")`.
- **Resolution**: **Fixed.** `textFormat: Text.RichText` → `Text.StyledText`. StyledText renders the `<br>` line breaks (and the Unicode arrows are plain characters), avoiding the full HTML/QTextDocument engine per label. Builds/qmllint clean.

### [D-006] Missing `sourceSize` on a 135×135 data-URI SVG drawn at 40px — ✅ DONE (HiDPI)
- **File**: `Traffic.qml:94-105`
- **Category**: Component Loading / Performance
- **Confidence**: 85/100
- **Finding**: `.icon` is a base64 `data:image/svg+xml` URI; the SVG is natively ~135×135 but displayed at 40px. With no `sourceSize`, each distinct icon rasterizes at ~135px (~73 KB) instead of ~40px (~6 KB). Bounded by the C++ icon cache (~9 shapes × 3 colors) and the fixed 20-item pool, but still wasteful and inconsistent (FlightMap's own/waypoint icons set `sourceSize`).
- **Resolution**: **Fixed, HiDPI-correct.** Added `sourceSize.width/height: 40` (the display size). Confirmed Qt auto-multiplies SVG `sourceSize` by `Screen.devicePixelRatio` — the existing own-position icon (`FlightMap.qml:854-874`) is displayed at its `sourceSize` of 50 and is crisp on HiDPI — so a logical-size `sourceSize` renders at 40×dpr and stays sharp. No manual `devicePixelRatio` math (would double-apply) and no over-allocation to 80. Matches the project convention.

### [D-007] Model-reuse safety is correct but coupled to the CONSTANT model
- **File**: `Traffic.qml` / `TrafficLabel.qml` (Behaviors), `FlightMap.qml:791-799,879-888`
- **Category**: ListView & Delegates
- **Confidence**: 88/100
- **Finding**: The "recycled delegate animates from the previous traffic" glitch **does not occur** — only because `trafficObjects` is a `CONSTANT` fixed list of 20 stable pointers, so `MapItemView` never rebinds `modelData`. Reuse happens in C++ via `replaceBy()`, which clears `animate` *before* the coordinate jump (documented write-ordering in `TrafficFactor_WithPosition.h`), so the `enabled: trafficInfo.animate` gates snap correctly. Latent risk: if the model is ever made dynamic, delegates would recycle and could glide/rotate/fade across traffics.
- **Mitigation**: No change needed. Add a one-line comment on the delegates noting they rely on a fixed CONSTANT list; if it becomes dynamic, snap the Behaviors off for one frame on `trafficInfo` change.

### [D-008] Use `required property` for model data instead of injected `modelData`
- **File**: `FlightMap.qml:796,885` (delegates of these components)
- **Category**: ListView & Delegates
- **Confidence**: 82/100
- **Finding**: `trafficInfo: modelData` relies on soft-deprecated context-property injection. Idiomatic Qt 6 is a `required property` on the delegate.
- **Mitigation**: Declare `required property TrafficFactor_WithPosition modelData` (with `pragma ComponentBehavior: Bound`, see [I-007]) and bind `trafficInfo` to it. (Touches FlightMap.qml, just outside scope, but it's where these are instantiated.)

---

## Investigation targets (human verification needed)

### [I-001] `contentWidth`/`contentHeight` vs `width`/`height` mismatch in the label math
- **File**: `TrafficLabel.qml:94-95` vs `65-66`
- **Confidence**: 72/100
- **Finding**: Centering uses `contentWidth/Height` (excludes padding) while `distFromCenter` uses `width/height` (includes padding) — inconsistent reference box if the style applies padding.
- **How to verify**: Check the active style's Label padding; pick one reference (prefer `width`/`height`, the visible box) for both.

### [I-002] `MapQuickItem.anchorPoint` unset; label centering folded into offsets
- **File**: `TrafficLabel.qml:26` (Traffic.qml:28 is fine)
- **Confidence**: 78/100
- **Finding**: Centering relies on negative child offsets rather than `anchorPoint`. Traffic.qml's approach is clean (correct rotation pivot); TrafficLabel mixes centering + gap + orbit in one expression.
- **How to verify**: Optionally set `anchorPoint: Qt.point(width/2, height/2)` so the label `x`/`y` express only the orbit offset.

### [I-003] `trafficInfo: ({})` default is dead and misleading — ✅ DONE (via [D-001])
- **File**: `TrafficLabel.qml:29`
- **Confidence**: 72/100
- **Finding**: The empty-object default is never live (delegate always binds `modelData`), and chained `.positionInfo.trueTrack()` would actually throw on `{}` — so it's a false safety net.
- **Resolution**: **Fixed** as part of [D-001] — typing the property removed the `({})` default (a typed object property defaults to `null`). Build/qmllint clean.

### [I-004] Missing `readonly` on derived properties
- **File**: `TrafficLabel.qml:47` (`t`), `61` (`distFromCenter`); `trueTrackDEG` in both files
- **Confidence**: 72/100
- **Finding**: Bound-only, never imperatively assigned. `readonly` documents intent.
- **How to verify**: Add `readonly` to `t`/`distFromCenter` (safe); for `trueTrackDEG` confirm it still animates under its `Behavior` (expected fine).

### [I-005] Icon/label "lockstep" can desync at heading wrap
- **File**: `TrafficLabel.qml:47-48` vs `Traffic.qml:59,73`
- **Confidence**: 72/100
- **Finding**: Icon and label each own a *separate* animated `trueTrackDEG` with separate `RotationAnimation`s. If their prior values differ (e.g. one had a non-finite heading), they can animate from different starts / pick different Shortest arcs, breaking the "lockstep" the comment claims.
- **How to verify**: If exact lockstep matters, drive both from a single shared animated source.

### [I-006] 1000 ms animations exactly equal the 1 Hz update period
- **File**: `Traffic.qml:45`, `TrafficLabel.qml:82`
- **Confidence**: 80/100
- **Finding**: Zero slack — if an update arrives late (jitter/GC), the glide finishes and the icon momentarily freezes, then jumps. Restart-mid-flight is otherwise correct.
- **How to verify**: Consider ~1100–1200 ms for the `CoordinateAnimation` to absorb jitter; keep rotation at 1000 ms.

### [I-007] No `pragma ComponentBehavior: Bound`
- **File**: `Traffic.qml`, `TrafficLabel.qml`, `FlightMap.qml`
- **Confidence**: 70/100
- **Finding**: Delegates capture outer ids (`flightMap.bearing`, `flightMap.pixelPer10km`) without the pragma. No functional bug (no recycling), but emits deprecation warnings and blocks compiled bindings.
- **How to verify**: Add the pragma (pairs with [D-008]).

### [I-008] Unused `id: image` in Traffic.qml
- **File**: `Traffic.qml:95`
- **Confidence**: 65/100
- **Finding**: The `image` id is never referenced within the file.
- **How to verify**: Confirm no external `objectName` use, then drop the id.

---

## Summary

| Category | Lint | Deep | Investigate | Total |
|----------|------|------|-------------|-------|
| Bindings & Properties | 1 | 1 | 1 | 3 |
| Layout & Anchoring | — | 1 | 2 | 3 |
| Component Loading & Lifecycle | 1 | 1 | — | 2 |
| ListView & Delegates | — | 2 | 1 | 3 |
| States/Transitions | — | — | 2 | 2 |
| Performance & Quality | 2 | 3 | 1 | 6 |
| Imports/Style/Ordering | 4 | — | 1 | 5 |
| **Total** | **8** | **8** | **8** | **24** |

Findings below confidence 60 suppressed. The JS-2 lint flags and the `QtQuick.Controls` IMP-3 flag are confirmed false-positives/won't-fix.

**Top picks**: [D-002] the label-gap geometry (a real visual bug at off-axis headings), [D-001] typing `trafficInfo` (unlocks compiled bindings + subsumes I-003), and the per-instance perf trio [D-003]/[D-004]/[D-006] (cache the `trueTrack()` calls, drop the `fontGlean` Control, pin `sourceSize`).
