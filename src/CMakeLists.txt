
#
# Configure files
#

configure_file(flightMap.qrc.in flightMap.qrc)
configure_file(${CMAKE_SOURCE_DIR}/generatedSources/flightMap/flightMap-fonts.qrc.in flightMapFonts.qrc)
configure_file(icons.qrc.in icons.qrc)
configure_file(ressources.qrc.in ressources.qrc)


#
# Set up sources
#

set(SOURCES
    # Ressources
    ${CMAKE_CURRENT_BINARY_DIR}/flightMap.qrc
    ${CMAKE_CURRENT_BINARY_DIR}/icons.qrc

    # Other files
    AndroidManifest.xml.in
    flightMap.qrc.in
    icons.qrc.in
    README.md
    ressources.qrc.in

    # Header files
    dataManagement/DataManager.h
    dataManagement/Downloadable_Abstract.h
    dataManagement/Downloadable_MultiFile.h
    dataManagement/Downloadable_SingleFile.h
    dataManagement/SSLErrorHandler.h
    dataManagement/UpdateNotifier.h
    DemoRunner.h
    geomaps/Airspace.h
    geomaps/CUP.h
    geomaps/GeoJSON.h
    geomaps/GeoMapProvider.h
    geomaps/GPX.h
    geomaps/MBTILES.h
    geomaps/TileHandler.h
    geomaps/TileServer.h
    geomaps/Waypoint.h
    geomaps/WaypointLibrary.h
    GlobalObject.h
    GlobalSettings.h
    Librarian.h
    navigation/Aircraft.h
    navigation/Clock.h
    navigation/FlightRoute.h
    navigation/Leg.h
    navigation/Navigator.h
    navigation/RemainingRouteInfo.h
    platform/FileExchange.h
    platform/FileExchange_Abstract.h
    platform/Notifier.h
    platform/Notifier_Abstract.h
    platform/PlatformAdaptor.h
    platform/PlatformAdaptor_Abstract.h
    platform/SafeInsets_Abstract.h
    positioning/Geoid.h
    positioning/PositionInfo.h
    positioning/PositionInfoSource_Abstract.h
    positioning/PositionInfoSource_Satellite.h
    positioning/PositionProvider.h
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
    units/Angle.h
    units/ByteSize.h
    units/Distance.h
    units/Speed.h
    units/Time.h
    units/Units.h
    units/Volume.h
    units/VolumeFlow.h
    weather/Decoder.h
    weather/METAR.h
    weather/Station.h
    weather/TAF.h
    weather/WeatherDataProvider.h
    weather/Wind.h

    # C++ files
    dataManagement/DataManager.cpp
    dataManagement/Downloadable_Abstract.cpp
    dataManagement/Downloadable_MultiFile.cpp
    dataManagement/Downloadable_SingleFile.cpp
    dataManagement/SSLErrorHandler.cpp
    dataManagement/UpdateNotifier.cpp
    DemoRunner.cpp
    geomaps/Airspace.cpp
    geomaps/CUP.cpp
    geomaps/GeoJSON.cpp
    geomaps/GeoMapProvider.cpp
    geomaps/GPX.cpp
    geomaps/MBTILES.cpp
    geomaps/TileHandler.cpp
    geomaps/TileServer.cpp
    geomaps/Waypoint.cpp
    geomaps/WaypointLibrary.cpp
    GlobalObject.cpp
    GlobalSettings.cpp
    Librarian.cpp
    main.cpp
    navigation/Aircraft.cpp
    navigation/Clock.cpp
    navigation/FlightRoute.cpp
    navigation/FlightRoute_GPX.cpp
    navigation/Leg.cpp
    navigation/Navigator.cpp
    navigation/RemainingRouteInfo.cpp
    platform/FileExchange_Abstract.cpp
    platform/Notifier_Abstract.cpp
    platform/PlatformAdaptor_Abstract.cpp
    platform/SafeInsets_Abstract.cpp
    positioning/Geoid.cpp
    positioning/PositionInfo.cpp
    positioning/PositionInfoSource_Abstract.cpp
    positioning/PositionInfoSource_Satellite.cpp
    positioning/PositionProvider.cpp
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
    units/Angle.cpp
    units/Distance.cpp
    units/Speed.cpp
    units/Time.cpp
    units/Volume.cpp
    units/VolumeFlow.cpp
    weather/Decoder.cpp
    weather/METAR.cpp
    weather/Station.cpp
    weather/TAF.cpp
    weather/WeatherDataProvider.cpp
    weather/Wind.cpp

    ${HEADERS}
    )

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
    list(APPEND SOURCES
        platform/FileExchange_Android.h
        platform/FileExchange_Android.cpp
        platform/Notifier_Android.h
        platform/Notifier_Android.cpp
        platform/PlatformAdaptor_Android.h
        platform/PlatformAdaptor_Android.cpp
        platform/SafeInsets_Android.h
        platform/SafeInsets_Android.cpp
        android/src/de/akaflieg_freiburg/enroute/IntentLauncher.java
        android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java
        android/src/de/akaflieg_freiburg/enroute/Notifier.java
        android/src/de/akaflieg_freiburg/enroute/ShareActivity.java
        android/src/de/akaflieg_freiburg/enroute/ShareUtils.java
        )

    # Configure files
    configure_file(AndroidManifest.xml.in android/AndroidManifest.xml)
    configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/action/drawable-hdpi/ic_info_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_info.png COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/file/drawable-hdpi/ic_file_download_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_file_download.png COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/alert/drawable-hdpi/ic_error_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_error.png COPYONLY)
    configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/alert/drawable-hdpi/ic_warning_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_warning.png COPYONLY)

    # Copy manual into Android assets directory
    file(COPY ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/docs/manual DESTINATION android/assets)
    file(COPY ${CMAKE_SOURCE_DIR}/generatedSources/src/android DESTINATION .)
    file(COPY ${CMAKE_SOURCE_DIR}/src/android DESTINATION .)

    # The android target require a few additional variables
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/android" CACHE INTERNAL "")

    # Compile definitions
    add_compile_definitions(MANUAL_LOCATION="/android_asset/manual")

    qt_add_executable(${PROJECT_NAME} ${SOURCES} ${ANDROID_EXTRA_SOURCES})
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt6::Concurrent
        Qt6::Core
        Qt6::Core5Compat
        Qt6::HttpServer
        Qt6::Positioning
        Qt6::Quick
        Qt6::Sql
        Qt6::Svg
        Qt6::WebView
        sunset)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        QT_ANDROID_EXTRA_LIBS "${ANDROID_EXTRA_LIBS}"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/android"
        QT_ANDROID_TARGET_SDK_VERSION 33
    )
endif()


#
# Generate unix executable
#

if (NOT ANDROID)
    list(APPEND SOURCES
        platform/FileExchange_Linux.h
        platform/FileExchange_Linux.cpp
        platform/Notifier_Linux.h
        platform/Notifier_Linux.cpp
        platform/PlatformAdaptor_Linux.h
        platform/PlatformAdaptor_Linux.cpp
        platform/SafeInsets_Desktop.h
        platform/SafeInsets_Desktop.cpp
    )

    # Compile definitions
    add_compile_definitions(MANUAL_LOCATION="${CMAKE_INSTALL_FULL_DOCDIR}/manual")

    # Add executable and libraries
    qt_add_executable(${PROJECT_NAME} ${SOURCES})
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt6::Concurrent
        Qt6::Core
        Qt6::Core5Compat
        Qt6::DBus
        Qt6::HttpServer
        Qt6::Positioning
        Qt6::Quick
        Qt6::Sql
        Qt6::Svg
        Qt6::Widgets
        kdsingleapplication
        sunset)
    target_precompile_headers(${PROJECT_NAME}
        PRIVATE
        <QObject>
        <QPointer>
    )

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/docs/manual DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/3rdParty/sunset/src
    ${CMAKE_SOURCE_DIR}/3rdParty/GSL/include
    dataManagement
    geomaps
    navigation
    platform
    positioning
    traffic
    ui
    units
    weather
)

qt_add_qml_module(${PROJECT_NAME}
    URI akaflieg_freiburg.enroute
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
    QML_FILES
    qml/main.qml
    qml/dialogs/CenteringDialog.qml
    qml/dialogs/LongTextDialog.qml
    qml/items/AutoSizingMenu.qml
    qml/items/DegreeInput.qml
    qml/items/ElevationInput.qml
    qml/items/FlightVector.qml
    qml/items/NavBar.qml
    qml/items/RemainingRouteBar.qml
    qml/items/WordWrappingCheckDelegate.qml
    qml/items/WordWrappingItemDelegate.qml
    qml/items/WordWrappingSwitchDelegate.qml
    qml/pages/SettingsPage.qml
    qml/dialogs/AircraftSaveDialog.qml
    qml/dialogs/ErrorDialog.qml
    qml/dialogs/FirstRunDialog.qml
    qml/dialogs/FlightRouteAddWPDialog.qml
    qml/dialogs/FlightRouteSaveDialog.qml
    qml/dialogs/LongTextDialogMD.qml
    qml/dialogs/MissingPermissionsDialog.qml
    qml/dialogs/PasswordDialog.qml
    qml/dialogs/PasswordStorageDialog.qml
    qml/dialogs/TooManyDownloadsDialog.qml
    qml/dialogs/WaypointDescription.qml
    qml/dialogs/WaypointEditor.qml
    qml/dialogs/WeatherReport.qml
    qml/items/FlightMap.qml
    qml/items/Icon.qml
    qml/items/ImportManager.qml
    qml/items/MFM.qml
    qml/items/StandardHeader.qml
    qml/items/SwipeToDeleteDelegate.qml
    qml/items/Traffic.qml
    qml/items/TrafficLabel.qml
    qml/pages/AircraftPage.qml
    qml/pages/AircraftLibrary.qml
    qml/pages/BugReportPage.qml
    qml/pages/DonatePage.qml
    qml/pages/FlightRouteEditor.qml
    qml/pages/FlightRouteLibrary.qml
    qml/pages/InfoPage.qml
    qml/pages/Manual.qml
    qml/pages/DataManagerPage.qml
    qml/pages/MapPage.qml
    qml/pages/Nearby.qml
    qml/pages/ParticipatePage.qml
    qml/pages/Positioning.qml
    qml/pages/PrivacyPage.qml
    qml/pages/TrafficReceiver.qml
    qml/pages/WaypointLibraryPage.qml
    qml/pages/WeatherPage.qml
    SOURCES
    ui/ScaleQuickItem.h
    ui/ScaleQuickItem.cpp
)

qt_add_translations(${PROJECT_NAME}
    TS_FILES
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_de.ts
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_fr.ts
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_it.ts
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_pl.ts
)

find_path(Qt6_TRANSLATION_DIR qtbase_de.qm
    HINTS ${Qt6Core_DIR}/../../../translations
    REQUIRED
)
qt_add_resources(${PROJECT_NAME} translations
    BASE
    ${Qt6_TRANSLATION_DIR}
    PREFIX
    i18n
    FILES
    ${Qt6_TRANSLATION_DIR}/qtbase_de.qm
    ${Qt6_TRANSLATION_DIR}/qtbase_fr.qm
    ${Qt6_TRANSLATION_DIR}/qtbase_it.qm
    ${Qt6_TRANSLATION_DIR}/qtbase_pl.qm
    ${Qt6_TRANSLATION_DIR}/qtdeclarative_de.qm
    ${Qt6_TRANSLATION_DIR}/qtdeclarative_fr.qm
    #${Qt6_TRANSLATION_DIR}/qtdeclarative_it.qm
    ${Qt6_TRANSLATION_DIR}/qtdeclarative_pl.qm
)

# Enforce C++17 and no extensions
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_EXTENSIONS OFF)


#
# Generate documentation
#

configure_file(Doxyfile.in Doxyfile)
add_custom_target(enroute_doc COMMAND doxygen)
add_dependencies(fulldoc enroute_doc)
