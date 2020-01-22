# enroute flight navigation

**enroute flight navigation** is a mobile flight navigation app for Android and other devices. The app is free, open source and 100% non-commercial. We do not collect user data. **enroute flight navigation** is a project of [Akaflieg Freiburg](https://akaflieg-freiburg.de), a flight club based in Freiburg, Germany. You can find more information on the [homepage](https://akaflieg-freiburg.github.io/enroute).

## 1. Compilation

**enroute flight navigation** is a fairly large, but fairly standard Qt application that can be build on a standard Linux system. The build relies on **cmake**. The author uses Fedora Linux. Builds on Windows or MacOS hosts has never been attempted.

The CMake scripts download substantial amounts of data during the build process.

* Google's Material Design Icon set

* Google's Roboto Fonts

* Nitroshare's QHTTPEngine

* For Android only: precompiled version of the openSSL libraries for various architectures.

### 1.1 Building a desktop app

On a standard Linux system, the following commands will build a desktop version of the app. The commands produce a single binary at **./src/enroute** that links to Qt dynamically, but contains all the data required to run.

```shell
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

### 1.2 Building an Android app

The author uses Qt Creator to build Android apps.

### 1.3 Dependencies

* **Qt development libraries**, version ≥ 5.12 for the desktop app and version ≥ 5.14 for the Android app.

* **CMake**, version ≥ 3.13 for the desktop app and version ≥ 3.15 for the Android app.

* Command line utilities: **curl**, **git**, **tar** (used to download data at configuration stage), **Doxygen**, **dot** (to build documentation,) **inkscape** (used to generate PNG versions of icons from SVG sources) and **python3** (used to compile lists of files during the configuration stage)


