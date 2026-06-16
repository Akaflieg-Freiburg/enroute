# Design: Météo-France weather client

## Purpose

Display rain and cloud-base forecast overlays on the moving map, sourced from a
local **meteo-map-server** (a Raspberry-Pi pipeline that renders AROME forecast
PNGs). The client fetches a manifest and the PNG tiles, caches them, and shows
them as MapLibre image layers with a time slider and colour-scale legends.

> Wind was originally part of this (a server-rendered PNG layer) but has been
> replaced by the client-drawn vector field — see [wind-field.md](wind-field.md).
> `ForecastMapProvider` now serves **rain + cloud-base only**.

## Data flow

```
meteo-map-server  ──HTTP──►  ForecastMapProvider (C++)  ──►  FlightMap.qml (MapLibre image layers)
  index.json                  fetch + cache + scan             MFM.qml (layer menu: toggles,
  rain_map_*.png              parse metadata                     time slider, legends, refresh)
  cloudbase_map_*.png
```

## Key files

| File | Role |
|------|------|
| `src/weather/ForecastMapProvider.{h,cpp}` | Singleton: fetch `index.json`, download/cache PNGs, expose current map paths + layer metadata. |
| `src/qml/items/FlightMap.qml` | MapLibre `image` SourceParameters + raster LayerParameters for rain/cloudbase, plus the `ofmDimmer` background layer. |
| `src/qml/items/MFM.qml` | Layer menu: per-layer checkboxes, the shared forecast time slider, colour-scale legends, refresh button, AROME attribution. |
| `src/qml/items/ColorScaleLegend.qml` | Gradient bar + tick labels driven by server colour stops. |
| `src/qml/pages/SettingsPage.qml` | "Forecast Maps" server-URL field. |

## Server contract

- **`index.json`** at `<serverUrl>/index.json`:
  - `files`: list of PNG filenames available.
  - `reference_time`: the model run (ISO-8601 `…Z`). **When this changes, the
    client wipes its PNG cache** so a new run replaces stale tiles
    (`fetchIndex()` in ForecastMapProvider.cpp).
  - `layers`: per-layer `{units, vmin, vmax, colors[]}` (Qt `#AARRGGBB`), used to
    render the legends client-side so server and client always agree.
- **PNG naming** (parsed by regex in `scan()`):
  - `rain_map_<ISO>.png`, `cloudbase_map_<ISO>.png`. The `<ISO>` timestamp is the
    forecast valid time; the union of timestamps drives the time slider.
- **Geo-referencing** is a fixed bounding box (10°W–15°E, 38°N–55°N) baked into
  the `image` SourceParameter coordinates in FlightMap.qml.

## Caching & sync

- PNGs cached under `CacheLocation/meteo_france/`. Atomic write (`.tmp`→rename).
- `refresh()` supports both HTTP and a `file://` server URL (local dev/testing).
- On a successful full refresh, cache entries no longer on the server are purged.
- `lastRefreshLabel` / `status` surface sync state to the menu.

## Testing

Mostly integration/visual, but a few seams are unit-testable:

- **Filename parsing**: `scan()` regexes accept the documented names (with/without
  `Z`) and reject others. Feed a temp dir of fixture filenames → assert the
  resulting timestamp set.
- **Cache invalidation**: given a cache populated under run A, a refresh whose
  `index.json` carries run B empties the PNG cache before downloading.
- **Metadata parsing**: a known `index.json` → expected `rainColors/vmin/vmax`,
  `referenceTimeLabel`.
- **Contract**: keep a committed sample `index.json`; assert the server pipeline
  produces that shape and the client parses it (see wind-field.md for the
  cross-repo contract-test pattern).
- Manual checklist items: layer toggles, time-slider scrubbing, legend correctness,
  OFM dimming, refresh.
