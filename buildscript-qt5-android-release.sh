#!/bin/bash

# Fail on first error
set -e

# Clear build directory
rm -rf build-qt5-android-release
mkdir build-qt5-android-release

# Configure
cd build-qt5-android-release
$Qt5_DIR_SOURCE/configure \
    -opensource \
    -confirm-license \
    -c++std c++2a \
    -xplatform android-clang \
    -prefix $Qt5_DIR_ANDROID \
    -android-ndk $ANDROID_NDK_ROOT \
    -android-sdk $ANDROID_SDK_ROOT \
    -force-debug-info \
    -nomake tests \
    -nomake examples \
    -no-separate-debug-info \
    -release \
    -skip qt3d \
    -skip qtcharts \
    -skip qtconnectivity \
    -skip qtdatavis3d \
    -skip qtdoc \
    -skip qtgamepad \
    -skip qtimageformats \
    -skip qtlottie \
    -skip qtmultimedia \
    -skip qtnetworkauth \
    -skip qtpim \
    -skip qtpurchasing \
    -skip qtquick3d \
    -skip qtquickcontrols \
    -skip qtquicktimeline \
    -skip qtscript \
    -skip qtscxml \
    -skip qtserialbus \
    -skip qtserialport \
    -skip qtvirtualkeyboard \
    -skip qtwayland \
    -skip qtwebchannel \
    -skip qtwebengine \
    -skip qtwebglplugin \
    -skip qtwebsockets \
    -skip qtwebview \
    -skip qtxmlpatterns \
    -ssl \
    -I /home/kebekus/Software/buildsystems/Qt-bugfix/openssl-1.1.1j/include \
    -no-warnings-are-errors

nice make -j8
rm -rf $Qt5_DIR_ANDROID
nice make install
