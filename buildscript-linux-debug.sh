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

rm -rf build-debug

#
# Build the executable
#

mkdir build-debug
cd build-debug

export ASAN_OPTIONS=detect_leaks=0
export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_FLAGS="-fsanitize=address -fsanitize=undefined" \
    -DCMAKE_CXX_FLAGS="-fsanitize=address -fsanitize=undefined -Werror -Wall -Wextra" \
    -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address -fsanitize=undefined" \
    -DCMAKE_INSTALL_PREFIX=/ \
    -DCMAKE_MODULE_LINKER_FLAGS="-fsanitize=address -fsanitize=undefined" \
    ..

make -j9
cd ..
