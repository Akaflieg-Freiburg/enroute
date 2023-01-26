#!/bin/bash
set -e


mkdir -p qt6/build
cd qt6/build

for VARIABLE in armv7 arm64_v8a x86 x86_64
do
    rm -rf * .ninja_* .qt
    $Qt6_DIR_BASE/Src/configure \
	-platform android-clang \
	-prefix ../android_$VARIABLE \
	-android-ndk $ANDROID_NDK_ROOT \
	-android-sdk $ANDROID_SDK_ROOT \
	-android-abis x86_64 \
	-qt-host-path $Qt6_DIR_LINUX \
	-I /home/kebekus/Software/buildsystems/openssl-1.1.1k/include \
	-no-feature-assistant \
	-no-feature-designer \
	-no-feature-imageformat_bmp \
	-no-feature-quickcontrols2-fusion \
	-no-feature-quickcontrols2-imagine \
	-no-feature-quicktemplates2-hover \
	-submodules qtbase \
	-submodules qtdeclarative \
	-submodules qtimageformats \
	-submodules qtpositioning \
	-submodules qtshadertools \
	-submodules qtsvg \
	-submodules qtwebview
    cmake --build . --parallel
    cmake --install .

    rm -rf * .ninja_* .qt
    ../android_$VARIABLE/bin/qt-configure-module $Qt6_DIR_BASE/Src/qt5compat
    cmake -DMBGL_QT_WITH_INTERNAL_ICU:BOOL=On .
    cmake --build . --parallel
    cmake --install .

    rm -rf * .ninja_* .qt
    ../android_$VARIABLE/bin/qt-configure-module $Qt6_DIR_BASE/Src/qttranslations
    cmake --build . --parallel
    cmake --install .

    rm -rf * .ninja_* .qt
    ../android_$VARIABLE/bin/qt-configure-module ../../../qtlocation
    cmake -DMBGL_QT_WITH_INTERNAL_ICU:BOOL=On .
    cmake --build . --parallel
    cmake --install .

done
