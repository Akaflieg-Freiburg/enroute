# Android Java and C++/JNI review

Reviewed commit: `37d94f1a83796448c16b2d0f7a4bc583dd258f8f`.

## Scope and verification

This review read **all five first-party Java classes** under `src/android/src/de/akaflieg_freiburg/enroute/` in full: `MobileAdaptor`, `ShareActivity`, `UsbSerialHelper`, `UsbConnectionReceiver`, and `FlightLogService`. There are no Kotlin files under `src/`.

Also reviewed in full:

- `src/platform/PlatformAdaptor_Android.cpp/.h` and `FileExchange_Android.cpp/.h`.
- `src/traffic/TrafficDataSource_SerialPort.cpp`, the Bluetooth Classic/LE implementation files, their provider add-source functions, and `ConnectionScanner_Bluetooth.cpp`.
- `src/AndroidManifest.xml.in`, `src/android/build.gradle`, USB and FileProvider XML configuration, and `.github/workflows/android.yml`.

Traced supporting sections in `main.cpp`, `GlobalObject.cpp`, `FlightLog.cpp/.h`, `GlobalSettings.cpp`, `TrafficDataProvider.cpp`, `ConnectionInfo.cpp/.h`, positioning/sensors, the Android tile-server implementation, CMake, and QML location-permission/flight-log settings. This is a source review of the Android integration, not an exhaustive review of all shared application code or dependencies.

Executed checks:

- Compiled all five Java classes using `javac --release 17`, the installed Android API 36 SDK, Qt 6.12.0 Android JAR and real Qt binding sources, and the project's cached AndroidX/USB-serial dependency JARs. Only generated `R` resource IDs were stubbed. Compilation passed, with deprecation notes. An initial attempt lacked the Qt binding sources; adding those resolved the harness setup error.
- Inspected compiled JVM descriptors with `javap -s -private`, comparing the native declarations and Java entry points with the C++ exports/calls. No method-name or descriptor mismatch was found in the first-party bridge. This does not verify dynamic linking in a packaged APK.
- Checked the installed SDK API-version database for the service API in finding 1.
- Ran a small native harness against a real OpenJDK JVM and desktop Qt to test the JNI string conversion in finding 6.
- Inspected the installed Qt JNI implementation to establish reference ownership in finding 7, and Android documentation for findings 2 and 3.

**No APK/NDK build, Gradle Android lint, emulator run, physical-device test, or USB/Bluetooth hardware test was performed.** Standalone Java compilation cannot verify Android lifecycle behavior. Temporary checks are under `/tmp/opencode/enroute-android-review-*`. Application source was not changed.

## Findings

### 1. P1 — Flight recording calls an unavailable API on Android 9 — DONE

**Status:** Done (2026-10-22, uncommitted). `onStartCommand()` now calls the three-argument `startForeground()` only on API 29+ and the two-argument overload below. The `NoSuchMethodError` crash was reproduced with the unfixed code on an API 28 emulator (crash loop on every start with automatic detection persisted). With the fix, the service runs in the foreground without crashing on API 28 and API 29 emulators.

**Location:** `src/android/src/de/akaflieg_freiburg/enroute/FlightLogService.java:127–133`; supported minimum in `src/CMakeLists.txt:399–400`.

`onStartCommand()` unconditionally calls the three-argument `startForeground(id, notification, type)`. This overload was introduced in API **29**, while the app supports API **28**. There is no SDK-version guard or compatibility wrapper.

**Trigger/impact:** in a `FLIGHTLOG=ON` build on Android 9, enable automatic flight detection. Starting the service reaches a missing framework method and fails with `NoSuchMethodError`, terminating the process. A persisted enabled setting also starts this path when `FlightLog` initializes.

**Evidence:** the installed SDK `platforms/android-36/data/api-versions.xml` marks `startForeground(ILandroid/app/Notification;I)V` as `since="29"`; the two-argument overload is available since API 5. Verified against metadata, not reproduced on an Android 9 device.

**Fix:** use `ServiceCompat.startForeground()` or call the two-argument overload below API 29. Verify service startup on API 28 and a current Android version.

### 2. P1 — Location-service startup is not gated on location permission — DONE

**Status:** Done (2026-10-23, uncommitted). `FlightLogService.start()` now returns a boolean and calls `startForegroundService()` only if Android allows a location foreground service (API 34+: coarse or fine location granted); it catches start exceptions and owns the running state (`s_running`). `onStartCommand()` catches promotion failures, and a START_STICKY restart (Qt not running) now posts the restart notification and stops instead of promoting. In C++, `FlightLog::updateForegroundService()` starts/stops the service; only a user-initiated switch-on requests location permission, and a declined request shows the "Background location access" dialog. The service start is retried whenever the window regains focus (new `PlatformAdaptor::windowFocusGained()` from `MobileAdaptor.onWindowFocusChanged()`), which also covers multi-window mode where Qt keeps the app active. Related: the notification permission request now uses its own request code and its result is no longer forwarded to Qt. Verified on emulators: crash reproduced with the unfixed code on API 36 (launch and sticky restart); with the fix on API 36 no crash with location denied, revoked while running, or permanently denied; service starts after a grant in the startup dialog, the toggle dialog (precise and approximate), or system settings, in full-screen and multi-window mode; regression on API 28 and 29 without findings.

**Locations:** `FlightLogService.java:253–265` and `127–133`; `src/flightlog/FlightLog.cpp:901–908, 933–943`; `src/qml/pages/FlightLogPage.qml:145–148`.

Enabling automatic detection starts a location-type foreground service without first ensuring that coarse or fine location permission has been granted. The only permission requested by `FlightLogService.start()` is `POST_NOTIFICATIONS`. Calling `PositionProvider.startUpdates()` does not supply this gate: the C++ method forwards to the satellite source, and Qt's Android positioning backend checks permission without granting it. The QML location request is asynchronous and users can deny it.

**Trigger/impact:** on Android 14+ in a `FLIGHTLOG=ON` build, deny location access and enable automatic detection. Android enforces the runtime prerequisites for a location foreground service. An uncaught `SecurityException` in `onStartCommand()` can terminate the app. This is also relevant after permission revocation with automatic detection persisted as enabled. Receiving position from an external traffic receiver does not satisfy Android's permission requirement for the declared service type.

The C++ side additionally sets `m_foregroundServiceRunning = true` immediately after a void JNI call, without acknowledgement that promotion succeeded. A synchronous startup failure can therefore leave the native state incorrectly reporting that the service is running.

**Evidence:** traced the setting-to-JNI-to-service path, QML permission handling, and Qt positioning permission check. Android documents the required coarse/fine grant and foreground-start eligibility. No permission-denial device test was run.

**Fix:** make permission and foreground eligibility explicit prerequisites, wait for their results, handle service-start/promotion failures in Java, and report actual success/failure back to C++. Keep the native running state consistent with service lifecycle. Test denied, revoked, and newly granted permission states.

### 3. P2 — Package-visibility checks can reject installed sharing/viewing apps

**Location:** `src/android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java:656–677`; `src/AndroidManifest.xml.in`.

`customStartActivity()` refuses to launch the system chooser if `resolveActivity()` returns null or `queryIntentActivities()` returns an empty list. With Android 11+ package visibility, these queries can omit installed handlers even though starting their activity is allowed. The manifest has no matching `<queries>` declarations for these outgoing SEND/VIEW operations. Incoming activity intent filters do not grant that visibility.

**Trigger/impact:** try to open an exported FPL/PLN (`application/xml`) with an installed handler that is not automatically visible to Enroute. The app can report that no suitable app exists without ever invoking the chooser. Some MIME types are masked by Enroute itself appearing in the query results; this is not a claim that every share operation fails.

**Evidence:** traced the preflight exits and checked the application manifest, Qt deployment code, and cached direct dependency manifests for visibility declarations. Cross-checked Android's package-visibility guidance. Not reproduced on a device; the final merged APK manifest was not generated in this review.

**Fix:** launch the system chooser and handle `ActivityNotFoundException`, excluding the known Enroute component directly. If enumeration is genuinely required, declare appropriately scoped `<queries>` entries. Test with a newly installed third-party handler on Android 11+.

### 4. P2 — USB product labels are used as connection identifiers

**Locations:** `src/android/src/de/akaflieg_freiburg/enroute/UsbSerialHelper.java:110–116, 200–225, 275–282`; `ShareActivity.java:225–232`; `src/traffic/TrafficDataProvider.cpp:215–229`.

The Java enumeration and USB-attach path normally return `getProductName()`, not a unique device identifier. That label becomes the C++ connection identity and the Java `connections` map key. `openDevice()` chooses the first device with a matching product name.

**Trigger/impact:** attach two adapters with the same common product label, such as two identical USB-to-serial adapters. The user cannot reliably select the intended physical receiver, and adding the second can be rejected as an already-existing connection. Enumeration order can make a saved connection open the other adapter. `getPorts().get(0)` also means only the first port is addressable on a multiport adapter.

**Evidence:** source-level identity tracing through enumeration, JNI, duplicate detection, and open. No multi-adapter hardware test was run.

**Fix:** carry a distinct machine identifier and human-readable label across JNI, with a port index. Use the device path for current-session identity; if persistence across reconnection is required, define a stable identity/resolution policy rather than treating the product label as unique. Test identical adapters and multiport devices.

### 5. P2 — BLE reconnection can select a different same-named receiver

**Location:** `src/traffic/TrafficDataSource_BluetoothLowEnergy.cpp:305–317`; identity contract in `src/traffic/ConnectionInfo.cpp:173–195`.

The source is created for a specific `QBluetoothDeviceInfo`, but its discovery callback accepts any device whose **name** matches. It then overwrites `m_info` and connects to that device. On Android, the existing connection identity elsewhere in the application uses the Bluetooth address. The published `m_connectionInfo` is not updated by this switch.

**Trigger/impact:** two nearby receivers advertise the same default name. The first discovery result can replace the receiver selected by the user, supplying position/traffic from the wrong device while the connection metadata still identifies the original one. This is shared C++ used on Android, rather than Android-only code.

**Evidence:** source-level comparison of the discovery match, controller replacement, and connection identity. No radio test was run.

**Fix:** match the selected device's identity (Android address; platform-appropriate UUID where needed). If changing/randomized identities require rediscovery, make that an explicit resolution policy rather than silently matching only the display name. Test same-name/different-address devices.

### 6. P2 — Java strings are decoded using the wrong UTF-8 variant

**Locations:** `src/platform/FileExchange_Android.cpp:250–263` and `src/platform/PlatformAdaptor_Android.cpp:374–377`.

`GetStringUTFChars()` supplies JNI **modified UTF-8**, while `QString::fromUtf8()` expects standard UTF-8. Supplementary Unicode characters are encoded as surrogate pairs in modified UTF-8 and are rejected by the Qt decoder. This affects incoming document display names, shared text, and USB product names passed by the attach callback.

**Impact:** names containing emoji or other non-BMP characters are corrupted. For USB, this can also prevent a connection: normal enumeration converts via `QJniObject::toString()` correctly, while the attach callback produces a different, damaged name. Percent-encoded ASCII URIs are not themselves affected merely because the underlying filename contains emoji.

**Validation:** a native harness created a real Java string using `JNIEnv::NewString()` and ran the same `GetStringUTFChars()` → `QString::fromUtf8()` conversion. `Chart-✈-😀.png` became `Chart-✈-������.png`. This reproduces the encoding defect with OpenJDK and desktop Qt; it is not an Android import test.

**Fix:** use `QJniObject(jString).toString()` or `GetStringChars()`/`GetStringLength()` with `QString::fromUtf16()`. Retain the null checks. Do not use `fromLocalRef()` to consume a borrowed callback argument.

### 7. P3 — USB enumeration leaks JNI local references

**Location:** `src/platform/PlatformAdaptor_Android.cpp:254–263`, specifically line 258.

Each `GetObjectArrayElement()` creates a local reference. Constructing a `QJniObject` from it creates owned global references but does **not** take ownership of that original local reference. The loop never calls `DeleteLocalRef()`. In the installed Qt implementation, destroying `QJniEnvironment` clears exceptions; it does not pop a local frame or detach the thread.

**Impact:** repeated serial-device scans retain Java strings/local-reference entries on the long-lived Qt thread. The leak is small per scan, but accumulates over the session and is unnecessary at a frequently revisited JNI boundary. No table-overflow crash is claimed or reproduced.

**Evidence:** checked `QJniObject(jobject)`, `QJniObject::fromLocalRef()`, and `QJniEnvironment::~QJniEnvironment()` in the installed Qt source, along with the scanner's call path.

**Fix:** wrap the newly returned element with `QJniObject::fromLocalRef(env->GetObjectArrayElement(...))`, explicitly delete the local reference, or use a scoped local frame. Unlike finding 6's callback arguments, these are newly acquired references owned by the caller.

## Other review observations and remaining verification

- Java-to-C++ callbacks generally copy input before posting work with `GlobalObject::runOnMainThread()`, avoiding capture of a thread-local `JNIEnv*` in queued work. Back handling is asynchronous, and status-bar updates explicitly marshal to the Android UI thread.
- Several C++-initiated Java paths still touch Activity/window state directly from the Qt thread: safe-inset reads, pending-intent processing, picker/create-document launch, and notification permission requests. Device-side thread/lifecycle instrumentation would be useful here; this review does not claim a reproduced thread-affinity crash.
- USB byte chunks are fed into `processFLARMData()`, which maintains an incremental buffer. Splitting a read in the middle of a sentence is therefore not, by itself, a parser defect. The fixed polling budget (one 1024-byte read per 200 ms) should be exercised with sustained high-rate receivers, but no measured overflow is asserted here.
- Flight recording is opt-in (`FLIGHTLOG=ON`). The Android CI workflow builds the default configuration and contains no device/emulator run. It does not exercise the service failures above; the Gradle configuration also sets `lintOptions.abortOnError = false`.
- Highest-value runtime follow-up: API 28 service startup; API 34+ denied/revoked location permission; package visibility with a fresh third-party file handler; identical USB/BLE devices; activity recreation and background/resume during permission/file dialogs.

## References

- [Android package visibility: common use cases](https://developer.android.com/training/package-visibility/use-cases), especially opening files and custom sharesheets.
- [Android foreground service types: location prerequisites](https://developer.android.com/develop/background-work/services/fgs/service-types#location).
- Local Android SDK: `platforms/android-36/data/api-versions.xml`; `sources/android-35/android/app/Service.java`.
- Local Qt 6.12.0 source: `qtbase/src/corelib/kernel/qjniobject.cpp`, `qjnienvironment.cpp`, `qjnitypes_impl.h`; Android positioning `jnipositioning.cpp`.
