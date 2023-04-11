#
# Google Material Icons
#

set(material-design-icons_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/material-design-icons
    CACHE INTERNAL "" FORCE)

#
# openSSL
#

if(ANDROID)
    set(ANDROID_TARGET_ARCH ${CMAKE_ANDROID_ARCH_ABI})
    add_subdirectory(android_openssl)
endif()

#
# MapBox fonts
#

set(mapBoxFont_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/openseamap-gl
    CACHE INTERNAL "" FORCE)

#
# KDSingleAppplication
#

add_subdirectory(KDSingleApplication
                 ${CMAKE_CURRENT_BINARY_DIR}/KDSingleApplication)

#
# Sunset/sunrise computation
#

add_subdirectory(sunset ${CMAKE_CURRENT_BINARY_DIR}/sunset)
