#!/bin/bash

#
# Copyright © 2016-2020 Stefan Kebekus <stefan.kebekus@math.uni-freiburg.de>
#
#
# This script builds the scantools library and executables in "Debug" mode.
# Several sanitizers are switched on.
#
# Run this script in the main directory tree.

#
# Clean
#

rm -rf build-android-debug
mkdir -p build-android-debug
cd build-android-debug

#
# Configure
#

export ANDROID_SDK=/home/kebekus/Software/buildsystems/Android-SDK
export ANDROID_NDK=$ANDROID_SDK/ndk-bundle
export QTDIR=/home/kebekus/Software/buildsystems/Qt/5.14.1/android

cmake /home/kebekus/Software/projects/enroute \
      -DANDROID_ABI:STRING=armeabi-v7a \
      -DANDROID_BUILD_ABI_arm64-v8a:BOOL=ON \
      -DANDROID_BUILD_ABI_armeabi-v7a:BOOL=ON \
      -DANDROID_BUILD_ABI_x86:BOOL=OFF \
      -DANDROID_BUILD_ABI_x86_64:BOOL=OFF \
      -DANDROID_NATIVE_API_LEVEL:STRING=21 \
      -DANDROID_NDK:PATH=$ANDROID_NDK \
      -DANDROID_SDK:PATH=$ANDROID_SDK \
      -DANDROID_STL:STRING=c++_shared \
      -DCMAKE_BUILD_TYPE:STRING=Release \
      -DCMAKE_CXX_COMPILER:STRING=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ \
      -DCMAKE_C_COMPILER:STRING=$ANDROID_NDK/toolchains/llvm/prebuilt/linux-x86_64/bin/clang \
      -DCMAKE_FIND_ROOT_PATH:STRING=$QTDIR \
      -DCMAKE_PREFIX_PATH:STRING=$QTDIR \
      -DCMAKE_TOOLCHAIN_FILE:PATH=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
      -DQT_QMAKE_EXECUTABLE:STRING=$QTDIR/bin/qmake

# This is bizarrely necessary, or else 'android_deployment_settings.json'
# will lack our custom AndroidManifest and the SSL libraries
cmake ..


#
# Build the executable
#

make -j8
make apk
