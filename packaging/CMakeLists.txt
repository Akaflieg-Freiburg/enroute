#
# To comply with legal regulations, we need to make a copy of the Qt sources
# available on the internet. This script does it automatically but only on
# Stefan Kebekus' machines. Afterward, the sources for the precise Qt version
# used for building this binary will be available at
#
# https://cplx.vm.uni-freiburg.de/storage/QtSources/
#

add_custom_target(DistributeQtSource
  COMMAND ssh-cplx wget --quiet http://download.qt.io/archive/qt/${Qt5Core_VERSION_MAJOR}.${Qt5Core_VERSION_MINOR}/${Qt5Core_VERSION}/single/qt-everywhere-src-${Qt5Core_VERSION}.tar.xz
  COMMAND ssh-cplx mv qt-everywhere-src-${Qt5Core_VERSION}.tar.xz /var/www/storage/QtSources
  COMMENT "Downloading Qt sources and making them available on the internet"
  )

