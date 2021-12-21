#!/bin/bash

#
# This script builds "enroute flight navigation" for Android in debug mode.
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

rm -rf build6-android-debug
mkdir -p build6-android-debug
cd build6-android-debug

#
# Configure
#

cmake .. \
      -G Ninja\
      -DCMAKE_BUILD_TYPE:STRING=Debug \
      -DCMAKE_PREFIX_PATH:STRING=$Qt6_DIR_ANDROID\_x86 \
      -DOPENSSL_ROOT_DIR:PATH=$OPENSSL_ROOT_DIR \
      -DANDROID_NATIVE_API_LEVEL:STRING=23 \
      -DANDROID_NDK:PATH=$ANDROID_NDK_ROOT \
      -DCMAKE_TOOLCHAIN_FILE:PATH=$ANDROID_NDK_ROOT/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI:STRING=x86 \
      -DANDROID_STL:STRING=c++_shared \
      -DCMAKE_FIND_ROOT_PATH:PATH=$Qt6_DIR_ANDROID\_x86 \
      -DQT_HOST_PATH:PATH=$Qt6_DIR_LINUX \
      -DANDROID_SDK_ROOT:PATH=$ANDROID_SDK_ROOT


# This is bizarrely necessary, or else 'android_deployment_settings.json'
# will lack our custom AndroidManifest and the SSL libraries
cmake ..


#
# Build the executable
#

ninja

echo "Build APK"
ninja apk
