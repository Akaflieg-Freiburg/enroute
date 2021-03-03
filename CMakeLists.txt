#
# Set up cmake
#

cmake_minimum_required(VERSION 3.16)
include(ExternalProject)
option(BUILD_DOC "Build developer documentation" OFF)


#
# Project data
#

project(enroute VERSION 2.4.2)
set(APP_ID de.akaflieg_freiburg.enroute)
math(EXPR PROJECT_VERSION_CODE 10000*${PROJECT_VERSION_MAJOR}+100*${PROJECT_VERSION_MINOR}+${PROJECT_VERSION_PATCH})
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")


#
# Installation
#

include(GNUInstallDirs)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
find_package(Git REQUIRED)
find_package(Qt5 5.15 COMPONENTS Concurrent Core Gui LinguistTools Positioning Quick QuickWidgets Sql Svg REQUIRED)
if( ANDROID )
    find_package(Qt5 5.15 COMPONENTS AndroidExtras REQUIRED)
    find_package(Qt5QuickCompiler)
else()
    find_package(Qt5 5.15 COMPONENTS Widgets REQUIRED)
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
        COMMAND git log -1 --format=%H
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
