#
# Google Material Icons
#

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/material-design-icons-3.0.1)
  message(STATUS "Downloading Google Material Design Icons")
  execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND curl -s https://codeload.github.com/google/material-design-icons/tar.gz/3.0.1
    COMMAND tar -xz --wildcards *production*.svg
    )
endif()
set(material-design-icons_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/material-design-icons-3.0.1 CACHE INTERNAL "" FORCE)


#
# Nitroshare's QHTTPEngine
#

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/qhttpengine-1.0.1)
  message(STATUS "Downloading Nitroshare's QHTTPEngine")
  execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND curl -s https://codeload.github.com/nitroshare/qhttpengine/tar.gz/1.0.1
    COMMAND tar -xz 
    )
endif()
set(qhttpengine_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/qhttpengine-1.0.1 CACHE INTERNAL "" FORCE)
add_subdirectory(${qhttpengine_SOURCE_DIR} ${qhttpengine_SOURCE_DIR}-build)


#
# openSSL
#

if (ANDROID)
  ExternalProject_Add(openssl
    GIT_REPOSITORY https://github.com/KDAB/android_openssl.git
    GIT_TAG master
    GIT_SHALLOW true
    GIT_PROGRESS true
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    )
  set(openssl_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl-prefix/src/openssl CACHE INTERNAL "" FORCE)
endif()


#
# MapBox fonts
#

message(STATUS "Downloading Google Roboto Fonts")
execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND git clone https://github.com/aAXEe/openseamap-gl.git --depth 1 --quiet
    )
execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/generateFontQRC.py
    )
set(flightMapFontsQRC ${CMAKE_CURRENT_BINARY_DIR}/flightMap-fonts.qrc CACHE INTERNAL "" FORCE)
