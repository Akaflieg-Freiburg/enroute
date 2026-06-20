# Environment Setup — enroute Build Environment

Tested on: macOS (Apple Silicon + Intel), fresh install, June 2026.

---

## 1. Core Tools (required for both targets)

### Xcode

Install the full **Xcode app** from the Mac App Store (not just Command Line Tools). The Qt installer requires it and, on current macOS, `clang` resolves from inside `Xcode.app` — the standalone CLT package no longer ships a current toolchain.

After installing, activate the developer tools:
```bash
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
xcodebuild -license accept
```

Verify:
```bash
clang --version   # should show Apple clang from Xcode.app path
xcode-select -p   # should print /Applications/Xcode.app/Contents/Developer
```

> **Do not remove Xcode** after the build — `clang` lives inside it. Removing it breaks both the macOS and Android builds.

### Homebrew
```bash
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
```
Follow the post-install instructions to add brew to PATH (shown at end of install).

### Git, CMake, Ninja
```bash
brew install git cmake ninja
```

### GitHub CLI
```bash
brew install gh
gh auth login   # choose GitHub.com → HTTPS → authenticate via browser
```

---

## 2. Qt — macOS Desktop Build

**Version required:** 6.10.1 (check `.github/workflows/macos.yml` in enroute for the current version)

**Install path:** `~/Qt/6.10.1/macos/`

### Install via Qt Online Installer

Download the installer DMG from https://www.qt.io/download-qt-installer-oss

Mount and run headless (replace `YOUR_PASSWORD` with your Qt account password — pass via env var, never on the command line):

```bash
export QT_PW="YOUR_PASSWORD"

/Volumes/qt-online-installer-macOS-x64-4.8.1/qt-online-installer-macOS-x64-4.8.1.app/Contents/MacOS/qt-online-installer-macOS-x64-4.8.1 \
  --accept-licenses --accept-obligations --confirm-command --default-answer \
  -m qubolino@gmail.com --password "$QT_PW" \
  --root ~/Qt \
  install \
  qt.qt6.6101.macos \
  qt.qt6.6101.qt5compat \
  qt.qt6.6101.qtcharts \
  qt.qt6.6101.qtconnectivity \
  qt.qt6.6101.qthttpserver \
  qt.qt6.6101.qtimageformats \
  qt.qt6.6101.qtlocation \
  qt.qt6.6101.qtmultimedia \
  qt.qt6.6101.qtpositioning \
  qt.qt6.6101.qtsensors \
  qt.qt6.6101.qtserialport \
  qt.qt6.6101.qtshadertools \
  qt.qt6.6101.qtspeech \
  qt.qt6.6101.qtwebsockets \
  qt.qt6.6101.qtwebview
```

> **Note on component names:** The component names above follow the pattern `qt.qt6.<XXYYZZ>.<module>` where `XXYYZZ` is the version without dots (6.10.1 → `6101`). If the installer rejects them as "virtual", parse the real names from the installer's cache:
> ```bash
> grep -r "qt\.qt6\." ~/Library/Caches/qt-unified-mac-online/ | grep -o 'qt\.qt6\.[^"<]*' | sort -u | less
> ```

### Verify
```bash
~/Qt/6.10.1/macos/bin/qmake --version
~/Qt/6.10.1/macos/bin/qt-cmake --version
```

---

## 3. Qt — Android Build

**Version required:** 6.9.3 (check `.github/workflows/android.yml` in enroute for the current version)

**Install path:** `~/Qt-android/6.9.3/`

### Install via Qt Online Installer (same installer, different root + components)

```bash
export QT_PW="YOUR_PASSWORD"

/Volumes/qt-online-installer-macOS-x64-4.8.1/qt-online-installer-macOS-x64-4.8.1.app/Contents/MacOS/qt-online-installer-macOS-x64-4.8.1 \
  --accept-licenses --accept-obligations --confirm-command --default-answer \
  -m qubolino@gmail.com --password "$QT_PW" \
  --root ~/Qt-android \
  install \
  qt.qt6.693.android_arm64_v8a \
  qt.qt6.693.android_armv7 \
  qt.qt6.693.android_x86 \
  qt.qt6.693.android_x86_64 \
  qt.qt6.693.addons.qt5compat \
  qt.qt6.693.addons.qtconnectivity \
  qt.qt6.693.addons.qthttpserver \
  qt.qt6.693.addons.qtimageformats \
  qt.qt6.693.addons.qtlocation \
  qt.qt6.693.addons.qtmultimedia \
  qt.qt6.693.addons.qtpositioning \
  qt.qt6.693.addons.qtsensors \
  qt.qt6.693.addons.qtserialport \
  qt.qt6.693.addons.qtshadertools \
  qt.qt6.693.addons.qtspeech \
  qt.qt6.693.addons.qtwebsockets \
  qt.qt6.693.addons.qtwebview
```

> **Note:** For Android, addons use the `.addons.` infix without a platform suffix. The base platforms (`android_arm64_v8a` etc.) do have a platform suffix.

> **Troubleshooting cache.lock:** If you see `Cannot obtain the lock for file cache.lock`:
> ```bash
> rm ~/Library/Caches/qt-unified-mac-online/cache.lock
> ```

### Verify
```bash
~/Qt-android/6.9.3/android_arm64_v8a/bin/qmake --version
```

---

## 4. Android SDK, NDK, and JDK (required only for Android build)

### JDK 17
```bash
brew install --cask temurin@17
```
Installs to `/Library/Java/JavaVirtualMachines/temurin-17.jdk/`. Verify:
```bash
java -version   # should show openjdk 17
```

### Android Command Line Tools
```bash
brew install --cask android-commandlinetools
```
This installs `sdkmanager` to `/opt/homebrew/share/android-commandlinetools/bin/` (Apple Silicon) or `/usr/local/share/android-commandlinetools/bin/` (Intel).

### SDK Platform and NDK

**Versions required:**
- Platform: `android-34`
- NDK: `27.2.12479018`

```bash
# Set SDK root
export ANDROID_SDK_ROOT=~/Library/Android/sdk
mkdir -p "$ANDROID_SDK_ROOT"

sdkmanager --sdk_root="$ANDROID_SDK_ROOT" "platforms;android-34"
sdkmanager --sdk_root="$ANDROID_SDK_ROOT" "ndk;27.2.12479018"
sdkmanager --sdk_root="$ANDROID_SDK_ROOT" "build-tools;34.0.0"
```

Accept all licenses when prompted:
```bash
sdkmanager --sdk_root="$ANDROID_SDK_ROOT" --licenses
```

### Add adb to PATH

Add to `~/.zshrc`:
```bash
export PATH="$PATH:$HOME/Library/Android/sdk/platform-tools"
```
Then `source ~/.zshrc`.

### Verify
```bash
adb version
ls ~/Library/Android/sdk/ndk/27.2.12479018/
```

---

## 5. Summary — What's needed for each target

| Tool | macOS build | Android build |
|------|-------------|---------------|
| Xcode CLI tools | ✓ | ✓ |
| Homebrew | ✓ | ✓ |
| git, cmake, ninja | ✓ | ✓ |
| GitHub CLI (`gh`) | ✓ | ✓ |
| Qt 6.10.1 macOS | ✓ | — |
| Qt 6.9.3 Android | — | ✓ |
| JDK 17 | — | ✓ |
| Android SDK (platform-34) | — | ✓ |
| Android NDK (27.2.12479018) | — | ✓ |
| adb (platform-tools) | — | ✓ (deploy) |
