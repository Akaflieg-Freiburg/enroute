#
# Prepare flatpack manifest
#

set(FLATPAK_RUNTIME_VERSION 5.14)

#
# Configure
#

configure_file(${APP_ID}.json.in ${APP_ID}.json)


#
# Prepare TAR file
#

add_custom_target(flatpak
  COMMAND git-archive-all ${PROJECT_NAME}-${PROJECT_VERSION}.tar.xz
  COMMENT "Generating tarball for flatpak"
  )


#
# Sample build
#

add_custom_target(flatpak-binary
  COMMAND rm -rf flatpakBuildDirectory .flatpak-builder
  COMMAND flatpak remote-add --user --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
  COMMAND flatpak install --user --assumeyes org.kde.Sdk/x86_64/${FLATPAK_RUNTIME_VERSION} org.kde.Platform/x86_64/${FLATPAK_RUNTIME_VERSION}
  COMMAND flatpak-builder flatpakBuildDirectory ${APP_ID}.json
  COMMAND echo ""
  COMMAND echo ""
  COMMAND echo "Run your flatpak with the following command"
  COMMAND echo ""
  COMMAND echo " flatpak-builder --run ${CMAKE_CURRENT_BINARY_DIR}/flatpakBuildDirectory ${CMAKE_CURRENT_BINARY_DIR}/${APP_ID}.json ${PROJECT_NAME}"
  COMMAND echo ""
  COMMAND echo ""
  DEPENDS flatpak
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating flatpak binary"
  )
