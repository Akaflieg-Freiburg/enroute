#
# Configure files
#

configure_file(${APP_ID}.json.in ${APP_ID}.json)

if (NOT ANDROID)
  configure_file(${APP_ID}.appdata.xml.in ${APP_ID}.appdata.xml)
  configure_file(${APP_ID}.desktop.in ${APP_ID}.desktop)
  configure_file(${APP_ID}.svg ${APP_ID}.svg)
  
  install(FILES ${PROJECT_BINARY_DIR}/metadata/${APP_ID}.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
  install(FILES ${PROJECT_BINARY_DIR}/metadata/${APP_ID}.appdata.xml DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo)
  install(FILES ${PROJECT_SOURCE_DIR}/metadata/${APP_ID}.svg DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps)
endif()
