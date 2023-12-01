#
# Set up cmake
#
set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
cmake_minimum_required(VERSION 3.16)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(ExternalProject)
option(BUILD_DOC "Build developer documentation" OFF)




#
# Project data
#

project(enroute VERSION 2.29.51)
set(APP_ID de.akaflieg_freiburg.enroute)
set(DISPLAY_NAME "Enroute")
math(EXPR PROJECT_VERSION_CODE 10000*${PROJECT_VERSION_MAJOR}+100*${PROJECT_VERSION_MINOR}+${PROJECT_VERSION_PATCH})
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("CMAKE_OSX_DEPLOYMENT_TARGET: ${CMAKE_OSX_DEPLOYMENT_TARGET}")

#
# Installation
#

include(GNUInstallDirs)


#
# Qt Setup
#

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


#
# External Packages
#

find_package(Doxygen)
find_package(libzip REQUIRED)
find_package(Git REQUIRED)
find_package(Qt6 COMPONENTS Concurrent Core Core5Compat Gui HttpServer LinguistTools Location Positioning Quick QuickControls2 QuickWidgets Sql Svg TextToSpeech Widgets REQUIRED)
find_package(QMapLibre COMPONENTS Location REQUIRED)

if( ANDROID OR IOS)
    find_package(Qt6 COMPONENTS Sensors WebView REQUIRED)
else()
    find_package(Qt6 COMPONENTS DBus Widgets REQUIRED)
endif()

if(IOS)
    set(PRODUCT_BUNDLE_IDENTIFIER de.akafliegfreiburg.enroute)
    set(CMAKE_XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon)
    include(CMakeLists_ios.txt OPTIONAL)
endif()

#
# Create target collecting all documentation
#

if ( BUILD_DOC )
    add_custom_target(fulldoc ALL)
else()
    add_custom_target(fulldoc)
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
add_compile_definitions(GIT_COMMIT="${GIT_COMMIT}")


#
# Subdirectories
#

add_subdirectory(3rdParty)
add_subdirectory(generatedSources)
add_subdirectory(metadata)
add_subdirectory(packaging)
add_subdirectory(src)
