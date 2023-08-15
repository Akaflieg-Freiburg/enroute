#
# Generate PNG versions of the main program icon
#

add_custom_target(mainIcons
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/icons
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/icons/${APP_ID}_angular.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/icons/${APP_ID}_angular.png -h 512 -w 512
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/icons/${APP_ID}.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/icons/${APP_ID}.png -h 512 -w 512
  COMMENT "Generated sources: main program icons"
  )


#
# Generate feature graphic for the Google Play Store
#

add_custom_target(featureGraphic
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/propaganda/GooglePlay
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/propaganda/SnapStore
  COMMAND convert -font Helvetica-bold -fill white -pointsize 60 -gravity center -draw "text 0,-100 'Enroute Flight Navigation'" ${CMAKE_SOURCE_DIR}/propaganda/GooglePlay/featureGraphic.png ${CMAKE_CURRENT_SOURCE_DIR}/propaganda/GooglePlay/featureGraphic.png
  COMMAND convert -font Helvetica-bold -fill white -pointsize 120 -gravity center -draw "text 0,-100 'Enroute Flight Navigation'" ${CMAKE_SOURCE_DIR}/propaganda/Snap/featureGraphic.png ${CMAKE_CURRENT_SOURCE_DIR}/propaganda/SnapStore/featureGraphic.jpg
  COMMENT "Generated sources: feature fraphic for Google Play and Snap Store"
  VERBATIM
  )


#
# Sprites for the flight map
#

add_custom_target(flightMapSprites
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/sprites
  COMMAND rm -f *.png
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD.svg -o AD.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-GLD.svg -o AD-GLD.png -h 20 -w 20
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-GRASS.svg -o AD-GRASS.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-INOP.svg -o AD-INOP.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL.svg -o AD-MIL.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL-GRASS.svg -o AD-MIL-GRASS.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL-PAVED.svg -o AD-MIL-PAVED.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-PAVED.svg -o AD-PAVED.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-UL.svg -o AD-UL.png -h 20 -w 20
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-WATER.svg -o AD-WATER.png -h 30 -w 30
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/MRP.svg -o MRP.png -h 25 -w 25
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/NDB.svg -o NDB.png -h 25 -w 25
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/RP.svg -o RP.png -h 25 -w 25
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VOR.svg -o VOR.png -w 22
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VORDME.svg -o VORDME.png -w 22
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VORTAC.svg -o VORTAC.png -w 26
  COMMAND spriteGenerator *.png
  COMMAND zopflipng -m -y sprite.png sprite.png
  COMMAND mv sprite.json ${CMAKE_SOURCE_DIR}/generatedSources/flightMap/sprite.json
  COMMAND mv sprite.png ${CMAKE_SOURCE_DIR}/generatedSources/flightMap/sprite.png
  COMMAND rm -f *.png
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD.svg -o AD.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-GLD.svg -o AD-GLD.png -h 40 -w 40
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-GRASS.svg -o AD-GRASS.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-INOP.svg -o AD-INOP.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL.svg -o AD-MIL.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL-GRASS.svg -o AD-MIL-GRASS.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-MIL-PAVED.svg -o AD-MIL-PAVED.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-PAVED.svg -o AD-PAVED.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-UL.svg -o AD-UL.png -h 40 -w 40
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/AD-WATER.svg -o AD-WATER.png -h 60 -w 60
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/MRP.svg -o MRP.png -h 50 -w 50
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/NDB.svg -o NDB.png -h 50 -w 50
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/RP.svg -o RP.png -h 50 -w 50
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VOR.svg -o VOR.png -w 44
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VORDME.svg -o VORDME.png -w 44
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/flightMap/sprites/VORTAC.svg -o VORTAC.png -w 52
  COMMAND spriteGenerator --pixelRatio 2 *.png
  COMMAND zopflipng -m -y sprite.png sprite.png
  COMMAND mv sprite.json ${CMAKE_SOURCE_DIR}/generatedSources/flightMap/sprite@2x.json
  COMMAND mv sprite.png ${CMAKE_SOURCE_DIR}/generatedSources/flightMap/sprite@2x.png
  COMMENT "Generated sources: flight map sprites"
  )


#
# Waypoint icons
#
# Some waypoint icons cannot be rendered well by th Qt SVG module. We produce
# PNG versions of those.
#

add_custom_target(waypointIcons
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/waypoints
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/icons/waypoints/AD-GLD.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/waypoints/AD-GLD.png -h 96 -w 96
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/icons/waypoints/AD-GRASS.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/waypoints/AD-GRASS.png -h 96 -w 96
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/icons/waypoints/AD-MIL-GRASS.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/waypoints/AD-MIL-GRASS.png -h 96 -w 96
  COMMAND rsvg-convert ${CMAKE_SOURCE_DIR}/src/icons/waypoints/AD-UL.svg -o ${CMAKE_CURRENT_SOURCE_DIR}/src/icons/waypoints/AD-UL.png -h 96 -w 96
  COMMENT "Generated sources: waypoint icons"
  )


#
# MapBox font ressource
#

add_custom_target(flightMapFonts
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/fonts/Roboto\ Condensed\ Italic
  COMMAND build-glyphs ${CMAKE_SOURCE_DIR}/3rdParty/Google-Fonts/RobotoCondensed-Italic.ttf ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/fonts/Roboto\ Condensed\ Italic  
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/fonts/Roboto\ Medium
  COMMAND build-glyphs ${CMAKE_SOURCE_DIR}/3rdParty/Google-Fonts/Roboto-Medium.ttf ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/fonts/Roboto\ Medium
  COMMAND mkdir -p ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/fonts/Roboto\ Regular
  COMMAND build-glyphs ${CMAKE_SOURCE_DIR}/3rdParty/Google-Fonts/Roboto-Regular.ttf ${CMAKE_CURRENT_SOURCE_DIR}/flightMap/fonts/Roboto\ Regular
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/generateFontQRC.py ${CMAKE_SOURCE_DIR}/generatedSources ${CMAKE_CURRENT_SOURCE_DIR}/flightMap
  COMMENT "Generated sources: mapbox font ressource"
  )
set(flightMapFontsQRC ${CMAKE_CURRENT_BINARY_DIR}/flightMap-fonts.qrc CACHE INTERNAL "" FORCE)


#
# Attributions
#

add_custom_target(attributions
  COMMAND pwd
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/generateAttributions.py
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  COMMENT "Generated sources: attributions"
  )


#
# Screenshots
#

add_custom_target(screenshots4GooglePlay
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=de-DE ${CMAKE_BINARY_DIR}/src/enroute --sg
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=en-US ${CMAKE_BINARY_DIR}/src/enroute --sg
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=fr-FR ${CMAKE_BINARY_DIR}/src/enroute --sg
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=it-IT ${CMAKE_BINARY_DIR}/src/enroute --sg
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=pl-PL ${CMAKE_BINARY_DIR}/src/enroute --sg
    COMMAND cp -prdv fastlane/metadata/* ../../generatedSources/fastlane/metadata/
    COMMENT "Generated sources: screenshots for google play"
    DEPENDS ${PROJECT_NAME}
  )
add_custom_target(screenshots4Manual
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=en_US ${CMAKE_BINARY_DIR}/src/enroute --sm
    COMMAND mv 01-*.png ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/01-intro/autogenerated/
    COMMAND mv 02-*.png ${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/02-steps/autogenerated/
    COMMENT "Generated sources: screenshots for manual"
    DEPENDS ${PROJECT_NAME}
  )


#
# Manual texts
#

add_custom_target(manualText
    COMMAND echo "Privacy Policies" >${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "================" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "English" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "-------" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=EN ${CMAKE_BINARY_DIR}/src/enroute --string :text/privacy.html | pandoc --from=HTML --write=rst >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    #
    COMMAND echo "Deutsche Übersetzung" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "--------------------" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=DE ${CMAKE_BINARY_DIR}/src/enroute --string :text/privacy.html | pandoc --from=HTML --write=rst >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    #
    COMMAND echo "Traduction en français" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "----------------------" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=FR ${CMAKE_BINARY_DIR}/src/enroute --string :text/privacy.html | pandoc --from=HTML --write=rst >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    #
    COMMAND echo "Traduzione italiana" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "-------------------" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=IT ${CMAKE_BINARY_DIR}/src/enroute --string :text/privacy.html | pandoc --from=HTML --write=rst >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    #
    COMMAND echo "Polskie tłumaczenie" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "-------------------" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=PL ${CMAKE_BINARY_DIR}/src/enroute --string :text/privacy.html | pandoc --from=HTML --write=rst >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    #
    COMMAND echo "Traducción al español" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "---------------------" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=ES ${CMAKE_BINARY_DIR}/src/enroute --string :text/privacy.html | pandoc --from=HTML --write=rst >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    COMMAND echo "" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/privacy.rst
    #
    COMMAND echo "Authors" >${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/authors.rst
    COMMAND echo "=======" >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/authors.rst
    COMMAND QT_QPA_PLATFORM=xcb LC_ALL=EN ${CMAKE_BINARY_DIR}/src/enroute --string :text/authors.html | pandoc --from=HTML --write=rst >>${CMAKE_SOURCE_DIR}/3rdParty/enrouteText/manual/04-appendix/authors.rst
    #
    COMMENT "Generate texts for manual"
    DEPENDS ${PROJECT_NAME}
  )


#
# Target 'generatedSources' that combines all of the above
#

add_custom_target(generatedSources
  DEPENDS attributions
  DEPENDS flightMapFonts
  DEPENDS flightMapSprites
  DEPENDS mainIcons
  DEPENDS manualText
  DEPENDS waypointIcons
  )
