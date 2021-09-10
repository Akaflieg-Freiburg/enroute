#
# Translations
#

# The Qt library translations are a mess. We try to find the files at various
# places in the file system, generate our own *.qm files with lrelease and then
# use the lconvert tool to concatenate the *.qm file with the Qt library
# translations. Resulting files are then called enroute_XX.qm where XX is the
# language code. The variable LANGS contains all language codes that will be
# considered.

find_path(Qt5_TRANSLATION_DIR qtbase_de.qm
    HINTS ${Qt5Core_DIR}/../../../translations
    HINTS /usr/share/qt5/translations
    HINTS /usr/share/qt/translations
    HINTS /usr/translations
    REQUIRED
    )
set(Qt5_LCONVERT_EXECUTABLE Qt5::lconvert)

set(LANGS de fr it pl)
foreach(_lang ${LANGS})
    set(QT_QM_FILES ${Qt5_TRANSLATION_DIR}/qtbase_${_lang}.qm)
    if (EXISTS ${Qt5_TRANSLATION_DIR}/qtdeclarative_${_lang}.qm)
        list(APPEND QT_QM_FILES ${Qt5_TRANSLATION_DIR}/qtdeclarative_${_lang}.qm)
    endif()
    if (EXISTS ${Qt5_TRANSLATION_DIR}/qtlocation_${_lang}.qm)
        list(APPEND QT_QM_FILES ${Qt5_TRANSLATION_DIR}/qtlocation_${_lang}.qm)
    endif()
    add_custom_command(
        OUTPUT enroute_${_lang}.qm
        DEPENDS ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_${_lang}.ts
        COMMAND ${Qt5_LRELEASE_EXECUTABLE} ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_${_lang}.ts -qm enrouteOnly_${_lang}.qm
        COMMAND ${Qt5_LCONVERT_EXECUTABLE} enrouteOnly_${_lang}.qm ${QT_QM_FILES} -o enroute_${_lang}.qm
        COMMENT "Generating translations for language ${_lang}"
        )
endforeach()


#
# Configure files
#

configure_file(flightMap.qrc.in flightMap.qrc)
configure_file(${CMAKE_SOURCE_DIR}/generatedSources/flightMap/flightMap-fonts.qrc.in flightMapFonts.qrc)
configure_file(icons.qrc.in icons.qrc)
configure_file(qml.qrc.in qml.qrc)
configure_file(ressources.qrc.in ressources.qrc)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/knotifications5/enroute\ flight\ navigation.notifyrc.in enroute\ flight\ navigation.notifyrc)


#
# Set up sources
#

set(SOURCES
    # Ressources
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/icons.qrc

    # Translations
    enroute_de.qm
    enroute_fr.qm
    enroute_it.qm
    enroute_pl.qm

    # Other files
    AndroidManifest.xml.in
    flightMap.qrc.in
    icons.qrc.in
    qml.qrc.in
    README.md
    ressources.qrc.in

    # Header files
    Aircraft.h
    Clock.h
    DemoRunner.h
    geomaps/Airspace.h
    geomaps/Downloadable.h
    geomaps/DownloadableGroup.h
    geomaps/DownloadableGroupWatcher.h
    geomaps/GeoMapProvider.h
    geomaps/MapManager.h
    geomaps/TileHandler.h
    geomaps/TileServer.h
    geomaps/Waypoint.h
    Global.h
    Librarian.h
    MobileAdaptor.h
    navigation/FlightRoute.h
    navigation/FlightRoute_Leg.h
    navigation/Navigator.h
    positioning/Geoid.h
    positioning/PositionInfo.h
    positioning/PositionInfoSource_Abstract.h
    positioning/PositionInfoSource_Satellite.h
    positioning/PositionProvider.h
    Settings.h
    traffic/FlarmnetDB.h
    traffic/PasswordDB.h
    traffic/TrafficDataSource_Abstract.h
    traffic/TrafficDataSource_AbstractSocket.h
    traffic/TrafficDataSource_File.h
    traffic/TrafficDataSource_Simulate.h
    traffic/TrafficDataSource_Tcp.h
    traffic/TrafficDataSource_Udp.h
    traffic/TrafficDataProvider.h
    traffic/TrafficFactor_Abstract.h
    traffic/TrafficFactor_DistanceOnly.h
    traffic/TrafficFactor_WithPosition.h
    traffic/Warning.h
    ui/ScaleQuickItem.h
    units/Angle.h
    units/Distance.h
    units/Speed.h
    units/Time.h
    weather/Decoder.h
    weather/METAR.h
    weather/Station.h
    weather/TAF.h
    weather/WeatherDataProvider.h
    weather/Wind.h

    # C++ files
    Aircraft.cpp
    Clock.cpp
    DemoRunner.cpp
    geomaps/Airspace.cpp
    geomaps/Downloadable.cpp
    geomaps/DownloadableGroup.cpp
    geomaps/DownloadableGroupWatcher.cpp
    geomaps/GeoMapProvider.cpp
    geomaps/MapManager.cpp
    geomaps/TileHandler.cpp
    geomaps/TileServer.cpp
    geomaps/Waypoint.cpp
    Global.cpp
    Librarian.cpp
    main.cpp
    MobileAdaptor.cpp
    MobileAdaptor_share.cpp
    navigation/FlightRoute.cpp
    navigation/FlightRoute_GPX.cpp
    navigation/FlightRoute_Leg.cpp
    navigation/Navigator.cpp
    positioning/Geoid.cpp
    positioning/PositionInfo.cpp
    positioning/PositionInfoSource_Abstract.cpp
    positioning/PositionInfoSource_Satellite.cpp
    positioning/PositionProvider.cpp
    Settings.cpp
    traffic/FlarmnetDB.cpp
    traffic/PasswordDB.cpp
    traffic/TrafficDataSource_Abstract.cpp
    traffic/TrafficDataSource_Abstract_FLARM.cpp
    traffic/TrafficDataSource_Abstract_GDL90.cpp
    traffic/TrafficDataSource_Abstract_XGPS.cpp
    traffic/TrafficDataSource_AbstractSocket.cpp
    traffic/TrafficDataSource_File.cpp
    traffic/TrafficDataSource_Simulate.cpp
    traffic/TrafficDataSource_Tcp.cpp
    traffic/TrafficDataSource_Udp.cpp
    traffic/TrafficDataProvider.cpp
    traffic/TrafficFactor_Abstract.cpp
    traffic/TrafficFactor_DistanceOnly.cpp
    traffic/TrafficFactor_WithPosition.cpp
    traffic/Warning.cpp
    ui/ScaleQuickItem.cpp
    units/Angle.cpp
    units/Distance.cpp
    units/Speed.cpp
    units/Time.cpp
    weather/Decoder.cpp
    weather/METAR.cpp
    weather/Station.cpp
    weather/TAF.cpp
    weather/WeatherDataProvider.cpp
    weather/Wind.cpp

    ${HEADERS}
    )

#
# Compile definitions
#

add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)


#
# We use this macro here to avoid creating extremely large C++ files with binary content
#

qt_add_big_resources(SOURCES
    ${CMAKE_CURRENT_BINARY_DIR}/flightMapFonts.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/ressources.qrc
    )

#
# Generate android executable
#
if( ANDROID )
    qtquick_compiler_add_resources(QTQUICKCOMPILEDRESOURCES ${CMAKE_CURRENT_BINARY_DIR}/qml.qrc)
    list(APPEND SOURCES
        ${QTQUICKCOMPILEDRESOURCES}
        android/src/de/akaflieg_freiburg/enroute/IntentLauncher.java
        android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java
        android/src/de/akaflieg_freiburg/enroute/ShareActivity.java
        android/src/de/akaflieg_freiburg/enroute/ShareUtils.java
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

    # Copy manual into Android assets directory
    file(COPY ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/docs/manual DESTINATION android/assets)

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
        COMMAND cp -prd ${CMAKE_CURRENT_SOURCE_DIR}/android .
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

    # Compile definitions
    add_compile_definitions(MANUAL_LOCATION="/android_asset/manual")

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

    # Add libraries and include paths
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::AndroidExtras Qt5::Core Qt5::Positioning Qt5::Quick Qt5::Sql Qt5::Svg Qt5::WebView qhttpengine sunset)
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/3rdParty/sunset/src ${CMAKE_SOURCE_DIR}/3rdParty/GSL/include)
endif()


#
# Generate unix executable
#

if (NOT ANDROID)
    list(APPEND SOURCES
        ${CMAKE_CURRENT_BINARY_DIR}/qml.qrc
        )

    # Compile definitions
    add_compile_definitions(MANUAL_LOCATION="${CMAKE_INSTALL_FULL_DOCDIR}/manual")

    # Add executable and libraries
    add_executable(${PROJECT_NAME} ${SOURCES})
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Positioning Qt5::Quick Qt5::Sql Qt5::Svg Qt5::WebView KF5::Notifications qhttpengine kdsingleapplication sunset)
    target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/3rdParty/sunset/src ${CMAKE_SOURCE_DIR}/3rdParty/GSL/include)
    target_precompile_headers(${PROJECT_NAME} PRIVATE
        <QObject>
        <QPointer>
        )

    # Install
    install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/enroute\ flight\ navigation.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/docs/manual DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()

# Enforce C++17 and no extensions
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)


#
# Generate documentation
#

configure_file(Doxyfile.in Doxyfile)
add_custom_target(enroute_doc COMMAND doxygen)
add_dependencies(fulldoc enroute_doc)
