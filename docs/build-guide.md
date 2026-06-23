# enroute Build Guide

Covers: macOS universal binary (.dmg) and Android arm64 APK.

Prerequisites: see [env-setup.md](env-setup.md).

---

## Repository Layout

```
~/labs/
  enroute/                        # main app source (qubolino/enroute)
  enrouteDependencies/            # dependencies (Akaflieg-Freiburg/enrouteDependencies)
    libzip/                       # submodule
    maplibre-native-qt/           # submodule (used for Android)
    maplibre-native-qt-flat/      # submodule (used for macOS)
    bzip2/                        # submodule
    zlib/                         # submodule
  meteo-map-server/               # second project (qubolino/meteo-map-server)
```

---

## Step 0 — Clone Repos

```bash
cd ~/labs

gh repo clone qubolino/enroute
gh repo clone qubolino/meteo-map-server
gh repo clone Akaflieg-Freiburg/enrouteDependencies -- --recurse-submodules
```

If submodules are missing or empty after clone:
```bash
cd enrouteDependencies
git submodule update --init --recursive

# If a submodule fails because its directory is non-empty (e.g. enrouteText):
rm -rf 3rdParty/enrouteText
git submodule update --init --recursive

# Force-init any that are still empty:
git submodule update --init --force spriteGenerator enrouteOGN sunset metaf
```

> The `egm` directory (WW15MGH.DAC) is a data directory, not a submodule — ignore it if it appears in missing-submodule warnings.

---

## macOS Build (universal binary: x86_64 + arm64)

Build order: libzip → maplibre-native-qt → enroute

### 1. Build libzip

```bash
cd ~/labs

~/Qt/6.10.1/macos/bin/qt-cmake \
  -S enrouteDependencies/libzip \
  -B build-libzip \
  -G Ninja \
  -DBUILD_DOC=OFF \
  -DBUILD_EXAMPLES=OFF \
  -DBUILD_REGRESS=OFF \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TOOLS=OFF \
  -DENABLE_BZIP2=OFF \
  -DENABLE_LZMA=OFF \
  -DENABLE_ZSTD=OFF \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_INSTALL_PREFIX=~/Qt/6.10.1/macos

cmake --build build-libzip
cmake --install build-libzip
```

### 2. Build maplibre-native-qt (flat variant)

```bash
cd ~/labs

~/Qt/6.10.1/macos/bin/qt-cmake \
  -S enrouteDependencies/maplibre-native-qt-flat \
  -B build-maplibre-native-qt \
  -G Ninja \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
  -DBUILD_TESTING=OFF \
  -DCMAKE_PREFIX_PATH=~/Qt/6.10.1/macos \
  -DCMAKE_INSTALL_PREFIX=~/Qt/6.10.1/macos

cmake --build build-maplibre-native-qt
cmake --install build-maplibre-native-qt
```

> **Note:** macOS uses `maplibre-native-qt-flat` (the `-flat` submodule). Android uses `maplibre-native-qt` (no suffix). Do not mix them up.

### 3. Build enroute

```bash
cd ~/labs

~/Qt/6.10.1/macos/bin/qt-cmake \
  -S enroute \
  -B build-enroute \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
  -DCMAKE_INSTALL_PREFIX=. \
  -DQTDEPLOY=ON

cmake --build build-enroute
cmake --install build-enroute
```

### 4. Package as .dmg

```bash
cd ~/labs/build-enroute

~/Qt/6.10.1/macos/bin/macdeployqt \
  enroute\ Flight\ Navigation.app \
  -dmg \
  -appstore-compliant

mv "enroute Flight Navigation.dmg" ~/labs/enroute-macOS-universal.dmg
```

Output: `~/labs/enroute-macOS-universal.dmg`

---

## Android Build (arm64-v8a)

Build order: libzip → maplibre-native-qt → enroute

### 1. Build libzip (Android)

```bash
cd ~/labs

~/Qt-android/6.9.3/android_arm64_v8a/bin/qt-cmake \
  -S enrouteDependencies/libzip \
  -B build-libzip-android \
  -G Ninja \
  -DBUILD_DOC=OFF \
  -DBUILD_EXAMPLES=OFF \
  -DBUILD_REGRESS=OFF \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TOOLS=OFF \
  -DENABLE_BZIP2=OFF \
  -DENABLE_LZMA=OFF \
  -DENABLE_ZSTD=OFF \
  -DCMAKE_INSTALL_PREFIX=~/Qt-android/6.9.3/android_arm64_v8a

cmake --build build-libzip-android
cmake --install build-libzip-android
```

### 2. Build maplibre-native-qt (Android)

```bash
cd ~/labs

~/Qt-android/6.9.3/android_arm64_v8a/bin/qt-cmake \
  -S enrouteDependencies/maplibre-native-qt \
  -B build-maplibre-native-qt-android \
  -G Ninja \
  -DBUILD_TESTING=OFF \
  -DCMAKE_PREFIX_PATH=~/Qt-android/6.9.3/android_arm64_v8a \
  -DCMAKE_INSTALL_PREFIX=~/Qt-android/6.9.3/android_arm64_v8a

cmake --build build-maplibre-native-qt-android
cmake --install build-maplibre-native-qt-android
```

> **Note:** This step is long (~300 compilation units). If interrupted, just re-run `cmake --build build-maplibre-native-qt-android` — it resumes from where it left off.

### 3. Build enroute (Android)

```bash
cd ~/labs

~/Qt-android/6.9.3/android_arm64_v8a/bin/qt-cmake \
  -S enroute \
  -B build-enroute-android \
  -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug

cmake --build build-enroute-android
```

Output APK: `~/labs/build-enroute-android/src/android-build/build/outputs/apk/debug/android-build-debug.apk`

---

## Deploying to Android Device

### First time setup

Enable USB debugging on the tablet:
- Settings → About → tap Build Number 7 times → Developer Options → USB Debugging: ON

Connect via USB, then:
```bash
adb devices   # confirm device appears
```

### Install APK

**If the app is already installed from the Play Store** (different signature → must uninstall first):

> **Back up user data before uninstalling** — see the Data Backup section below.

```bash
adb uninstall de.akaflieg_freiburg.enroute
adb install ~/labs/build-enroute-android/src/android-build/build/outputs/apk/debug/android-build-debug.apk
```

**If reinstalling a debug build over a previous debug build:**
```bash
adb install -r ~/labs/build-enroute-android/src/android-build/build/outputs/apk/debug/android-build-debug.apk
```

---

## Data Backup and Restore

The app has `allowBackup=false` so `adb backup` produces only a header. Use `run-as` instead.

### Back up user data

```bash
mkdir -p ~/labs/enroute-data-backup/VAC

# Internal files
adb shell "run-as de.akaflieg_freiburg.enroute cat '/data/data/de.akaflieg_freiburg.enroute/files/flight route.geojson'" \
  > ~/labs/enroute-data-backup/flight_route.geojson

adb shell "run-as de.akaflieg_freiburg.enroute cat '/data/data/de.akaflieg_freiburg.enroute/files/waypoint library.geojson'" \
  > ~/labs/enroute-data-backup/waypoint_library.geojson

# VAC charts — too large for cat; copy to sdcard first, then pull
adb shell "run-as de.akaflieg_freiburg.enroute sh -c \
  'mkdir -p /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/VAC-backup && \
   cp /data/data/de.akaflieg_freiburg.enroute/files/VAC/*.webp \
      /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/VAC-backup/'"

adb pull /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/VAC-backup \
  ~/labs/enroute-data-backup/VAC

# Clean up sdcard staging
adb shell rm -rf /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/VAC-backup
```

> **Warning:** `/sdcard/Android/data/<package>/` is deleted automatically when the app is uninstalled. Always complete the `adb pull` before uninstalling.

### Restore user data

After installing the APK, launch and close the app once so it creates its data directories. Then:

```bash
# Stage files via sdcard (direct push to /data/data/ is blocked)
adb push ~/labs/enroute-data-backup/flight_route.geojson \
  /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/flight_route_tmp.geojson
adb push ~/labs/enroute-data-backup/waypoint_library.geojson \
  /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/waypoint_library_tmp.geojson

# Copy into internal storage
adb shell "run-as de.akaflieg_freiburg.enroute sh -c \
  'cp /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/flight_route_tmp.geojson \
      \"/data/data/de.akaflieg_freiburg.enroute/files/flight route.geojson\"'"
adb shell "run-as de.akaflieg_freiburg.enroute sh -c \
  'cp /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/waypoint_library_tmp.geojson \
      \"/data/data/de.akaflieg_freiburg.enroute/files/waypoint library.geojson\"'"

# VAC charts
adb push ~/labs/enroute-data-backup/VAC \
  /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/VAC-restore

adb shell "run-as de.akaflieg_freiburg.enroute mkdir -p \
  /data/data/de.akaflieg_freiburg.enroute/files/VAC"
adb shell "run-as de.akaflieg_freiburg.enroute sh -c \
  'cp /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/VAC-restore/VAC/*.webp \
      /data/data/de.akaflieg_freiburg.enroute/files/VAC/'"

# Clean up sdcard staging
adb shell rm -f /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/flight_route_tmp.geojson
adb shell rm -f /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/waypoint_library_tmp.geojson
adb shell rm -rf /sdcard/Android/data/de.akaflieg_freiburg.enroute/files/VAC-restore

# Verify
adb shell "run-as de.akaflieg_freiburg.enroute ls -lh /data/data/de.akaflieg_freiburg.enroute/files/"
adb shell "run-as de.akaflieg_freiburg.enroute ls /data/data/de.akaflieg_freiburg.enroute/files/VAC | wc -l"
```

Expected output: flight route.geojson (7.3K), waypoint library.geojson (~1.9M), VAC/ with 526 files.

---

## Incremental Rebuilds

After source changes, you only need to re-run `cmake --build` — no need to reconfigure:

```bash
cmake --build build-enroute          # macOS
cmake --build build-enroute-android  # Android
```

If CMakeLists.txt or Qt version changes, re-run the full `qt-cmake` configure step first.

The dependency builds (libzip, maplibre) rarely need to be redone — only if you update `enrouteDependencies` submodules.

---

## Checking the workflow files for version requirements

The enroute repo's CI workflows are the authoritative source for which Qt version to use:

```bash
cat ~/labs/enroute/.github/workflows/macos.yml    | grep -i qt
cat ~/labs/enroute/.github/workflows/android.yml  | grep -i qt
```
