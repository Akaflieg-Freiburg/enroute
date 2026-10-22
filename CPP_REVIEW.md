# C++ code review

Reviewed commit: `37d94f1a83796448c16b2d0f7a4bc583dd258f8f`.

## Scope and outcome

Targeted correctness review of first-party C++ under `src/`, covering traffic reception, file/chart import, download handling, navigation and positioning, service lifetime, item models, and flight-log detection/storage. This is not an exhaustive audit of every C++ file or third-party dependency. No application source was changed.

Six actionable findings are listed below, ordered by priority. **P1** means high priority (crash, loss of live data, or destructive failure); **P2** means a functional defect that should be fixed. Flight-log findings concern the work-in-progress feature, whose UI is disabled by default.

## Findings

### 1. P1 — Validate decoded GDL90 length before checking the CRC

**Location:** `src/traffic/TrafficDataSource_Abstract_GDL90.cpp:185–235`, especially lines 224 and 231–232. Entry path: `src/traffic/TrafficDataSource_Udp.cpp:119–125`.

The minimum-length check applies to the raw, escaped input. Unescaping can shrink that input below the two CRC bytes plus message ID required by the subsequent code. There is no second length check before `message.chopped(2)` and `message.at(message.size()-2)`.

For example, raw bytes `7d 7d 00` pass the initial three-byte check. The decoder treats both `7d` bytes as escape prefixes and produces just one byte, `20`, with `isEscaped == false`. The CRC code then calls `chopped(2)` on a one-byte array and would also index at `-1`. A UDP datagram `7e 7d 7d 00 7e` reaches this path without a valid CRC ever being required.

**Impact:** malformed receiver/network input can abort a debug build or cause undefined behavior in a release build, interrupting navigation.

**Validation:** compiled the unchanged decode/CRC prefix extracted from the production function against the installed Qt, with assertions enabled. Input `7d7d00` terminated with SIGABRT and the QByteArray assertion `n <= d.size - pos`.

**Recommendation:** reject decoded messages shorter than three bytes before any CRC slicing/indexing. Also reject malformed escape sequences explicitly. Add regression cases for repeated escapes, trailing escapes, and short decoded messages.

### 2. P1 — UDP deduplication suppresses legitimate recurring reports

**Location:** `src/traffic/TrafficDataSource_Udp.cpp:100–109`; cache definition in `src/traffic/TrafficDataSource_Udp.h:138–146`.

The receiver remembers the last 512 accepted datagram hashes without an expiration time. An identical datagram is dropped before parsing, and dropped datagrams do not advance the ring. The cache also survives disconnect/reconnect.

This is not limited to copies delivered simultaneously on multiple interfaces. Valid recurring packets can be byte-identical: for example, `XGPS,7.8,48.0,200,0,0` from a stationary source. XGPS has no timestamp field; its receive time is assigned during parsing. After the first packet, all identical reports can be discarded indefinitely when there are insufficient distinct intervening packets.

**Impact:** fresh data is treated as stale. In the XGPS path, heartbeat refresh stops and expires after five seconds; position also eventually expires (20 seconds on mobile). See `TrafficDataSource_Abstract_XGPS.cpp:65–72`, `TrafficDataSource_Abstract.cpp:32–45`, and `positioning/PositionInfo.h:189–193`. Reconnecting does not clear the offending cache entry.

**Validation:** compiled the exact production hash-filter block and supplied 600 identical valid XGPS reports. Only **1 of 600** was accepted.

**Recommendation:** restrict deduplication to a short, timestamped duplicate-delivery window, optionally incorporating sender/interface information. Clear the cache on reconnect. Verify that same-batch duplicates are suppressed while legitimate periodic reports continue refreshing data and heartbeat.

### 3. P1 — Replacing a chart deletes the working copy before the replacement succeeds

**Location:** `src/geomaps/VACLibrary.cpp:258–287`; deletion implementation at `src/geomaps/VACLibrary.cpp:348–371`.

`importVAC()` calls `remove(vac.name)` before encoding/reading the replacement and before `saveFileAtomically()`. `remove()` deletes the existing chart image and removes its library entry. Several subsequent operations can still fail, including WebP encoding, reopening the input, and writing/committing the new file.

**Impact:** attempting to update an installed chart can destroy the old usable chart when the replacement fails, for example because storage is full. The use of QSaveFile inside `saveFileAtomically()` does not preserve the original because it has already been deleted.

**Validation:** traced the import, remove, and atomic-write error paths. No disk-full fault injection was performed.

**Recommendation:** prepare and atomically commit the replacement before changing the library or deleting any previous file. If the old and new paths differ, delete the old image only after successful replacement. A regression test should force replacement failure and verify that the old image and library entry remain intact.

### 4. P2 — Plain ZIP archives of coordinate-named charts are rejected

**Location:** `src/fileFormats/TripKit.cpp:215–223`; validity check at `src/geomaps/VAC.cpp:118–122`.

The fallback `readVACs()` constructs `GeoMaps::VAC(path, {})` using an archive entry name as though it were a filesystem path. It then calls `vac.isValid()`, which requires `QFile::exists(fileName)`. Normally the entry exists only inside the ZIP, so every chart is rejected. A coincidentally matching file in the working directory can even change the result.

**Impact:** the coordinate-named image ZIP format explicitly documented in `src/fileFormats/TripKit.h:42–45` cannot normally be imported; the error misleadingly complains about missing `toc.json`.

**Validation:** compiled the actual TripKit/ZipFile/VAC readers and opened a ZIP containing `Chart-geo_7_49_8_48.png`. The reader returned invalid with `The zip archive does not contain the required file 'toc.json'.` The test used an actual small PNG payload and no matching extracted file.

**Recommendation:** validate the name and coordinates independently of filesystem existence, then validate/extract the image through the ZIP reader. If supporting GeoTIFF metadata inside ZIPs, parse the extracted bytes or a temporary file rather than probing the working directory. Test a plain ZIP without `toc.json` or pre-extracted files.

### 5. P2 — TIFF dimensions encoded as LONG are not read

**Location:** `src/fileFormats/TIFF.cpp:258–293` and `305–353`.

The field parser recognizes the byte size of `DT_Long` but only reads ASCII, SHORT, and DOUBLE values. LONG fields are stored as empty lists. TIFF permits ImageWidth (256) and ImageLength (257) to use either SHORT or LONG, so valid images using LONG dimensions fail in `readRasterSize()` with `No data for tag 256.` In addition, width/height are stored in `quint16`, which would truncate large LONG values if LONG parsing were added alone.

**Impact:** otherwise valid GeoTIFF charts cannot supply their georeferencing to the importer merely because their producer chose a permitted integer encoding.

**Validation:** compiled the production TIFF reader and supplied two minimal little-endian TIFF metadata fixtures with identical 640×480 dimensions. SHORT fields were accepted as `QSize(640, 480)`; LONG fields were rejected with `No data for tag 256.` These fixtures exercise metadata parsing, not raster decoding.

**Recommendation:** decode unsigned LONG fields and retain dimension values in a sufficiently wide type until they have been checked against QSize/application limits. Cover SHORT/LONG and both byte orders in regression tests.

### 6. P2 — Flight-log mutations ignore persistence failures after changing memory and files

**Location:** `src/flightlog/FlightLog.cpp:203–211`, `251–343`, `390–393`, and `596–599`. Storage methods return success/failure in `src/flightlog/FlightLogStorage.cpp`.

Single/bulk deletion and clearing remove IGC files and publish the new in-memory flight list before attempting the database operation. Their boolean database results are ignored. Add/update operations similarly publish changes before checking whether they persisted. This differs from `importFromJSON()`, which correctly checks `upsertMany()` before publishing its new list.

**Impact:** with a locked database, full disk, or other write failure, deleted flights can reappear on restart with permanently missing tracks, and added/edited flights can revert. An asynchronously emitted `saveError` reports the problem but cannot restore the deleted IGC files or reconcile the already-published state.

**Validation:** traced the mutation order and the database failure returns, including the rollback in `removeMany()`. No runtime storage-failure injection was performed. The flight-log UI is currently opt-in via `FLIGHTLOG=ON`.

**Recommendation:** require successful database persistence before publishing list mutations or deleting track files. For deletes, leftover orphan track files after a cleanup failure are preferable to live database rows referencing deleted files. Apply consistent failure handling to add/update and automatic landing finalization as well. Test locked-database failures and restart behavior.

## Verification and limits

- Used the locally installed Qt 6.12.0 desktop kit for isolated, headless C++ checks.
- Reproduced findings 1 and 2 using unchanged source blocks extracted from the production functions; these were not end-to-end network tests.
- Reproduced findings 4 and 5 by compiling the production file readers with generated fixtures.
- Findings 3 and 6 are supported by source-level error-path tracing; destructive failure scenarios were not exercised against user data.
- Temporary harnesses and fixtures are under `/tmp/opencode/`; no application window was launched and no production code was modified.
- A full application build, GUI smoke run, and mobile-device testing were not performed for this review. The existing build/lint/smoke checks do not exercise the malformed-input and storage-failure scenarios above.
