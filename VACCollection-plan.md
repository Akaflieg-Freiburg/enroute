# Server-Distributed VAC Collections

## Context

Enroute distributes per-country aviation/base/terrain maps via a JSON catalog (`maps.json` on enroute-data.akaflieg-freiburg.de); VAC charts are currently only user-imported (single images or TripKit zips). Goal: distribute per-country VAC collections (e.g. France) from the data server so users receive and update them together with their country maps — with zero behavior change for old app versions and no new download mechanism.

**Format decision (user-approved): SQLite container**, one file per country (`Europe/France.vac`), produced server-side from the existing TripKits (images pre-encoded to webp on the server — no slow on-device re-encoding). Charts are read in place from the container; only the currently-viewed chart is extracted to cache. Rationale over TripKit-as-is: atomic updates, no doubled storage, fast index reads, consistent with the app's existing sqlite/mbtiles handling.

**Key insight that makes delivery free:** map sets (`Downloadable_MultiFile`, MultiUpdate policy) already treat a *missing* sibling as an available update ([Downloadable_MultiFile.cpp:476-503](src/dataManagement/Downloadable_MultiFile.cpp#L476-L503)) and `update()` calls `startDownload()` on it ([Downloadable_MultiFile.cpp:330-356](src/dataManagement/Downloadable_MultiFile.cpp#L330-L356)). So once `Europe/France.vac` joins the "France" map set, users with France installed get it via the existing "updates available" notification (one tap), and new installs include it. **Backward compat verified:** old versions create an inert, invisible `Downloadable` for unknown suffixes (joins no map set, no visible group; `update()` never downloads files not already present locally).

**Other user decisions:** name collisions with manually imported VACs → show both, distinguished by info text; no opt-out setting.

## Container format spec (for enrouteServer; conversion script lives there, out of scope)

```sql
CREATE TABLE metadata (key TEXT UNIQUE NOT NULL, value TEXT);
-- required: ('schemaVersion','1'), ('name','France')
-- optional: ('description',...), ('attribution',...), ('publicationDate','yyyyMMdd')
CREATE TABLE charts (
  name TEXT PRIMARY KEY,               -- e.g. "LFGA COLMAR HOUSSEN"
  topLeftLat REAL NOT NULL,    topLeftLon REAL NOT NULL,
  topRightLat REAL NOT NULL,   topRightLon REAL NOT NULL,
  bottomLeftLat REAL NOT NULL, bottomLeftLon REAL NOT NULL,
  bottomRightLat REAL NOT NULL,bottomRightLon REAL NOT NULL,
  image BLOB NOT NULL                  -- webp-encoded raster
);
```

WGS84 degrees; corners match the MapLibre image-source quad in [FlightMap.qml:136-149](src/qml/items/FlightMap.qml#L136-L149). Catalog entry in existing maps.json: `{"path": "Europe/France.vac", "time": "yyyyMMdd", "size": <bytes>, "bbox": [...]}`. Regenerate + `VACUUM` the whole file per update (no incremental writes) so `time`/`size` semantics match other maps.

## Implementation steps

### 1. New reader `src/fileFormats/VACCollection.{h,cpp}`

Subclass of `FileFormats::DataFileAbstract`; QSqlDatabase handling copied from [MBTILES.cpp](src/fileFormats/MBTILES.cpp) (unique connection name via QRandomGenerator, `removeDatabase` in dtor, `openFileURL` for the file).

- Ctor: open DB → validate `metadata.schemaVersion == '1'` (also rejects non-sqlite garbage) → read `name` → read chart index **without the image column** (`SELECT name, topLeftLat, … FROM charts`; never touch blob pages on index read). Build `QList<GeoMaps::VAC>` with `fileName` = container path, `collection` = collection name; skip rows with invalid coordinates.
- `QByteArray imageData(const QString& chartName)` — prepared/bound query returning the webp blob.
- Add both files to `SOURCES` in `src/CMakeLists.txt` (next to `fileFormats/TripKit.*`).

### 2. Downloadable plumbing (`src/dataManagement/`)

- [Downloadable_Abstract.h:50](src/dataManagement/Downloadable_Abstract.h#L50): append enum value `VACCollection` at the end (don't reorder — QML compares numeric values). A separate value from `VAC` is needed: [MapSet.qml:130](src/qml/items/MapSet.qml#L130) keys Rename on `VAC`, and MultiFile labels are per-type.
- [Downloadable_SingleFile.cpp:57-87](src/dataManagement/Downloadable_SingleFile.cpp#L57-L87): `else if (tmpName.endsWith(u".vac"_s)) setContentType(VACCollection);` (with the dot).
- [Downloadable_MultiFile.cpp](src/dataManagement/Downloadable_MultiFile.cpp) `description()` (~line 62) and `infoText()` (~line 106) switches: add `case VACCollection:` → `tr("Approach Charts")`. The switches have no `default`, so `-Wswitch` finds every site. Fix the pre-existing typo at line 127 ("Visual Approach ChartTerrain Map" → "Visual Approach Chart") in passing.
- [DataManager.cpp](src/dataManagement/DataManager.cpp):
  - `cleanDataDirectory()` (lines 93-97): whitelist `.vac`. **Must land in the same commit** — otherwise every start deletes downloaded collections.
  - `createOrRecycleItem()` (line 286): add `.vac` to the map-set-joining condition (so `Europe/France.vac` joins the France set via objectName+section match) and route into a new group: `if (localFileName.endsWith(u".vac"_s)) m_vacCollections.add(downloadable);`
- [DataManager.h](src/dataManagement/DataManager.h): new member `Downloadable_MultiFile m_vacCollections` + `Q_PROPERTY(... vacCollections READ vacCollections CONSTANT)`. Do **not** add it to `m_mapsAndData` — the items are already inside map sets; adding the group would double-count update sizes.

### 3. `GeoMaps::VAC` value type ([VAC.h](src/geomaps/VAC.h)/[VAC.cpp](src/geomaps/VAC.cpp))

- New member + property `QString collection;` (empty = manually imported).
- `infoText()`: collection charts → `tr("%1 chart collection").arg(collection)`; else unchanged.
- Optional: read-only `section` property (`tr("Manually Imported")` vs collection name) so VAC lists' existing `section.property: "section"` starts grouping.
- **Leave the QDataStream operators untouched** — collection charts are never persisted; old `VAC.data` files stay readable, no version bump.

### 4. `GeoMaps::VACLibrary` ([VACLibrary.h](src/geomaps/VACLibrary.h)/[VACLibrary.cpp](src/geomaps/VACLibrary.cpp))

New state: `QVector<VAC> m_collectionVacs` (never saved to `VAC.data`) and cache dir `QStandardPaths::CacheLocation + "/VAC"`.

- **Wiring**: in the ctor's deferred singleShot (pattern already used for `janitor()`, [VACLibrary.cpp:53](src/geomaps/VACLibrary.cpp#L53)): connect `GlobalObject::dataManager()->vacCollections()` `filesChanged` + `fileContentChanged` → `updateCollections()`, then call it once (DataManager may already be populated). GlobalObject lazy allocation makes this safe; VACLibrary is not itself a GlobalObject.
- **`updateCollections()`** (slot, coalesced via a 0 ms single-shot compressing timer — `filesChanged` fires once per sibling during bulk updates): rebuild `m_collectionVacs` from `vacCollections()->files()` via `FileFormats::VACCollection` (skip corrupt files with qWarning); cache hygiene (drop cache subdirs of uninstalled collections; drop cached files whose mtime token mismatches the container); `emit dataChanged()`.
- **`Q_INVOKABLE VAC materialize(const VAC&)`** — the on-demand extraction hook. User charts pass through. Collection charts: extract `imageData()` via QSaveFile to `cache/<containerBaseName>/<sanitizedName>-<containerMtimeSecs>.webp` if absent; return a copy with `fileName` = cache path. The mtime token in the filename doubles as a MapLibre cache-buster after collection updates. On failure return input unchanged.
- Merged views: `vacs()` (sort a merged copy, stop sorting `m_vacs` in place), `vacsByDistance()`, `vacs4Point()`, `isEmpty()` (`m_vacs` and `m_collectionVacs`). `get(name)`: user chart wins, **materialize before returning** ([main.qml:783](src/qml/main.qml#L783) assigns the result to `Global.currentVAC`).
- Guards: `remove()`/`rename()`/`clear()` operate on `m_vacs` only; `rename()` refuses collection charts. New `Q_PROPERTY(bool hasManuallyImported ...)` for the "Clear VAC library" menu.
- `janitor()`: no functional change needed (it only touches `<AppData>/VAC` and `m_vacs`). **Pre-existing bug found at [VACLibrary.cpp:376](src/geomaps/VACLibrary.cpp#L376)**: stray-file loop iterates `imageFilesWithVAC` instead of `imageFilesWithoutVAC` — recommend fixing in passing (one-word change, but touches user data; flag in the PR).

### 5. QML / UI

- [VAC.qml:54](src/qml/pages/VAC.qml#L54): `Global.currentVAC = VACLibrary.materialize(model.modelData)`; delegate text gains an infoText second line for provenance (mirroring DataManagerPage's vacDelegate).
- [WaypointDescription.qml:430](src/qml/dialogs/WaypointDescription.qml#L430): wrap in `VACLibrary.materialize(...)`.
- [DataManagerPage.qml](src/qml/pages/DataManagerPage.qml) VAC tab: show collection charts **read-only** (users must be able to find server-delivered charts here; management stays in the Maps tab): `renameAction`/`removeAction` `enabled: element.model.modelData.collection === ""`; "Clear VAC library…" `enabled: VACLibrary.hasManuallyImported`, dialog text notes collections are removed via the Maps tab.
- [MapSet.qml](src/qml/items/MapSet.qml): no change required — the `.vac` member surfaces via the set's infoText once the switch cases exist; icon falls through to `ic_map.svg`.
- [MapPage.qml](src/qml/pages/MapPage.qml): add a `Connections { target: VACLibrary; onDataChanged: ... }` that resets `Global.currentVAC = Global.defaultVAC` when the displayed chart no longer resolves — the extracted cache file can outlive an uninstalled collection, so `isValid` alone won't catch it.
- [main.qml:174](src/qml/main.qml#L174) menu visibility: works unchanged via the updated `VACLibrary.isEmpty`.

## Edge cases

- Map-set delete → `filesChanged` → charts drop, cache dir wiped, `currentVAC` reset; empty `aviation_maps` subdirs cleaned on next start.
- Corrupt/truncated `.vac`: schema validation fails → skipped with warning (downloads themselves are atomic via QSaveFile).
- User already imported the France TripKit: duplicates appear with distinct info texts (accepted decision); `operator==` (defaulted, includes `collection`) and `get()` preference keep them distinct.
- Old app versions with the new catalog: verified inert (no map-set join, no visible group, `update()` skips items without local files).

## Verification

```bash
QTDIR=$(find ~/Software/buildsystems/Qt -maxdepth 2 -type d \( -name gcc_64 -o -name macos \) | sort -V | tail -1)
cmake -B build/claude -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH="$QTDIR"
cmake --build build/claude && cmake --build build/claude --target all_qmllint
```

Manual end-to-end test — no server needed: `updateDataItemListAndWhatsNew()` creates items for any file found in the data directory ([DataManager.cpp:469-474](src/dataManagement/DataManager.cpp#L469-L474)), so a hand-placed `.vac` flows through the whole pipeline:

1. Build a test container in the scratchpad with the sqlite3 CLI (`readfile()` is built in), using any small webp and plausible Colmar-area corners (schema above).
2. Copy to `~/.local/share/Akaflieg Freiburg/enroute flight navigation/aviation_maps/Europe/France.vac`.
3. Launch the app (**ask the user first** — per CLAUDE.md it opens a real window on their desktop). Check: main menu shows Approach Charts; DataManager VAC tab lists the chart with "France chart collection" info text and disabled Rename/Uninstall; selecting it displays the image on the moving map; extracted file appears under `~/.cache/Akaflieg Freiburg/enroute flight navigation/VAC/France/`; deleting `France.vac` makes the chart disappear and resets the map view.
4. Restart once: `cleanDataDirectory()` must not delete the file; `VAC.data` must contain no collection charts.

## Suggested order

1. VACCollection reader + CMakeLists (compiles standalone)
2. ContentType enum + suffix mappings + MultiFile switch cases (`-Wswitch` guides)
3. DataManager whitelist/grouping/`m_vacCollections`
4. VAC `collection` member + infoText
5. VACLibrary merge/materialize/guards
6. QML changes
7. Build, qmllint, manual test
