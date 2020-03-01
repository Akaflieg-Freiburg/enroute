#
# Generate PNG versions of the main program icon
#

add_custom_target(mainIcons
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/metadata
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}_angular.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/metadata/${APP_ID}_angular.png -h 512 -w 512
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/metadata/${APP_ID}.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/metadata/${APP_ID}.png -h 512 -w 512
  COMMENT "Generated sources: main program icons"
  )


#
# Sprites for the flight map
#

add_custom_target(flightMapSprites
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD@2x.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-GLD.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-GLD.png -h 20 -w 20
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-GLD.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-GLD@2x.png -h 40 -w 40
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-GRASS.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-GRASS.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-GRASS.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-GRASS@2x.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-INOP.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-INOP.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-INOP.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-INOP@2x.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL@2x.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL-GRASS.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL-GRASS.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL-GRASS.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL-GRASS@2x.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL-PAVED.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL-PAVED.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL-PAVED.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-MIL-PAVED@2x.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-PAVED.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-PAVED.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-PAVED.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-PAVED@2x.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-UL.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-UL.png -h 20 -w 20
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-UL.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-UL@2x.png -h 40 -w 40
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-WATER.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-WATER.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-WATER.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/AD-WATER@2x.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/MRP.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/MRP.png -h 25 -w 25
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/MRP.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/MRP@2x.png -h 50 -w 50
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/NDB.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/NDB.png -h 25 -w 25
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/NDB.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/NDB@2x.png -h 50 -w 50
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/RP.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/RP.png -h 25 -w 25
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/RP.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/RP@2x.png -h 50 -w 50
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VOR.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VOR.png -w 22
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VOR.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VOR@2x.png -w 44
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VORDME.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VORDME.png -w 22
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VORDME.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VORDME@2x.png -w 44
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VORTAC.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VORTAC.png -w 26
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VORTAC.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites/VORTAC@2x.png -w 52
  COMMENT "Generated sources: flight map sprites"
  )


#
# MapBox font ressource
#

add_custom_target(mapBoxFontRessource
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/flightMap
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/generateFontQRC.py ${CMAKE_SOURCE_DIR}/3rdParty ${CMAKE_CURRENT_SOURCE_DIR}/flightMap
  COMMENT "Generated sources: mapbox font ressource"
  )
set(flightMapFontsQRC ${CMAKE_CURRENT_BINARY_DIR}/flightMap-fonts.qrc CACHE INTERNAL "" FORCE)


#
# Target 'generatedSources' that combines all of the above
#

add_custom_target(generatedSources
  DEPENDS flightMapSprites
  DEPENDS mainIcons
  DEPENDS mapBoxFontRessource
  )
