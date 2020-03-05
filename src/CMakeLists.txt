#
# Configure files
#

configure_file(flightMap.qrc.in flightMap.qrc)
configure_file(${CMAKE_SOURCE_DIR}/generatedSources/flightMap/flightMap-fonts.qrc.in flightMapFonts.qrc)
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
# Set up sources
#
set(SOURCES
    # Ressources
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/flightMapFonts.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/icons.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/qml.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/ressources.qrc
    
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
        ${openssl_SOURCE_DIR}/x86/libssl_1_1.so
        ${openssl_SOURCE_DIR}/x86_64/libcrypto_1_1.so
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
