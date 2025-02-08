#
# Set up cmake
#

cmake_minimum_required(VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ExternalProject)
option(QTDEPLOY "Generate and run Qt deployment scripts" OFF)


#
# Project data
#

project(enroute VERSION 2.32.9)
set(APP_ID de.akaflieg_freiburg.enroute)
set(DISPLAY_NAME "Enroute")
math(EXPR PROJECT_VERSION_CODE 10000*${PROJECT_VERSION_MAJOR}+100*${PROJECT_VERSION_MINOR}+${PROJECT_VERSION_PATCH})



#
# Installation
#

include(GNUInstallDirs)


#
# Qt Setup
#

find_package(Qt6 6.8 COMPONENTS Bluetooth Concurrent Core Core5Compat Gui HttpServer LinguistTools Location Positioning Quick QuickControls2 QuickWidgets Sql Svg TextToSpeech Widgets REQUIRED)
set(QT_VERSION ${Qt6Core_VERSION})

if(CMAKE_SYSTEM_NAME STREQUAL "Android")
    find_package(Qt6 6.8 COMPONENTS Sensors SerialPort WebView REQUIRED)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    find_package(Qt6 6.8 COMPONENTS SerialPort WebView REQUIRED)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "iOS")
    find_package(Qt6 6.8 COMPONENTS Sensors WebView REQUIRED)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(Qt6 6.8 COMPONENTS DBus SerialPort Widgets REQUIRED)
    find_package(Qt6 6.8 COMPONENTS WebView)
endif()

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES de es fr it pl)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(QT_QML_GENERATE_QMLLS_INI ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

qt_policy(SET QTP0002 NEW)
qt_policy(SET QTP0004 NEW)


#
# External Packages
#

find_package(libzip REQUIRED)
find_package(Git REQUIRED)
find_package(QMapLibre COMPONENTS Location REQUIRED)


if(IOS)
    set(PRODUCT_BUNDLE_IDENTIFIER de.akafliegfreiburg.enroute)
    set(CMAKE_XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon)
    include(CMakeLists_ios.txt OPTIONAL)
endif()


#
# Find GIT repository data.
#

if (EXISTS ${CMAKE_SOURCE_DIR}/.git)
    # Get the current working branch
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    message("Currently working with git branch ${GIT_BRANCH}")

    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    message("Currently working with git Commit ${GIT_COMMIT}")

    # Get the latest commit date of the working branch
    execute_process(
        COMMAND git log -1 --format=%as
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_DATE
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    message("Currently working with git commit from ${GIT_DATE}")
else()
    message("Not working from a GIT directory")
    set(GIT_BRANCH "")
    set(GIT_COMMIT "")
    set(GIT_DATE "")
endif()


#
# Subdirectories
#

add_subdirectory(3rdParty)
add_subdirectory(generatedSources)
add_subdirectory(metadata)
add_subdirectory(packaging)
add_subdirectory(src)
