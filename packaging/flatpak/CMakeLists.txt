#
# Prepare flatpack manifest
#

if (DEFINED GIT_COMMIT)
  configure_file(${APP_ID}.json.in ${APP_ID}.json)
else()
  message("Not building inside a GIT directory, or git command not available. Flatpak manifest will not be generated.")
endif()
