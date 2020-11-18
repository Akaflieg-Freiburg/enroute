#
# Prepare snapcraft.yaml and binary snap
#


#
# Configure
#

configure_file(snap/snapcraft.yaml.in snap/snapcraft.yaml @ONLY)


#
# Sample build
#

add_custom_target(snap-binary
    COMMAND snapcraft
    COMMAND echo ""
    COMMAND echo ""
    COMMAND echo "Install your snap with the following command"
    COMMAND echo ""
    COMMAND echo " snap install ${CMAKE_CURRENT_BINARY_DIR}/enroute-flight-navigation_1.8.2_amd64.snap --devmode"
    COMMAND echo ""
    COMMAND echo ""
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating snap binary"
    )


#
# Publication
#

add_custom_target(snap-publish
    COMMAND rm -rf enrouteSnap
    COMMAND git clone --depth 1 git@github.com:Akaflieg-Freiburg/enrouteSnap.git
    COMMAND cp snap/snapcraft.yaml enrouteSnap/snap
    COMMAND cd enrouteSnap
    COMMAND git diff-index --quiet HEAD || git commit -am "New upstream release"
    COMMAND git push
    COMMAND cd ..
    COMMAND rm -rf enrouteSnap
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Publishing snap"
    )
