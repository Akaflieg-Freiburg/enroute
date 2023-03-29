#!/bin/bash
set -e

export ANDROID_NDK_HOME=$ANDROID_NDK_ROOT
export PATH=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH

mkdir -p qt6/build
cd qt6/build

for VARIABLE in  arm64_v8a x86 x86_64 armv7
do
    PREFIX=$PWD/../android_$VARIABLE

    if [[ $VARIABLE = "armv7" ]]
    then
	SSLPLATFORM=android-arm
	ABI=armeabi-v7a
    fi
    if [[ $VARIABLE = "arm64_v8a" ]]
    then
	SSLPLATFORM=android-arm64
	ABI=arm64-v8a
    fi
    if [[ $VARIABLE = "x86_64" ]]
    then
	SSLPLATFORM=android-x86_64
	ABI=x86_64
    fi
    if [[ $VARIABLE = "x86" ]]
    then
	SSLPLATFORM=android-x86
	ABI=x86
    fi

    rm -rf * .ninja_* .qt
    $Qt6_DIR_BASE/../Tools/OpenSSL/src/Configure \
	$SSLPLATFORM \
	--prefix=$PREFIX
    make -j8
    make install
    
    rm -rf * .ninja_* .qt
    $Qt6_DIR_BASE/Src/configure \
	-platform android-clang \
	-prefix $PREFIX \
	-android-ndk $ANDROID_NDK_ROOT \
	-android-sdk $ANDROID_SDK_ROOT \
	-android-abis $ABI \
	-qt-host-path $Qt6_DIR_LINUX \
	-no-feature-assistant \
	-no-feature-designer \
	-no-feature-imageformat_bmp \
	-no-feature-quickcontrols2-fusion \
	-no-feature-quickcontrols2-imagine \
	-no-feature-quicktemplates2-hover \
	-ssl \
	-submodules qtbase \
	-submodules qtdeclarative \
	-submodules qtimageformats \
	-submodules qtpositioning \
	-submodules qtshadertools \
	-submodules qtsvg \
	-submodules qtwebview \
	-- \
	-DOPENSSL_ROOT_DIR=$PREFIX \
	-DOPENSSL_INCLUDE_DIR=$PREFIX/include \
	-DOPENSSL_LIBRARIES=$PREFIX/lib
    cmake --build . --parallel
    cmake --install .

    rm -rf * .ninja_* .qt
    $PREFIX/bin/qt-configure-module $Qt6_DIR_BASE/Src/qt5compat
    cmake -DMBGL_QT_WITH_INTERNAL_ICU:BOOL=On .
    cmake --build . --parallel
    cmake --install .

    rm -rf * .ninja_* .qt
    $PREFIX/bin/qt-configure-module $Qt6_DIR_BASE/Src/qttranslations
    cmake --build . --parallel
    cmake --install .

    rm -rf * .ninja_* .qt
    $PREFIX/bin/qt-configure-module ../../../qtlocation
    cmake -DMBGL_QT_WITH_INTERNAL_ICU:BOOL=On .
    cmake --build . --parallel
    cmake --install .

done
