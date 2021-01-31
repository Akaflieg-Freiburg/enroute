#!/bin/bash

#
# This script builds "enroute flight navigation" for the Linux desktop in
# "Debug" mode.  Several sanitizers are switched on.
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
# Run this script in the main directory tree.

#
# Clean
#

rm  -rf build-linux-debug

#
# Build the executable
#

mkdir build-linux-debug
cd build-linux-debug

export ASAN_OPTIONS=detect_leaks=0
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++


cmake \
    -G Ninja\
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_FIND_ROOT_PATH:STRING=$Qt5_DIR_LINUX \
    -DCMAKE_UNITY_BUILD:BOOL=ON \
    ..

ninja
cd ..
