# Enroute — Weather Layer Changes (feature/weather-layer)

## Foundation

- **Weather layer QML item**: METAR flight-category dots and wind barbs on the map
- **ForecastMapProvider**: C++ singleton that downloads forecast PNGs from a local HTTP server, scans the cache, and exposes current map paths to QML
- **SettingsPage**: Forecast Maps section with server URL field and refresh button
- **Filename regex**: optional Z suffix support in forecast map filename matching

## METAR wind barbs

- **Direction fix**: removed erroneous `+Math.PI` inversion; moved dot anchor from (14,14) to (40,40) in 80×80 canvas so all directions fit
- **Reactive repaint**: switched from `onMetarChanged` to `onMetChanged` to match `QProperty`-based binding

## Weather menu (MFM.qml)

- **Color scale legends** (`ColorScaleLegend.qml`): gradient bar with 4 labeled tick marks, matching server-side color stops; shown for rain and cloudbase layers
- **Layer metadata from index.json**: colors, units, vmin/vmax read from server and reflected in client legends
- **Model run attribution**: "AROME dd MMM HH:mmZ" + "© Météo-France" replacing generic "Model run" label
- **Wind level slider**: replaced pressure radio buttons with a slider; levels labeled in approximate FL (FL000/FL033/FL065/FL100)
- **Cloudbase units**: displayed in aircraft's vertical distance unit (ft or m)
- **Refresh button**: moved into weather menu with "last refreshed" label
- **Layout fixes**: explicit `implicitWidth: 280` on custom `ItemDelegate`s to prevent `AutoSizingMenu` from collapsing

## Settings layout

- **URL TextField**: `Layout.preferredWidth: 0` to prevent column inflation in `GridLayout`

## Map layer ordering and dimming

- **OFM dimming**: `background` LayerParameter between OFM tiles and forecast layers dims the base chart to 68% white when wind layer is active
- **Layer order**: cloudbase → rain → wind (wind on top, rain above cloudbase)
- **NOTAM/waypoint hiding**: `layout.visibility` set to `none` when wind layer active (symbol layers cannot be dimmed via MapLibre stack position)

## Local file:// testing mode

- `ForecastMapProvider` detects `file://` server URL, reads `index.json` from local directory, scans PNGs directly — no network required for development

## Cache management

- **Cache invalidation**: when `reference_time` in `index.json` changes (new model run), all cached PNGs are wiped before downloading the fresh set — prevents serving stale maps from an older run
