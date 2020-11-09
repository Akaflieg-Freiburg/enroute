#!/bin/bash

#
# This script builds "enroute flight navigation" for Android in release mode.
#
# See https://github.com/Akaflieg-Freiburg/enroute/wiki/Build-scripts
#

#
# Copyright © 2020 Stefan Kebekus <stefan.kebekus@math.uni-freiburg.de>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place - Suite 330, Boston, MA 02111-1307, USA.
#


#
# Fail on first error
#

set -e

#
# Clean up
#

rm -rf build-android-release
mkdir -p build-android-release
cd build-android-release

#
# Configure
#

cmake .. \
      -G Ninja\
      -DANDROID_ABI:STRING=armeabi-v7a \
      -DANDROID_BUILD_ABI_arm64-v8a:BOOL=ON \
      -DANDROID_BUILD_ABI_armeabi-v7a:BOOL=ON \
      -DANDROID_BUILD_ABI_x86:BOOL=ON \
      -DANDROID_BUILD_ABI_x86_64:BOOL=ON \
      -DANDROID_NATIVE_API_LEVEL:STRING=21 \
      -DANDROID_NDK:PATH=$ANDROID_NDK_ROOT \
      -DANDROID_SDK:PATH=$ANDROID_SDK_ROOT \
      -DANDROID_STL:STRING=c++_shared \
      -DCMAKE_BUILD_TYPE:STRING=RelWithDebInfo \
      -DCMAKE_CXX_COMPILER:STRING=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/clang++ \
      -DCMAKE_CXX_FLAGS="-Werror -Wall -Wextra" \
      -DCMAKE_C_COMPILER:STRING=$ANDROID_NDK_ROOT/toolchains/llvm/prebuilt/linux-x86_64/bin/clang \
      -DCMAKE_FIND_ROOT_PATH:STRING=$Qt5_DIR_ANDROID \
      -DCMAKE_PREFIX_PATH:STRING=$Qt5_DIR_ANDROID \
      -DCMAKE_TOOLCHAIN_FILE:PATH=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake

# This is bizarrely necessary, or else 'android_deployment_settings.json'
# will lack our custom AndroidManifest and the SSL libraries
cmake ..


#
# Build the executable
#

ninja

echo "Build APK"
ninja apk

echo "Build AAB"
ninja aab

#$Qt5_DIR_ANDROID/bin/androiddeployqt --input android_deployment_settings.json --output android-build --release --apk enroute-release-unsigned.apk
#echo "Unsigned APK file is available at $PWD/enroute-release-unsigned.apk"

if [ -z "$ANDROID_KEYSTORE_FILE" -o -z "$ANDROID_KEYSTORE_PASS" ]
then
    echo "Not signing APK because \$ANDROID_KEYSTORE_FILE or \$ANDROID_KEYSTORE_PASS not set"
else
    echo "Signing APK"
    $ANDROID_SDK_ROOT/build-tools/28.0.3/apksigner sign \
						   --ks $ANDROID_KEYSTORE_FILE \
						   --ks-pass pass:$ANDROID_KEYSTORE_PASS \
						   --in android-build/enroute.apk \
						   --out enroute-release-signed.apk
    echo "Signed APK file is available at $PWD/enroute-release-signed.apk"
    echo
    echo "Signing AAB"
    jarsigner -keystore $ANDROID_KEYSTORE_FILE \
	      -storepass $ANDROID_KEYSTORE_PASS \
	      android-build/build/outputs/bundle/release/android-build-release.aab "Stefan Kebekus"
    cp android-build/build/outputs/bundle/release/android-build-release.aab enroute-release-signed.aab
    
    echo "Signed AAB file is available at $PWD/enroute-release-signed.aab"
fi
