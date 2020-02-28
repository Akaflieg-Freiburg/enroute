#
# Configure files
#

configure_file(flightMap.qrc.in flightMap.qrc)
configure_file(icons.qrc.in icons.qrc)
configure_file(qml.qrc.in qml.qrc)
configure_file(ressources.qrc.in ressources.qrc)

configure_file(text/bugReport.html.in text/bugReport.html)
configure_file(text/firstStart.html.in text/firstStart.html)
configure_file(text/info_author.html.in text/info_author.html)
configure_file(text/info_enroute.html.in text/info_enroute.html)
configure_file(text/info_license.html.in text/info_license.html)
configure_file(text/participate.html.in text/participate.html)
configure_file(text/whatsnew.html.in text/whatsnew.html)


#
# Generate PNG versions of the main program icon
#

add_custom_command(
  OUTPUT ${CMAKE_BINARY_DIR}/metadata/${APP_ID}_angular.png ${CMAKE_BINARY_DIR}/metadata/${APP_ID}.png 
  DEPENDS ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}.svg ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}_angular.svg
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}_angular.svg -o ${CMAKE_BINARY_DIR}/metadata/${APP_ID}_angular.png -h 512 -w 512
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}.svg -o ${CMAKE_BINARY_DIR}/metadata/${APP_ID}.png -h 512 -w 512
  COMMENT "Generate main program icons"
  )


#
# Generate PNG versions of the aviation symbols
#

set(AVIATION_ICONS
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-GLD.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-GLD@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-GRASS.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-GRASS@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-INOP.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-INOP@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL-GRASS.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL-GRASS@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL-PAVED.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL-PAVED@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-PAVED.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-PAVED@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-UL.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-UL@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-WATER.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-WATER@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/MRP.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/MRP@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/NDB.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/NDB@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/RP.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/RP@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VOR.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VOR@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VORDME.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VORDME@2x.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VORTAC.png
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VORTAC@2x.png
    )
add_custom_command(
    OUTPUT ${AVIATION_ICONS}
    COMMAND mkdir -p ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD.png -h 30 -w 30
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD@2x.png -h 60 -w 60
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-GLD.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-GLD.png -h 20 -w 20
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-GLD.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-GLD@2x.png -h 40 -w 40
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-GRASS.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-GRASS.png -h 30 -w 30
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-GRASS.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-GRASS@2x.png -h 60 -w 60
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-INOP.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-INOP.png -h 30 -w 30
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-INOP.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-INOP@2x.png -h 60 -w 60
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL.png -h 30 -w 30
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL@2x.png -h 60 -w 60
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL-GRASS.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL-GRASS.png -h 30 -w 30
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL-GRASS.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL-GRASS@2x.png -h 60 -w 60
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL-PAVED.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL-PAVED.png -h 30 -w 30
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL-PAVED.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-MIL-PAVED@2x.png -h 60 -w 60
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-PAVED.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-PAVED.png -h 30 -w 30
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-PAVED.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-PAVED@2x.png -h 60 -w 60
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-UL.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-UL.png -h 20 -w 20
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-UL.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-UL@2x.png -h 40 -w 40
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-WATER.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-WATER.png -h 30 -w 30
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-WATER.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/AD-WATER@2x.png -h 60 -w 60
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/MRP.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/MRP.png -h 25 -w 25
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/MRP.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/MRP@2x.png -h 50 -w 50
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/NDB.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/NDB.png -h 25 -w 25
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/NDB.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/NDB@2x.png -h 50 -w 50
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/RP.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/RP.png -h 25 -w 25
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/RP.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/RP@2x.png -h 50 -w 50
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VOR.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VOR.png -w 22
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VOR.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VOR@2x.png -w 44
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VORDME.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VORDME.png -w 22
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VORDME.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VORDME@2x.png -w 44
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VORTAC.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VORTAC.png -w 26
    COMMAND rsvg-convert ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VORTAC.svg -o ${CMAKE_CURRENT_BINARY_DIR}/flightMap/sprites/VORTAC@2x.png -w 52
    COMMENT Generating Icons
    )


#
# Set up sources
#

set(SOURCES
    # Ressources
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap.qrc
    ${flightMapFontsQRC}
    ${CMAKE_CURRENT_BINARY_DIR}/icons.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/qml.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/ressources.qrc

    # List icons here to make sure that they are built
    ${AVIATION_ICONS}
    ${CMAKE_BINARY_DIR}/metadata/${APP_ID}.png
    ${CMAKE_BINARY_DIR}/metadata/${APP_ID}.svg
    
    # C++ files
    Aircraft.cpp
    Airspace.cpp
    AviationUnits.cpp
    Downloadable.cpp
    FlightRoute.cpp
    FlightRoute_Leg.cpp
    GeoMapProvider.cpp
    GlobalSettings.cpp
    main.cpp
    MapManager.cpp
    MobileAdaptor.cpp
    SatNav.cpp
    ScaleQuickItem.cpp
    TileHandler.cpp
    TileServer.cpp
    Waypoint.cpp
    Wind.cpp
    )


#
# Generate android executable
#
if( ANDROID )
    # Configure AndroidManifest.xml
    configure_file(AndroidManifest.xml.in android/AndroidManifest.xml)

    # Generate android extra sources. This includes icons, but also a few other files
    set(ANDROID_EXTRA_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xxxhdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xxhdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xhdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-hdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-mdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/splash.xml
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/values/apptheme.xml
        ${CMAKE_CURRENT_BINARY_DIR}/android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java
        )
    add_custom_command(
        OUTPUT ${ANDROID_EXTRA_SOURCES}
        DEPENDS ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}.svg
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/android android
        COMMAND ${CMAKE_COMMAND} -E make_directory android/res/drawable-xxxhdpi
        COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}.svg -o ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xxxhdpi/icon.png -h 384 -w 384
        COMMAND ${CMAKE_COMMAND} -E make_directory android/res/drawable-xxhdpi
        COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}.svg -o ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xxhdpi/icon.png -h 288 -w 288
        COMMAND ${CMAKE_COMMAND} -E make_directory android/res/drawable-xhdpi
        COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}.svg -o ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xhdpi/icon.png -h 192 -w 192
        COMMAND ${CMAKE_COMMAND} -E make_directory android/res/drawable-hdpi
        COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}.svg -o ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-hdpi/icon.png -h 144 -w 144
        COMMAND ${CMAKE_COMMAND} -E make_directory android/res/drawable-mdpi
        COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}.svg -o ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-mdpi/icon.png -h 96 -w 96
        COMMENT "Generate android icons"
        )

    # The android target require a few additional variables
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/android" CACHE INTERNAL "")

    # The android target depends on our own copy of openssl
    set(ANDROID_EXTRA_LIBS
        ${openssl_SOURCE_DIR}/arm/libcrypto_1_1.so
        ${openssl_SOURCE_DIR}/arm/libssl_1_1.so
        ${openssl_SOURCE_DIR}/arm64/libcrypto_1_1.so
        ${openssl_SOURCE_DIR}/arm64/libssl_1_1.so
        ${openssl_SOURCE_DIR}/x86/libcrypto_1_1.so
        ${openssl_SOURCE_DIR}/x86/libssl_1_1.so;${openssl_SOURCE_DIR}/x86_64/libcrypto_1_1.so
        ${openssl_SOURCE_DIR}/x86_64/libssl_1_1.so
        CACHE INTERNAL ""
        )

    # The android target is a library, not an executable
    add_library(${PROJECT_NAME} SHARED ${SOURCES} ${ANDROID_EXTRA_SOURCES})

    # Add libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::AndroidExtras Qt5::Core Qt5::Positioning Qt5::Quick Qt5::Sql Qt5::Svg qhttpengine)
endif()


#
# Generate unix executable
#
if (NOT ANDROID)
    # Add executable and libraries
    add_executable(${PROJECT_NAME} ${SOURCES})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Positioning Qt5::Quick Qt5::Sql Qt5::Svg qhttpengine)

    # Install
    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})    
endif()


#
# Generate documentation
#
configure_file(Doxyfile.in Doxyfile)
add_custom_target(enroute_doc COMMAND doxygen)
add_dependencies(fulldoc enroute_doc)
