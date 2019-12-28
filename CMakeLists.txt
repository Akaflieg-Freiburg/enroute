#
# Set up cmake
#

cmake_minimum_required(VERSION 3.13)
include(ExternalProject)
include(GNUInstallDirs)


#
# Project data
#

project(enroute VERSION 0.2.1)
set(APP_ID de.akaflieg_freiburg.enroute)
set(PROJECT_DATE 2019-12-22)
math(EXPR PROJECT_VERSION_CODE 10000*${PROJECT_VERSION_MAJOR}+100*${PROJECT_VERSION_MINOR}+${PROJECT_VERSION_PATCH})
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")


#
# Qt Setup
#

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#
# External Packages
#

find_package(Doxygen)
find_package(Qt5 5.12 COMPONENTS Core Gui Positioning Quick QuickWidgets Sql Svg REQUIRED)
if( ANDROID )
  find_package(Qt5 5.14 COMPONENTS AndroidExtras REQUIRED)
endif()


# Create target collecting all documentation
add_custom_target(doc)

# Subdirectories
add_subdirectory(3rdParty)
add_subdirectory(metadata)
add_subdirectory(src)
