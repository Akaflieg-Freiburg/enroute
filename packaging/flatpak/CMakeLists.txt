#
# Prepare flatpack manifest
#


#
# Configure
#

configure_file(${APP_ID}.json.in ${APP_ID}.json)


#
# Sample build
#

add_custom_target(flatpak-binary
    COMMAND rm -rf flatpakBuildDirectory .flatpak-builder
    COMMAND flatpak remote-add --user --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
    COMMAND flatpak install --user --assumeyes org.kde.Sdk/x86_64/5.15 org.kde.Platform/x86_64/5.15 io.qt.qtwebengine.BaseApp/x86_64/5.15
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


#
# Publication
#

add_custom_target(flatpak-publish
    COMMAND rm -rf de.akaflieg_freiburg.enroute
    COMMAND git clone --depth 1 git@github.com:flathub/de.akaflieg_freiburg.enroute.git
    COMMAND cp de.akaflieg_freiburg.enroute.json de.akaflieg_freiburg.enroute
    COMMAND cd de.akaflieg_freiburg.enroute
    COMMAND git diff-index --quiet HEAD || git commit -am "New upstream release"
    COMMAND git push
    COMMAND cd ..
    COMMAND rm -rf de.akaflieg_freiburg.enroute
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Publishing flatpak"
    )
