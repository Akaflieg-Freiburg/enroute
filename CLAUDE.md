# Project: enroute
Qt 6.9+, C++20, CMake + Ninja. UI in QML, backend in C++.
QML modules are declared with qt_add_qml_module; C++ types reach QML via QML_ELEMENT.
QML module URI: akaflieg_freiburg.enroute

## Build & lint
# The system Qt under /usr/lib64 is incomplete; build against the Qt online-installer
# build under ~/Software/buildsystems/Qt. Auto-detect the newest version (it changes
# over time) and pass it as the prefix — do NOT hardcode a version number.
# Development happens on both Linux (kit dir "gcc_64") and macOS (kit dir "macos"),
# so detect whichever desktop kit is present — do NOT hardcode the platform either.
QTDIR=$(find ~/Software/buildsystems/Qt -maxdepth 2 -type d \( -name gcc_64 -o -name macos \) | sort -V | tail -1)
cmake -B build/claude -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_PREFIX_PATH="$QTDIR"
cmake --build build/claude
cmake --build build/claude --target all_qmllint   # lint all QML (resolves project modules)
# NOTE: this project has no test target — `ctest` reports 0 tests. Verify via build +
# qmllint (and, if needed, by launching the app).

## Layout
src/        C++ (QObject-derived types marked QML_ELEMENT, module URI "akaflieg_freiburg.enroute")
qml/        QML UI files, listed under QML_FILES in qt_add_qml_module
CMakeLists.txt uses qt_add_executable + qt_add_qml_module; AUTOMOC is on.

## Conventions & rules
- Always build into build/claude (pass -B build/claude). Never configure into the repo
  root or into build/ directly, and do not create any other build directory.
- Don't hand-edit moc_*, qrc_*, or anything under build/ — they're generated.
- New C++ type exposed to QML: add QML_ELEMENT, ensure the file is in the module's
  SOURCES, then rebuild before assuming QML can see it.
- New .qml file: add it to QML_FILES in CMakeLists.txt, or it won't be in the module.
- After editing C++ headers, rebuild — moc-generated code won't update otherwise.
- A display is available (DISPLAY=:0 / Wayland). Launching the app puts a real window on
  the user's live desktop and may use real sensors/Bluetooth/position — don't launch it
  unprompted; ask first.
- Match existing qmlformat / clang-format style; don't reformat unrelated lines.
