# Project: <name>
Qt <6.x>, C++<17/20>, CMake + Ninja. UI in QML, backend in C++.
QML modules are declared with qt_add_qml_module; C++ types reach QML via QML_ELEMENT.

## Build & test
cmake -B build/claude -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build/claude
ctest --test-dir build/claude --output-on-failure
cmake --build build/claude --target all_qmllint   # lint all QML (resolves project modules)

## Layout
src/        C++ (QObject-derived types marked QML_ELEMENT, module URI "<App.Backend>")
qml/        QML UI files, listed under QML_FILES in qt_add_qml_module
tests/      Qt Test (C++) and TestCase (.qml) cases registered via qt_add_test / add_test
CMakeLists.txt uses qt_add_executable + qt_add_qml_module; AUTOMOC is on.

## Conventions & rules
- Always build into build/claude (pass -B build/claude). Never configure into the repo
  root or into build/ directly, and do not create any other build directory.
- Don't hand-edit moc_*, qrc_*, or anything under build/ — they're generated.
- New C++ type exposed to QML: add QML_ELEMENT, ensure the file is in the module's
  SOURCES, then rebuild before assuming QML can see it.
- New .qml file: add it to QML_FILES in CMakeLists.txt, or it won't be in the module.
- After editing C++ headers, rebuild — moc-generated code won't update otherwise.
- The GUI needs a display; in this session verify via `ctest`, not by launching the app.
- Match existing qmlformat / clang-format style; don't reformat unrelated lines.
