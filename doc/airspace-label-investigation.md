# Missing airspace labels on shared boundaries

Investigation record, 2026-11-03. No code was changed.

## Symptom

In the moving map, some airspace boundaries carry no label. Example: TMA Stuttgart,
south of Stuttgart (Tübingen / Reutlingen area, "Lufträume bis 9.000 ft"). The
southern boundaries of the sectors are labeled, the northern boundaries are not.

## Root cause in one paragraph

Airspace labels are placed along polygon rings with `symbol-placement: line`
(layer `AirspaceLabels`, `src/qml/items/FlightMap.qml`). Where several airspaces
share one boundary segment, every one of them tries to label the same line.
MapLibre builds the collision footprint of a line-placed label as a chain of
circles centred on the line itself; the layer's `text-offset: [0, 1]` only moves
the drawn glyphs, not the circles. Labels are longer than the gap between
neighbouring anchors, so the second feature never fits. Placement runs in GeoJSON
feature order, and the data lists sectors by ascending floor, so the inner
(lower) sector wins and the outer sector's inner boundary stays unlabeled.

## Evidence

### Geometry (Germany.geojson, feature indices from today's file)

| # | MLI | role on the screenshot |
|---|---|---|
| 4580 | E • 1000 AGL - 2500 AGL | north of the top line, ring runs SW→NE |
| 4581 | C • 3500 - 5500 | north of the top line, same footprint as 4583 |
| 4582 | D • 4500 - FL 100 | between top and middle line |
| 4583 | D • 5500 - FL 100 | north of the top line, same footprint as 4581 |
| 4584 | D • 5500 - FL 100 | big western sector, shares the 48.413 N line and the western arc |
| 4586 | D • FL 75 - FL 100 | south of the middle line |

Shared straight edges in view:

- Top line, 48.4953 N 9.0056 E → 48.5747 N 9.4222 E (bearing 74°): shared by
  4580, 4581, 4583 (all SW→NE) and 4582 (NE→SW). Only the E label survives.
- Middle line, 48.4147 N 9.0153 E → 48.5625 N 9.6256 E: shared by 4582 (SW→NE)
  and 4586 (NE→SW). Only "D • 4500 - FL 100" survives.
- Bottom line "D • FL 75 - FL 100": not shared with a visible sector, labeled.
- Western arc near Bodelshausen: shared by 4582 and 4584, only "D • 5500" survives.

All rings in the file are counter-clockwise and closed. All six sectors pass the
altitude filter (SBO 1000 … 7500 ft, limit 9000 ft).

### Rendering (pixel sampling across the lines in the screenshot)

- Bands (`ABCDs` layer, `line-offset: 3.5`) are inset on both sides of every shared
  edge. The north band of the top line is twice as dark because C and D 5500 overlap.
- Every surviving label sits inside its own polygon (north of the top, middle and
  bottom lines; south-west of the western arc). Label side and ring orientation are
  therefore not the problem.

### MapLibre Native code (sources now live under `src/mln/`, not `src/mbgl/`)

- `text/collision_feature.cpp`, `CollisionFeature::initialize` and `bboxifyLabel`:
  for `alongLine` labels the shaped text's top/bottom only set the circle size;
  the circles are interpolated along the line with no perpendicular offset.
- `layout/symbol_projection.cpp`, `placeGlyphAlongLine`: the perpendicular
  `lineOffsetY` is applied to the glyph position only. Net direction is
  `perp(line direction) * offset`, independent of the upright flip, which is why
  labels follow the ring orientation and land inside their polygon.
- `text/get_anchors.cpp`, `getAnchors`: anchors every `symbol-spacing` (250 px
  default); if `spacing - labelLength < spacing/4`, spacing becomes
  `labelLength + spacing/4`, so the free gap is at least 62 px and at most 100 px.
- `text/placement.cpp`, `getBucketSymbols`: symbols are tested in bucket (source)
  order unless `symbol-z-order` is explicitly `viewport-y` or a `symbol-sort-key`
  is set.

Measured on the screenshot: "D • 4500 - FL 100" is about 150 px long,
"E • 1000 AGL - 2500 AGL" about 240 px, the collision circle diameter about 18 px
(text-size 0.85 × 14 px plus padding). A 150 px label cannot fit a 100 px gap.

Upstream: mapbox/mapbox-gl-js issue 4798, "Respect text-offset in line-placed
symbol collision handling", open since 2017-06-07. MapLibre inherited the behaviour.

### Why the winner is always the inner sector

`enrouteServer/scripts/generateWorldAviationMap.py` concatenates OFMX and openAIP
features without re-sorting; the Stuttgart block arrives ordered by ascending floor
(E 1000 AGL, C 3500, D 4500, D 5500, D 5500, D FL 75). Lower floor means nearer the
airport, so the inner sector is placed first and wins every shared edge.
Consequence: C 3500-5500 and D 5500-FL 100 (4581, 4583) have identical footprints,
so the D 5500 label of 4583 can never appear anywhere on the map.

### Ruled out

- `text-max-angle` at corners: the shared edges are 32 to 48 km straight lines with
  room for six or more anchors each.
- Tile clipping / `anchorInsideTile`: would thin labels out, not remove them along
  the whole edge.
- Altitude filter, ring winding, label side: see above.

## Remedies, ranked

1. **Interior labels (recommended first step).** Add a second symbol layer on the
   `aviation-data` source with `symbol-placement: point`. For polygon features
   MapLibre places one label per polygon per tile at the pole of inaccessibility
   (`mapbox::polylabel` in `layout/symbol_layout.cpp`). `text-variable-anchor`
   lets the stacked C / D 5500 pair get two labels. Pure style change in
   `FlightMap.qml`. Drawbacks: label sits in the middle of the band rather than at
   the boundary, and relocates when the zoom level changes.
2. **Server-side inset label lines.** enrouteServer already uses shapely
   (`scripts/openAIP2.py`). Emit each airspace boundary as a separate LineString
   shrunk inward with a negative buffer, carrying MLI/MLM, and point the label layer
   at those. Neighbouring labels then live on distinct geometry. The separation
   must exceed the 18 px collision diameter, so a 400 m inset only helps from about
   zoom 12 upwards. Needs a coordinated data-format change in server and app.
3. **Pick the winner deliberately** with `symbol-sort-key` (for example prefer the
   higher floor). Cheap, but only moves the gap; still one label per shared edge.
4. **`text-allow-overlap: true`** restores every label but overprints wherever
   several sectors share one side; on the top line three labels would stack.
   Not usable on its own.
5. **Patch MapLibre** to shift the collision circles by the text offset. The
   QMapLibre used here is home-compiled from `../enrouteDependencies`, so a local
   patch is feasible; see "Upstream status" below for size and shape.

## Suggested confirmation experiment

Temporarily set `text-allow-overlap: true` on the `AirspaceLabels` layer and look at
the top line: E, C and D 5500 should appear stacked north of the line, D 4500 south
of it. This has not been run yet.

## Helper scripts used (in /tmp/claude-asp, not kept)

- `inspect.py`: list airspace polygons in a lat/lon window with orientation and
  properties.
- `edges.py`: merge collinear runs into long edges and report edges shared between
  polygons.
- Pixel profiles across a line: sample RGB along the normal of a line at a given
  screen x to tell band and label sides apart (eyeballing the crops was unreliable).

## Upstream status (checked 2026-11-03)

Which MapLibre is actually in use: `enrouteDependencies/maplibre-native-qt-flat`
(maplibre-native-qt v3.0.0-13-g45ab7f6, 2025-06-08) with vendored maplibre-native
commit b50faeb9 (2025-05-01, "node-v5.2.0-104"). The copy under
`~/Software/buildsystems/Qt/6.12.0/gcc_64` is what CMake finds, but it is installed
from that build, not from the Qt installer.

**MapLibre GL JS: fixed since 3.0.0 (pre.5, March 2023).** PR
maplibre/maplibre-gl-js#2260 "Improve line labels using text-offset" closed issues
#2170 (gaps/bunching at vertices) and #2171 (collision boxes ignore the offset, so
two labels on opposite sides of one line collide). GL JS now offsets the line
geometry itself (`transformToOffsetNormal`, `findOffsetIntersectionPoint` in
`src/symbol/projection.ts`) and generates its collision circles dynamically along
the path returned by `placeFirstAndLastGlyph`, so the circles follow the text.
Render tests `text-offset/line-collision`, `line-concave`, `line-convex`,
`line-sharp-*` were added with the PR. Size: projection.ts +231/-47, util.ts +33,
collision_index.ts one line.

**MapLibre Native: not fixed in any version, including main as of today.** Both
the vendored commit and upstream main (`src/mln/text/collision_feature.cpp`,
`collision_index.cpp`) still precompute the circles on the line in
`bboxifyLabel` and project `circle.anchor` in `placeLineFeature`; the first/last
glyph positions (which do carry the offset) are only used to decide which circles
are active. The last functional change to these files was 2022-07-05; since then
only renames and clang-tidy. The parity tracking issue maplibre-native#2445 lists
all `text-offset/line-*` render tests as failing, so the gap is known but nobody
has ported it. Updating QMapLibre or maplibre-native therefore does not help.

**Shape of a local patch** (not written yet): `SymbolInstance` already stores
`textOffset` (`src/mbgl/layout/symbol_instance.hpp`) and builds
`textCollisionFeature` in its initializer list. Pass the y offset into
`CollisionFeature` → `initialize` → `bboxifyLabel`, convert it to tile units with
`boxScale`, and shift each `boxAnchor` by the unit normal of its segment
(`perp(p1 - p0)`), with the same sign as `placeGlyphAlongLine` uses
(`perp(lineDir) * lineOffsetY`). Roughly 15 lines plus the vendored-copy rebuild in
enrouteDependencies for every platform. Limits: ignores overscaling (as the
existing circle size does) and does not fix the vertex gaps of #2170; a full port
of the GL JS approach would also need dynamic circle generation along the
projected path, which native lacks entirely.
