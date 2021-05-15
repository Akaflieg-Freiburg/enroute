#
# Prepare snapcraft.yaml and binary snap
#


#
# Sample build
#

add_custom_target(snap-binary
  COMMAND snapcraft clean --enable-experimental-extensions
  COMMAND rm -r enroute-flight-navigation_${PROJECT_VERSION}_amd64.snap
  COMMAND snapcraft --enable-experimental-extensions
  COMMAND mv enroute-flight-navigation_${PROJECT_VERSION}_amd64.snap ${CMAKE_CURRENT_BINARY_DIR}
  COMMAND echo ""
  COMMAND echo ""
  COMMAND echo "Install your snap with the following command"
  COMMAND echo ""
  COMMAND echo " snap install ${CMAKE_CURRENT_BINARY_DIR}/enroute-flight-navigation_${PROJECT_VERSION}_amd64.snap --devmode"
  COMMAND echo ""
  COMMAND echo ""
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Generating snap binary"
  )


#
# Publication
#

add_custom_target(snap-publish
  COMMAND snapcraft login --with ~/.ubuntuLogin
  COMMAND snapcraft upload ${CMAKE_CURRENT_BINARY_DIR}/enroute-flight-navigation_${PROJECT_VERSION}_amd64.snap --release stable
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Publishing snap"
  )
