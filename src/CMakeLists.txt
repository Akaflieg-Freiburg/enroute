
#
# Configure files
#

configure_file(config.h.in config.h)
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
    config.h.in
    flightMap.qrc.in
    icons.qrc.in
    README.md
    ressources.qrc.in

    # Header files
    ../3rdParty/KDSingleApplication/src/kdsingleapplication.cpp
    ../3rdParty/KDSingleApplication/src/kdsingleapplication_localsocket.cpp
    ../3rdParty/KDSingleApplication/src/kdsingleapplication.h
    ../3rdParty/KDSingleApplication/src/kdsingleapplication_lib.h
    ../3rdParty/KDSingleApplication/src/kdsingleapplication_localsocket_p.h
    ../3rdParty/sunset/src/sunset.h
    dataManagement/DataManager.h
    dataManagement/Downloadable_Abstract.h
    dataManagement/Downloadable_MultiFile.h
    dataManagement/Downloadable_SingleFile.h
    dataManagement/SSLErrorHandler.h
    fileFormats/CSV.h
    fileFormats/CUP.h
    fileFormats/DataFileAbstract.h
    fileFormats/FPL.h
    fileFormats/GeoTIFF.h
    fileFormats/MapURL.h
    fileFormats/MBTILES.h
    fileFormats/PLN.h
    fileFormats/TIFF.h
    fileFormats/TripKit.h
    fileFormats/ZipFile.h
    DemoRunner.h
    geomaps/Airspace.h
    geomaps/GeoJSON.h
    geomaps/GeoMapProvider.h
    geomaps/GPX.h
    geomaps/OpenAir.h
    geomaps/TileHandler.h
    geomaps/TileServer.h
    geomaps/Waypoint.h
    geomaps/WaypointLibrary.h
    geomaps/VAC.h
    geomaps/VACLibrary.h
    GlobalObject.h
    GlobalSettings.h
    Librarian.h
    Sensors.h
    navigation/Aircraft.h
    navigation/Atmosphere.h
    navigation/Clock.h
    navigation/FlightRoute.h
    navigation/Leg.h
    navigation/Navigator.h
    navigation/RemainingRouteInfo.h
    notam/NOTAM.h
    notam/NOTAMList.h
    notam/NOTAMProvider.h
    notification/Notification.h
    notification/Notification_DataUpdateAvailable.h
    notification/NotificationManager.h
    platform/FileExchange.h
    platform/FileExchange_Abstract.h
    platform/PlatformAdaptor.h
    platform/PlatformAdaptor_Abstract.h
    platform/SafeInsets_Abstract.h
    positioning/Geoid.h
    positioning/PositionInfo.h
    positioning/PositionInfoSource_Abstract.h
    positioning/PositionInfoSource_Satellite.h
    positioning/PositionProvider.h
    traffic/ConnectionInfo.h
    traffic/ConnectionScanner_Abstract.h
    traffic/ConnectionScanner_Bluetooth.h
    traffic/ConnectionScanner_SerialPort.h
    traffic/FlarmnetDB.h
    traffic/PasswordDB.h
    traffic/TrafficDataSource_Abstract.h
    traffic/TrafficDataSource_AbstractSocket.h
    traffic/TrafficDataSource_BluetoothClassic.h
    traffic/TrafficDataSource_BluetoothLowEnergy.h
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
    units/Density.h
    units/Distance.h
    units/Pressure.h
    units/Speed.h
    units/Temperature.h
    units/Timespan.h
    units/Units.h
    units/Volume.h
    units/VolumeFlow.h
    weather/Decoder.h
    weather/DensityAltitude.h
    weather/METAR.h
    weather/Station.h
    weather/TAF.h
    weather/WeatherDataProvider.h
    weather/Wind.h

    # C++ files
    ../3rdParty/sunset/src/sunset.cpp
    dataManagement/DataManager.cpp
    dataManagement/Downloadable_Abstract.cpp
    dataManagement/Downloadable_MultiFile.cpp
    dataManagement/Downloadable_SingleFile.cpp
    dataManagement/SSLErrorHandler.cpp
    DemoRunner.cpp
    fileFormats/CSV.cpp
    fileFormats/CUP.cpp
    fileFormats/DataFileAbstract.cpp
    fileFormats/FPL.cpp
    fileFormats/GeoTIFF.cpp
    fileFormats/MapURL.cpp
    fileFormats/MBTILES.cpp
    fileFormats/PLN.cpp
    fileFormats/TIFF.cpp
    fileFormats/TripKit.cpp
    fileFormats/ZipFile.cpp
    geomaps/Airspace.cpp
    geomaps/GeoJSON.cpp
    geomaps/GeoMapProvider.cpp
    geomaps/GPX.cpp
    geomaps/OpenAir.cpp
    geomaps/TileHandler.cpp
    geomaps/TileServer.cpp
    geomaps/Waypoint.cpp
    geomaps/WaypointLibrary.cpp
    geomaps/VAC.cpp
    geomaps/VACLibrary.cpp
    GlobalObject.cpp
    GlobalSettings.cpp
    Librarian.cpp
    Sensors.cpp
    main.cpp
    navigation/Aircraft.cpp
    navigation/Atmosphere.cpp
    navigation/Clock.cpp
    navigation/FlightRoute.cpp
    navigation/FlightRoute_GPX.cpp
    navigation/Leg.cpp
    navigation/Navigator.cpp
    navigation/RemainingRouteInfo.cpp
    notam/NOTAM.cpp
    notam/NOTAMList.cpp
    notam/NOTAMProvider.cpp
    notification/Notification.cpp
    notification/Notification_DataUpdateAvailable.cpp
    notification/NotificationManager.cpp
    platform/FileExchange_Abstract.cpp
    platform/PlatformAdaptor_Abstract.cpp
    platform/SafeInsets_Abstract.cpp
    positioning/Geoid.cpp
    positioning/PositionInfo.cpp
    positioning/PositionInfoSource_Abstract.cpp
    positioning/PositionInfoSource_Satellite.cpp
    positioning/PositionProvider.cpp
    traffic/ConnectionInfo.cpp
    traffic/ConnectionScanner_Abstract.cpp
    traffic/ConnectionScanner_Bluetooth.cpp
    traffic/ConnectionScanner_SerialPort.cpp
    traffic/FlarmnetDB.cpp
    traffic/PasswordDB.cpp
    traffic/TrafficDataSource_Abstract.cpp
    traffic/TrafficDataSource_Abstract_FLARM.cpp
    traffic/TrafficDataSource_Abstract_GDL90.cpp
    traffic/TrafficDataSource_Abstract_XGPS.cpp
    traffic/TrafficDataSource_AbstractSocket.cpp
    traffic/TrafficDataSource_BluetoothClassic.cpp
    traffic/TrafficDataSource_BluetoothLowEnergy.cpp
    traffic/TrafficDataSource_File.cpp
    traffic/TrafficDataSource_Simulate.cpp
    traffic/TrafficDataSource_Tcp.cpp
    traffic/TrafficDataSource_Udp.cpp
    traffic/TrafficDataProvider.cpp
    traffic/TrafficDataProvider_BluetoothClassic.cpp
    traffic/TrafficFactor_Abstract.cpp
    traffic/TrafficFactor_DistanceOnly.cpp
    traffic/TrafficFactor_WithPosition.cpp
    traffic/Warning.cpp
    units/Angle.cpp
    units/Density.cpp
    units/Distance.cpp
    units/Pressure.cpp
    units/Speed.cpp
    units/Temperature.cpp
    units/Timespan.cpp
    units/Volume.cpp
    units/VolumeFlow.cpp
    weather/Decoder.cpp
    weather/DensityAltitude.cpp
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
# Generate Android executable
#

if(ANDROID)
    list(APPEND SOURCES
        platform/FileExchange_Android.h
        platform/FileExchange_Android.cpp
        platform/PlatformAdaptor_Android.h
        platform/PlatformAdaptor_Android.cpp
        platform/SafeInsets_Android.h
        platform/SafeInsets_Android.cpp
        traffic/TrafficDataSource_SerialPort.h
        traffic/TrafficDataSource_SerialPort.cpp
        android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java
        android/src/de/akaflieg_freiburg/enroute/ShareActivity.java
        android/src/de/akaflieg_freiburg/enroute/ShareUtils.java
    )

# Copy manual and other files into the Android directories
file(COPY ${CMAKE_SOURCE_DIR}/generatedSources/manual DESTINATION android/assets)
file(COPY ${CMAKE_SOURCE_DIR}/generatedSources/src/android DESTINATION .)
file(COPY ${CMAKE_SOURCE_DIR}/src/android DESTINATION .)

# Configure files
configure_file(AndroidManifest.xml.in android/AndroidManifest.xml)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/action/drawable-hdpi/ic_info_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_info.png COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/file/drawable-hdpi/ic_file_download_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_file_download.png COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/alert/drawable-hdpi/ic_error_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_error.png COPYONLY)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/material-design-icons/alert/drawable-hdpi/ic_warning_black_24dp.png ${CMAKE_CURRENT_BINARY_DIR}/android/res/drawable/ic_warning.png COPYONLY)
configure_file(android/src/de/akaflieg_freiburg/enroute/ShareActivity.java android/src/de/akaflieg_freiburg/enroute COPYONLY)
configure_file(android/src/de/akaflieg_freiburg/enroute/ShareUtils.java android/src/de/akaflieg_freiburg/enroute COPYONLY)
configure_file(android/src/de/akaflieg_freiburg/enroute/MobileAdaptor.java android/src/de/akaflieg_freiburg/enroute COPYONLY)


# The android target require a few additional variables
set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/android" CACHE INTERNAL "")

# Compile definitions
add_compile_definitions(MANUAL_LOCATION="/android_asset/manual")

qt_add_executable(${PROJECT_NAME} ${SOURCES} ${ANDROID_EXTRA_SOURCES}
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::Bluetooth
    Qt6::Concurrent
    Qt6::Core
    Qt6::Core5Compat
    Qt6::HttpServer
    Qt6::Positioning
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sensors
    Qt6::SerialPort
    Qt6::Sql
    Qt6::Svg
    Qt6::TextToSpeech
    Qt6::WebView
    QMapLibre::Location
    libzip::zip
)
qmaplibre_location_setup_plugins(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES
    QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_BINARY_DIR}/android"
    QT_ANDROID_TARGET_SDK_VERSION 34
    QT_ANDROID_MIN_SDK_VERSION 28
)
if(NOT ANDROID_PLATFORM STREQUAL "android-28")
    message(FATAL_ERROR "ANDROID_PLATFORM must be set to android-28, is currently set to " ${ANDROID_PLATFORM})
endif()


# Include SSL Libraries
set(ANDROID_TARGET_ARCH ${CMAKE_ANDROID_ARCH_ABI})
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../3rdParty/android_openssl dummyBinaryDir)
endif()


#
# Generate Linux executable
#

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    list(APPEND SOURCES
        platform/FileExchange_Linux.h
        platform/FileExchange_Linux.cpp
        platform/PlatformAdaptor_Linux.h
        platform/PlatformAdaptor_Linux.cpp
        platform/SafeInsets_Desktop.h
        platform/SafeInsets_Desktop.cpp
        traffic/TrafficDataSource_SerialPort.h
        traffic/TrafficDataSource_SerialPort.cpp
    )

# Compile definitions
add_compile_definitions(MANUAL_LOCATION="${CMAKE_INSTALL_FULL_DOCDIR}/manual")

# Add executable and libraries
qt_add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::Bluetooth
    Qt6::Concurrent
    Qt6::Core
    Qt6::Core5Compat
    Qt6::DBus
    Qt6::HttpServer
    Qt6::Positioning
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::SerialPort
    Qt6::Sql
    Qt6::Svg
    Qt6::TextToSpeech
    Qt6::Widgets
    QMapLibre::Location
    libzip::zip
)
if (TARGET Qt6::WebView)
    message(STATUS "QtWebView found.")
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt6::WebView
    )
endif()
target_precompile_headers(${PROJECT_NAME}
    PUBLIC
    <QQmlEngine>
)

# Install
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/generatedSources/manual DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()


#
# Generate macOS executable
#

if (CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
    message("CMAKE_OSX_DEPLOYMENT_TARGET: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    set(app_icon_macos "${CMAKE_CURRENT_SOURCE_DIR}/macos/enroute.icns")
    set_source_files_properties(${app_icon_macos} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")

    # qt_add_executable(photosurfaceexample MACOSX_BUNDLE main.cpp ${app_icon_macos})

    list(APPEND SOURCES
        platform/FileExchange_MacOS.h
        platform/FileExchange_MacOS.cpp
        platform/PlatformAdaptor_MacOS.h
        platform/PlatformAdaptor_MacOS.cpp
        platform/SafeInsets_Desktop.h
        platform/SafeInsets_Desktop.cpp
        traffic/TrafficDataSource_SerialPort.h
        traffic/TrafficDataSource_SerialPort.cpp
    )

# Compile definitions
add_compile_definitions(MANUAL_LOCATION="${CMAKE_INSTALL_FULL_DOCDIR}/manual")

# Add executable and libraries
qt_add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SOURCES} ${app_icon_macos})
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE true
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/macos/Info.plist"
    MACOSX_BUNDLE_ICON_FILE enroute.icns
    MACOSX_BUNDLE_BUNDLE_NAME "Enroute Flight Navigation"
)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::Bluetooth
    Qt6::Concurrent
    Qt6::Core
    Qt6::Core5Compat
    Qt6::HttpServer
    Qt6::Positioning
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::SerialPort
    Qt6::Sql
    Qt6::Svg
    Qt6::TextToSpeech
    Qt6::WebView
    Qt6::Widgets
    QMapLibre::Location
    libzip::zip
)
endif()


#
# Generate iOS executable
#

if (IOS)
    list(APPEND SOURCES
        platform/FileExchange_iOS.h
        platform/FileExchange_iOS.cpp
        platform/PlatformAdaptor_iOS.h
        platform/PlatformAdaptor_iOS.cpp
        platform/SafeInsets_iOS.h
        platform/SafeInsets_iOS.cpp
        ios/ObjCAdapter.mm
        ios/ObjCAdapter.h
        ios/ObjectiveC.h
        ios/ObjectiveC.m
        ios/SafeAreaService.h
        ios/SafeAreaService.m
    )

# Compile definitions
add_compile_definitions(MANUAL_LOCATION="${CMAKE_INSTALL_FULL_DOCDIR}/manual")

# Add executable and libraries
qt_add_executable(${PROJECT_NAME} ${SOURCES})
set(CMAKE_MACOSX_RPATH OFF)
qt_add_ios_ffmpeg_libraries(${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt6::Bluetooth
    Qt6::Concurrent
    Qt6::Core
    Qt6::Core5Compat
    Qt6::HttpServer
    Qt6::Location
    Qt6::Positioning
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Sensors
    Qt6::Sql
    Qt6::Svg
    Qt6::TextToSpeech
    Qt6::WebView
    QMapLibre::Location
    libzip::zip
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE true
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist"
)
set_source_files_properties(
    PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
qmaplibre_location_setup_plugins(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME}
    PUBLIC
    ios/mapLibreDependencies
    ios/mapLibreDependencies/optional
    ios/mapLibreDependencies/csscolorparser
    ios/mapLibreDependencies/parsedate
    ios/mapLibreDependencies/nunicode/include
)

#Resources
set(bundle_resources
    "ios/Assets.xcassets"
    "ios/de.lproj"
    "ios/en.lproj"
    "ios/es.lproj"
    "ios/fr.lproj"
    "ios/it.lproj"
    "ios/pl.lproj"
    ${CMAKE_SOURCE_DIR}/generatedSources/manual
)
target_sources(${PROJECT_NAME} PRIVATE ${bundle_resources})
set_source_files_properties(${bundle_resources} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
)
source_group("Resources" FILES ${bundle_resources})

#Launchscreen
set_target_properties(${PROJECT_NAME} PROPERTIES QT_IOS_LAUNCH_SCREEN "${CMAKE_CURRENT_SOURCE_DIR}/ios/LaunchScreen.storyboard")

#Privacy Manifest
set(privacy_manifest "${CMAKE_CURRENT_SOURCE_DIR}/ios/PrivacyInfo.xcprivacy")
target_sources("${PROJECT_NAME}" PRIVATE "${privacy_manifest}")
set_property(TARGET "${PROJECT_NAME}" APPEND PROPERTY RESOURCE "${privacy_manifest}")
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_SOURCE_DIR}/3rdParty/KDSingleApplication/src
    ${CMAKE_SOURCE_DIR}/3rdParty/sunset/src
    ${CMAKE_SOURCE_DIR}/3rdParty/GSL/include
    dataManagement
    fileFormats
    geomaps
    navigation
    notam
    notification
    platform
    positioning
    traffic
    ui
    units
    weather
    nunicode/include
)

set_source_files_properties(qml/items/Global.qml PROPERTIES QT_QML_SINGLETON_TYPE TRUE)
qt_add_qml_module(${PROJECT_NAME}
    URI akaflieg_freiburg.enroute
    VERSION 1.0
    NO_RESOURCE_TARGET_PATH
    DEPENDENCIES
    QtPositioning
    QtQuick
    QML_FILES
    qml/main.qml
    qml/dialogs/AddBTDeviceDialog.qml
    qml/dialogs/CenteringDialog.qml
    qml/dialogs/LongTextDialog.qml
    qml/dialogs/AircraftSaveDialog.qml
    qml/dialogs/ErrorDialog.qml
    qml/dialogs/FirstRunDialog.qml
    qml/dialogs/FlightRouteSaveDialog.qml
    qml/dialogs/LongTextDialogMD.qml
    qml/dialogs/MetarTafDialog.qml
    qml/dialogs/NotamListDialog.qml
    qml/dialogs/PasswordDialog.qml
    qml/dialogs/PasswordStorageDialog.qml
    qml/dialogs/PrivacyWarning.qml
    qml/dialogs/RenameVACDialog.qml
    qml/dialogs/WaypointDescription.qml
    qml/dialogs/WaypointEditor.qml
#    qml/items/AppWindow.qml
    qml/items/+Material/AppWindow.qml
    qml/items/AutoSizingMenu.qml
    qml/items/DecoratedListView.qml
    qml/items/DecoratedScrollView.qml
    qml/items/DegreeInput.qml
    qml/items/ElevationInput.qml
    qml/items/FlightMap.qml
    qml/items/FlightVector.qml
#    qml/items/Footer.qml
    qml/items/+Material/Footer.qml
    qml/items/Global.qml
#    qml/items/Icon.qml
    qml/items/+Material/Icon.qml
    qml/items/ImportManager.qml
#    qml/items/MapButton.qml
    qml/items/+Material/MapButton.qml
    qml/items/MapSet.qml
    qml/items/MFM.qml
    qml/items/MyTextField.qml
    qml/items/NavBar.qml
    qml/items/NotificationArea.qml
    qml/items/NotificationButton.qml
#    qml/items/PageHeader.qml
    qml/items/+Material/PageHeader.qml
    qml/items/RemainingRouteBar.qml
    qml/items/StandardHeader.qml
    qml/items/SwipeToDeleteDelegate.qml
    qml/items/Traffic.qml
    qml/items/TrafficLabel.qml
    qml/items/WaypointDelegate.qml
    qml/items/WordWrappingCheckDelegate.qml
    qml/items/WordWrappingRadioDelegate.qml
    qml/items/WordWrappingItemDelegate.qml
    qml/items/WordWrappingSwitchDelegate.qml
    qml/pages/VAC.qml
    qml/pages/AircraftPage.qml
    qml/pages/AircraftLibrary.qml
    qml/pages/BugReportPage.qml
    qml/pages/ConnectionManager.qml
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
    qml/pages/PressureAltitude.qml
    qml/pages/PrivacyPage.qml
    qml/pages/SettingsPage.qml
    qml/pages/TrafficReceiver.qml
    qml/pages/URLResolver.qml
    qml/pages/WaypointLibraryPage.qml
    qml/pages/Weather.qml
    SOURCES
    ui/ScaleQuickItem.h
    ui/ScaleQuickItem.cpp
    ui/SideViewQuickItem.h
    ui/SideViewQuickItem.cpp
    traffic/TrafficDataProvider_BluetoothLowEnergy.cpp
)

#
# Translations
#

qt_add_lrelease(
    TS_FILES
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_de.ts
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_es.ts
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_fr.ts
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_it.ts
    ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/assets/enroute_pl.ts
    QM_FILES_OUTPUT_VARIABLE qmFiles
)
qt_add_resources(
    ${PROJECT_NAME}
    translations_enroute
    BASE
    ${CMAKE_CURRENT_BINARY_DIR}
    PREFIX
    i18n
    FILES
    ${qmFiles}
)

find_path(Qt6_TRANSLATION_DIR qtbase_de.qm
    HINTS
    ${Qt6Core_DIR}/../../../translations
    /usr/translations
    REQUIRED
)
qt_add_resources(${PROJECT_NAME} translations_qt
    BASE
    ${Qt6_TRANSLATION_DIR}
    PREFIX
    i18n
    FILES
    ${Qt6_TRANSLATION_DIR}/qtbase_de.qm
    ${Qt6_TRANSLATION_DIR}/qtbase_es.qm
    ${Qt6_TRANSLATION_DIR}/qtbase_fr.qm
    ${Qt6_TRANSLATION_DIR}/qtbase_it.qm
    ${Qt6_TRANSLATION_DIR}/qtbase_pl.qm
    ${Qt6_TRANSLATION_DIR}/qtdeclarative_de.qm
    ${Qt6_TRANSLATION_DIR}/qtdeclarative_es.qm
    ${Qt6_TRANSLATION_DIR}/qtdeclarative_fr.qm
    #${Qt6_TRANSLATION_DIR}/qtdeclarative_it.qm
    ${Qt6_TRANSLATION_DIR}/qtdeclarative_pl.qm
)


#
# Install
#

if ((CMAKE_SYSTEM_NAME STREQUAL "Darwin") OR ((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME STREQUAL "Windows")))

    # Install the executable into "${CMAKE_INSTALL_PREFIX}/bin".
    install(TARGETS ${PROJECT_NAME}
        BUNDLE  DESTINATION .
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

endif()

if ( QTDEPLOY )

    # Generate the deployment script for the target MyApp.
    qt_generate_deploy_qml_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )

    # Call the deployment script on "cmake --install".
    install(SCRIPT ${deploy_script})

endif()
