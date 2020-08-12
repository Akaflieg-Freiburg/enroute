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
  COMMAND echo " snap install"
  COMMAND echo ""
  COMMAND echo ""
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Generating snap binary"
  )
