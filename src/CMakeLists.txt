#
# Configure files
#

qt5_add_translation(TRANSLATIONS
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_cz.ts
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_de.ts
     )
message(${TRANSLATIONS})

configure_file(flightMap.qrc.in flightMap.qrc)
configure_file(${CMAKE_SOURCE_DIR}/generatedSources/flightMap/flightMap-fonts.qrc.in flightMapFonts.qrc)
configure_file(icons.qrc.in icons.qrc)
configure_file(qml.qrc.in qml.qrc)
configure_file(ressources.qrc.in ressources.qrc)

configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/bugReport.html.in text/bugReport.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/bugReport_cz.html.in text/bugReport_cz.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/bugReport_de.html.in text/bugReport_de.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/firstStart.html.in text/firstStart.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/firstStart_cz.html.in text/firstStart_cz.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/firstStart_de.html.in text/firstStart_de.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/flightRouteLibraryInfo.html.in text/flightRouteLibraryInfo.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/flightRouteLibraryInfo_cz.html.in text/flightRouteLibraryInfo_cz.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/flightRouteLibraryInfo_de.html.in text/flightRouteLibraryInfo_de.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/missingPermissions.html.in text/missingPermissions.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/missingPermissions_cz.html.in text/missingPermissions_cz.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/missingPermissions_de.html.in text/missingPermissions_de.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/info_author.html.in text/info_author.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/info_author_cz.html.in text/info_author_cz.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/info_author_de.html.in text/info_author_de.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/info_enroute.html.in text/info_enroute.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/info_enroute_cz.html.in text/info_enroute_cz.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/info_enroute_de.html.in text/info_enroute_de.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/info_license.html.in text/info_license.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/info_license_cz.html.in text/info_license_cz.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/info_license_de.html.in text/info_license_de.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/participate.html.in text/participate.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/participate_cz.html.in text/participate_cz.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/participate_de.html.in text/participate_de.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/whatsnew.html.in text/whatsnew.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/whatsnew_cz.html.in text/whatsnew_cz.html)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/html/whatsnew_de.html.in text/whatsnew_de.html)


#
# Set up sources
#

# We use this macro here to avoid creating extremely large C++ files with binary content
qt5_add_big_resources(BIGRESOURCES ${CMAKE_CURRENT_BINARY_DIR}/flightMapFonts.qrc ${CMAKE_CURRENT_BINARY_DIR}/ressources.qrc)

set(SOURCES
    # Ressources
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/icons.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/qml.qrc
    ${TRANSLATIONS}
    ${BIGRESOURCES}

    # C++ files
    Aircraft.cpp
    Airspace.cpp
    AviationUnits.cpp
    Downloadable.cpp
    DownloadableGroup.cpp
    DownloadableGroupWatcher.cpp
    FlightRoute.cpp
    FlightRoute_GPX.cpp
    FlightRoute_Leg.cpp
    Geoid.cpp
    GeoMapProvider.cpp
    GlobalSettings.cpp
    Librarian.cpp
    main.cpp
    MapManager.cpp
    MobileAdaptor.cpp
    MobileAdaptor_share.cpp
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
    list(APPEND SOURCES
        android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java
        )

    # Configure files
    configure_file(AndroidManifest.xml.in android/AndroidManifest.xml)
    configure_file(android/build.gradle android/build.gradle COPYONLY)
    configure_file(android/gradle/wrapper/gradle-wrapper.properties android/gradle/wrapper/gradle-wrapper.properties COPYONLY)
    configure_file(android/gradle/wrapper/gradle-wrapper.jar android/gradle/wrapper/gradle-wrapper.jar COPYONLY)
    configure_file(android/gradlew android/gradlew COPYONLY)
    configure_file(android/gradlew.bat android/gradlew.bat COPYONLY)
    configure_file(android/res/drawable/splash.xml android/res/drawable/splash.xml COPYONLY)
    configure_file(android/res/values/apptheme.xml android/res/values/apptheme.xml COPYONLY)
    configure_file(android/res/values/apptheme.xml android/res/values/apptheme.xml COPYONLY)
    configure_file(android/res/xml/filepaths.xml android/res/xml/filepaths.xml COPYONLY)
    configure_file(android/src/de/akaflieg_freiburg/enroute/IntentLauncher.java android/src/de/akaflieg_freiburg/enroute/IntentLauncher.java COPYONLY)
    configure_file(android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java COPYONLY)
    configure_file(android/src/de/akaflieg_freiburg/enroute/ShareActivity.java android/src/de/akaflieg_freiburg/enroute/ShareActivity.java COPYONLY)
    configure_file(android/src/de/akaflieg_freiburg/enroute/ShareUtils.java android/src/de/akaflieg_freiburg/enroute/ShareUtils.java COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/file/drawable-hdpi/ic_file_download_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_file_download.png COPYONLY)

    # Generate android extra sources. This includes icons, but also a few other files
    set(ANDROID_EXTRA_SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xxxhdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xxhdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-xhdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-hdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable-mdpi/icon.png
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/splash.xml
        ${CMAKE_CURRENT_BINARY_DIR}/android/res/values/apptheme.xml
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
