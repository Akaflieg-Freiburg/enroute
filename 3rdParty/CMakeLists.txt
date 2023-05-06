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

# For simplicity, we do not build the subset library, but include the source files
# into the enroute target. This avoids problems on iOS and saves us the overhead of
# finding and loading the library at runtime.
