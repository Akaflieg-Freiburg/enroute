#
# Prepare flatpack manifest
#


#
# Configure
#

configure_file(${APP_ID}.json.in ${APP_ID}.json @ONLY)


#
# Sample build
#

add_custom_target(flatpak-binary
    COMMAND flatpak remote-add --user --if-not-exists flathub https://flathub.org/repo/flathub.flatpakrepo
    COMMAND flatpak install --system --assumeyes runtime/org.kde.Sdk/x86_64/6.8
    COMMAND flatpak install --system --assumeyes app/io.qt.qtwebengine.BaseApp/x86_64/6.8
    COMMAND flatpak-builder --force-clean flatpakBuildDirectory ${APP_ID}.json
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
# Run linter on sample build
#

add_custom_target(flatpak-lint
    COMMAND flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest de.akaflieg_freiburg.enroute.json
    COMMAND flatpak run --command=flatpak-builder-lint org.flatpak.Builder appstream ../../metadata/de.akaflieg_freiburg.enroute.appdata.xml
    COMMAND flatpak run --command=flatpak-builder-lint org.flatpak.Builder builddir flatpakBuildDirectory
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Linting flatpak build"
    )

#
# Publication
#

add_custom_target(flatpak-publish
    COMMAND rm -rf de.akaflieg_freiburg.enroute
    COMMAND git clone --depth 1 --branch upstream git@github.com:flathub/de.akaflieg_freiburg.enroute.git
    COMMAND cp de.akaflieg_freiburg.enroute.json de.akaflieg_freiburg.enroute
    COMMAND cp ${CMAKE_SOURCE_DIR}/CHANGELOG.md de.akaflieg_freiburg.enroute
    COMMAND cd de.akaflieg_freiburg.enroute
    COMMAND git diff-index --quiet HEAD || git commit -am "New upstream release"
    COMMAND git push
    COMMAND gh pr create --title "New upstream release" --body "Please merge the following new upstream release"
    COMMAND cd ..
    COMMAND rm -rf de.akaflieg_freiburg.enroute
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Publishing flatpak"
    )
