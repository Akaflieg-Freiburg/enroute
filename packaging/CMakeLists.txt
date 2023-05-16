#
# To comply with legal regulations, we need to make a copy of the Qt sources
# available on the internet. This script does it automatically but only on
# Stefan Kebekus' machines. Afterward, the sources for the precise Qt version
# used for building this binary will be available at
#
# https://cplx.vm.uni-freiburg.de/storage/QtSources/
#

if (UNIX AND NOT ANDROID)
    add_custom_target(distributeQtSource
        COMMAND ssh-cplx wget --quiet http://download.qt.io/archive/qt/${Qt6Core_VERSION_MAJOR}.${Qt6Core_VERSION_MINOR}/${Qt6Core_VERSION}/single/qt-everywhere-src-${Qt6Core_VERSION}.tar.xz
        COMMAND ssh-cplx mv qt-everywhere-src-${Qt6Core_VERSION}.tar.xz /var/www/storage/QtSources
        COMMENT "Downloading Qt sources and making them available on the internet"
        )

    add_subdirectory(flatpak)

    add_custom_target(publish
        DEPENDS distributeQtSource
        DEPENDS flatpak-publish
        )
endif (UNIX AND NOT ANDROID)

