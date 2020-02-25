#
# Google Material Icons
#

set(material-design-icons_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/material-design-icons CACHE INTERNAL "" FORCE)


#
# Nitroshare's QHTTPEngine
#

set(qhttpengine_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qhttpengine CACHE INTERNAL "" FORCE)
add_subdirectory(${qhttpengine_SOURCE_DIR} ${qhttpengine_SOURCE_DIR}-build)


#
# openSSL
#

if (ANDROID)
  set(openssl_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android_openssl CACHE INTERNAL "" FORCE)
endif()


#
# MapBox fonts
#

execute_process(
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/generateFontQRC.py
    )
set(flightMapFontsQRC ${CMAKE_CURRENT_BINARY_DIR}/flightMap-fonts.qrc CACHE INTERNAL "" FORCE)
