#
# Set up cmake
#

cmake_minimum_required(VERSION 3.13)
include(ExternalProject)
option(BUILD_DOC "Build documentation" OFF)


#
# Project data
#

project(enroute VERSION 1.7.0)
set(APP_ID de.akaflieg_freiburg.enroute)
set(PROJECT_DATE 2020-05-20)
math(EXPR PROJECT_VERSION_CODE 10000*${PROJECT_VERSION_MAJOR}+100*${PROJECT_VERSION_MINOR}+${PROJECT_VERSION_PATCH})
add_compile_definitions(PROJECT_VERSION="${PROJECT_VERSION}")


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
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


#
# External Packages
#

find_package(Doxygen)
find_package(Qt5 5.14 COMPONENTS Concurrent Core Gui LinguistTools Positioning Quick QuickWidgets Sql Svg REQUIRED)
if( ANDROID )
    find_package(Qt5 5.14 COMPONENTS AndroidExtras REQUIRED)
else()
    find_package(Qt5 5.14 COMPONENTS Widgets REQUIRED)
endif()

# Create target collecting all documentation
if ( BUILD_DOC )
    add_custom_target(fulldoc ALL)
else()
    add_custom_target(fulldoc)
endif()

# Find GIT commit, if git is available and if we are working in a git source
# tree

if(EXISTS ${CMAKE_SOURCE_DIR}/.git)
    find_package(Git)
    if(Git_FOUND)
        message("Building inside a GIT directory, git found at ${GIT_EXECUTABLE}")
        execute_process(COMMAND git --git-dir=${CMAKE_SOURCE_DIR}/.git log --pretty=format:'%H' -n1
            OUTPUT_VARIABLE GIT_COMMIT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        message("Currently working with git Commit ${GIT_COMMIT}")
    else()
        message("Git not found.")
    endif()
endif()


# Subdirectories
add_subdirectory(3rdParty)
add_subdirectory(generatedSources)
add_subdirectory(metadata)
add_subdirectory(packaging)
add_subdirectory(src)
