# enroute flight navigation

**enroute flight navigation** is a mobile flight navigation app for Android and
  other devices. The app is free, open source and 100% non-commercial. We do not
  collect user data. **enroute flight navigation** is a project of [Akaflieg
  Freiburg](https://akaflieg-freiburg.de), a flight club based in Freiburg,
  Germany. You can find more information on the
  [homepage](https://akaflieg-freiburg.github.io/enroute).

## 1. Compilation

**enroute flight navigation** is a fairly large, but fairly standard Qt
application that can be build on a standard Linux system. The build relies on
**cmake**. The author uses Fedora Linux. Builds on Windows or MacOS hosts has
never been attempted.

The GIT repository includes the following submodules, which are all tied in in
3rdParty.

* Google's Material Design Icon set

* Google's Roboto Fonts

* Nitroshare's QHTTPEngine

* For Android only: precompiled version of the openSSL libraries for various
  architectures.

### 1.1 Building a desktop app

On a standard Linux system, the following commands will build a desktop version
of the app. The commands produce a single binary at **./src/enroute** that links
to Qt dynamically, but contains all the data required to run.

```shell
# In source directory
git submodule init
git submodule update

# Create build directory
mkdir build
cd build

# Configure
cmake <path to source directory>

# Build
make -j5

# Enjoy
./src/enroute
```

We include a script, **buildscript-linux-debug.sh** that will build a debug
binary of **enroute** with LLVM's address-sanitizer and undefined-sanitizer
included. The CLANG compiler set must be available.

```shell
# Go to the main 'enroute' directory
cd <wherever you git cloned the directory>/enroute

# Build. This will cread a directory 'build-debug' and create the binary there
./buildscript-linux-debug.sh

# Run the binary, with memory leak detection disabled
export ASAN_OPTIONS=detect_leaks=0
./build-debug/src/enroute
```

### 1.2 Building an Android app

The author uses Qt Creator to build Android apps.

### 1.3 Dependencies

* **Qt development libraries**, version ≥ 5.12 for the desktop app and version ≥
    5.14 for the Android app.

* **CMake**, version ≥ 3.13 for the desktop app and version ≥ 3.15 for the
    Android app.

* Command line utilities: **curl**, **git**, **tar** (used to download data at
  configuration stage), **Doxygen**, **dot** (to build documentation,)
  **inkscape** (used to generate PNG versions of icons from SVG sources) and
  **python3** (used to compile lists of files during the configuration stage)

# 2. Other cmake targets

To aid development, the **cmake** scripts include a number of non-obvious
targets.

## 2.1 Target: DistributeQtSource

To comply with legal regulations, we need to make a copy of the Qt sources
available on the internet. This target does that automatically. Run the
following command on one of Stefan Kebekus' machines:

```bash
make DistributeQtSource
```

This will download the sources for the precise Qt version used for building this
binary and will make them available at
[https://cplx.vm.uni-freiburg.de/storage/QtSources](https://cplx.vm.uni-freiburg.de/storage/QtSources).


## 2.2 Target: generatedSources

This subdirectories of generatedSources contain source files that have been
generated from other sources, for instance icons in PNG format that have been
generated from scalable vector graphics. We include these generated sources in
our GIT repository because the tools required to build them are not universally
available on all platforms.

There exists a special CMake target, **generatedSources** that re-builds the
source files in this directory.
