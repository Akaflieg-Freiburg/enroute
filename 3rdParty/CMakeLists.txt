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
# KDSingleApplication
#

# For simplicity, we do not build the KDSingleApplication library, but include the source files
# into the enroute target. This saves us the overhead of
# finding and loading the library at runtime.


#
# Sunset/sunrise computation
#

# For simplicity, we do not build the sunset library, but include the source files
# into the enroute target. This avoids problems on iOS and saves us the overhead of
# finding and loading the library at runtime.


#
# spriteGenerator
#

# Build the spriteGenerator tool only when the flightMapSprites target is invoked.
# It is a host-only developer tool and must not be included in mobile builds.
if(NOT ANDROID AND NOT IOS)
    add_subdirectory(spriteGenerator EXCLUDE_FROM_ALL)
endif()


#
# enrouteOGN library
#

# Build the enrouteOGN library as a static library
add_subdirectory(enrouteOGN)

