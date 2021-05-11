#!/bin/bash

# Fail on first error
set -e

# Clear build director
rm -rf build-qt5-android-release
mkdir build-qt5-android-release

# Configure
cd build-qt5-android-release
../3rdParty/qt5/configure \
             -opensource \
	     -confirm-license \
	     -xplatform android-clang \
	     -prefix $HOME/Software/buildsystems/qt5-android-release \
	     -android-ndk $ANDROID_NDK_ROOT \
	     -android-sdk $ANDROID_SDK_ROOT \
	     -nomake tests \
	     -nomake examples \
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
	     -skip qttools \
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
