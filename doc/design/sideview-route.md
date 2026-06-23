# Design: Route side view

## Purpose

The side view is a vertical cross-section beneath the moving map. It has two
modes:

- **Track** — terrain/airspaces ahead along the current ground track.
- **Route** — terrain, airspaces, and the **planned altitude profile** along the
  whole flight route, plus (optionally) **projected wind barbs**.

## Architecture

`SideviewQuickItem` (C++, `QQuickItem`) computes geometry in **pixel space** and
exposes it as polygons/point-lists; `Sideview.qml` renders them with `Shape`,
`Repeater`s, and a `Canvas` overlay. Heavy recomputation is rate-limited to once
per 700 ms (`minimumUpdateInterval_ms`).

### Coordinate mapping (Route mode)

- **x** = distance along the route: `cumulativeDist[i] / totalRouteLen * renderW`.
  `renderW` is the Flickable **content** width (so x is in scrollable content
  space, matching the QML overlays).
- **y** = altitude: `altToY(alt)` maps `[scaleMinAltFt, scaleMaxAltFt]` (derived
  from the planned profile ± margin) to the pixel height.
- `coordAtDist(distM)` returns the `QGeoCoordinate` at a distance along the
  great-circle polyline — the key helper for sampling terrain/wind anywhere.

## Key files

| File | Role |
|------|------|
| `src/ui/SideviewQuickItem.{h,cpp}` | All geometry: terrain polygon, airspace polygons, planned profile, scale ranges, wind profile. `updatePropertiesRoute()` / `updatePropertiesTrack()`. |
| `src/qml/items/Sideview.qml` | Rendering: terrain/airspace `ShapePath`s, Y/X scale panels, altitude markers, ownship, mode toggle, **wind barb Canvas overlay**. |
| `src/navigation/FlightRoute.{h,cpp}` | `plannedAltitude(idx)` (per-waypoint planned altitude, persisted), `waypointETAs(...)`. |

## Key properties (C++ → QML)

- `mode` (Track/Route), `terrain` (QPolygonF), `airspaces` (QVariantMap of
  polygon vectors by category).
- `plannedProfile` (QPolygonF) + `plannedProfilePoints` (clickable altitude
  markers). Altitudes resolve as: stored `plannedAltitude` → aircraft cruise →
  terrain+1000 ft.
- `scaleMinAltFt` / `scaleMaxAltFt` / `scaleTotalDistKm` for the axis panels.
- `windProfile` + `showWind` — see below.

## Wind projection (Route mode)

When `showWind` is on (bound to the global `GlobalSettings.showWindLayer`), the
view samples the wind field on a **distance × altitude grid** (up to 14 columns ×
5 rows across the visible Y range) and exposes `windProfile`: for each sample
`{x, y, speedKn, dirFromDeg, alongKn}`.

- `alongKn` is the **along-track component** (+ = headwind, − = tailwind),
  computed as `speed · cos(windFromDir − trackBearing)`.
- **ETA-aware time**: each column's sampling time is the interpolated arrival
  time at that distance, from `FlightRoute::waypointETAs(...)` starting at
  `Navigator.departureTime` — so wind 1 h into the flight uses the +1 h forecast.
  (Spatial + altitude + time are all interpolated here; see wind-field.md.)
- `Sideview.qml` draws each sample as a horizontal barb in one `Canvas` overlay:
  **red = headwind, green = tailwind**, feathers encode `|alongKn|`.

Recomputation is triggered by route/geo-path changes, planned-altitude changes,
`WindFieldProvider::dataChanged`, and `Navigator::departureTimeChanged`.

## Testing

- **`coordAtDist`**: endpoints (0, totalLen) return the route endpoints; midpoint
  of a single straight leg returns the geographic midpoint.
- **`altToY` monotonicity**: higher altitude → smaller y; scaleMin/Max map to the
  view's bottom/top.
- **Planned-altitude resolution**: stored value used when present, else aircraft
  cruise, else terrain+1000 ft.
- **Along-track projection**: pure headwind (windFrom = track) → `alongKn = +W`;
  tailwind → `−W`; pure crosswind → `≈ 0`.
- **ETA monotonicity** of the column times along the route (delegates to
  `waypointETAs`, tested in wind-field.md).
- Manual: profile shape, altitude markers/editor, mode toggle, barb colours and
  density, scrolling.
