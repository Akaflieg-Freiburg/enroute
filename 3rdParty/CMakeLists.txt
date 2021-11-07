#
# Google Material Icons
#

set(material-design-icons_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/material-design-icons CACHE INTERNAL "" FORCE)


#
# Nitroshare's QHTTPEngine
#

#set(qhttpengine_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/qhttpengine CACHE INTERNAL "" FORCE)
#add_subdirectory(${qhttpengine_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/qhttpengine)


#
# openSSL
#

if (ANDROID)
  add_subdirectory(android_openssl)
endif()


#
# MapBox fonts
#

set(mapBoxFont_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openseamap-gl CACHE INTERNAL "" FORCE)


#
# KDSingleAppplication
#

#add_subdirectory(KDSingleApplication ${CMAKE_CURRENT_BINARY_DIR}/KDSingleApplication)


#
# Sunset/sunrise computation
#

add_subdirectory(sunset ${CMAKE_CURRENT_BINARY_DIR}/sunset)
