#
# Prepare flatpack manifest
#

if (DEFINED TAG_COMMIT)
  configure_file(${APP_ID}.json ${APP_ID}.json)
else()
  message("Not building inside a GIT directory, or git command not available. Flatpak manifest will not be generated.")
endif()
