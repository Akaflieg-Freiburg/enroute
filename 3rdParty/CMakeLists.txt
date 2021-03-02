#
# Google Material Icons
#

set(material-design-icons_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/material-design-icons CACHE INTERNAL "" FORCE)


#
# Nitroshare's QHTTPEngine
#

set(qhttpengine_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qhttpengine CACHE INTERNAL "" FORCE)
add_subdirectory(${qhttpengine_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/qhttpengine)


#
# openSSL
#

if (ANDROID)
  set(openssl_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/android_openssl/latest CACHE INTERNAL "" FORCE)
endif()


#
# MapBox fonts
#

set(mapBoxFont_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openseamap-gl CACHE INTERNAL "" FORCE)


#
# KDSingleAppplication
#

add_subdirectory(KDSingleApplication ${CMAKE_CURRENT_BINARY_DIR}/KDSingleApplication)


#
# Sunset/sunrise computation
#

add_subdirectory(sunset ${CMAKE_CURRENT_BINARY_DIR}/sunset)


#
# Guideline support library
#

add_subdirectory(GSL ${CMAKE_CURRENT_SOURCE_DIR}/GSL)
