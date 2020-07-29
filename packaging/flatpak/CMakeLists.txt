#
# Prepare flatpack manifest
#


#
# Configure
#

configure_file(${APP_ID}.json.in ${APP_ID}.json)
configure_file(${CMAKE_SOURCE_DIR}/3rdParty/flatpak-kde-runtime/patch/qtdeclarative-use-python3.patch patch/qtdeclarative-use-python3.patch COPYONLY)


#
# Sample build
#

add_custom_target(flatpak-binary
  COMMAND rm -rf flatpakBuildDirectory .flatpak-builder
  COMMAND flatpak remote-add --user --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
  COMMAND flatpak install --user --assumeyes org.freedesktop.Sdk/x86_64/19.08 org.freedesktop.Platform/x86_64/19.08
  COMMAND flatpak-builder flatpakBuildDirectory ${APP_ID}.json
  COMMAND echo ""
  COMMAND echo ""
  COMMAND echo "Run your flatpak with the following command"
  COMMAND echo ""
  COMMAND echo " flatpak-builder --run ${CMAKE_CURRENT_BINARY_DIR}/flatpakBuildDirectory ${CMAKE_CURRENT_BINARY_DIR}/${APP_ID}.json ${PROJECT_NAME}"
  COMMAND echo ""
  COMMAND echo ""
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating flatpak binary"
  )
