#
# Google Material Icons
#

set(material-design-icons_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/material-design-icons
    CACHE INTERNAL "" FORCE)

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
