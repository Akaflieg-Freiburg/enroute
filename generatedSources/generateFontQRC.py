#!/bin/python3

import glob
import sys

file = open(sys.argv[2]+'/flightMap-fonts.qrc.in', 'w')
file.write('<!DOCTYPE RCC><RCC version="1.0">\n')
file.write('  <qresource>\n')
file.write('  <!-- MapBox font files -->\n')

for fileName in glob.glob(sys.argv[1]+'/**/*.pbf', recursive=True):
    fontName = fileName.split('/')[-2]+'/'+fileName.split('/')[-1]
    file.write('    <file alias="flightMap/fonts/{}">'.format(fontName)
               + "${CMAKE_SOURCE_DIR}/3rdParty/openseamap-gl/fonts/"
               + '{}</file>\n'.format(fontName))
    if fileName.split('/')[-2] == 'Roboto Regular':
        file.write('    <file alias="flightMap/fonts/Open Sans Regular,Arial Unicode MS Regular/{}">'.format(fileName.split('/')[-1])
                   + "${CMAKE_SOURCE_DIR}/3rdParty/openseamap-gl/fonts/"
                   + '{}</file>\n'.format(fontName))

file.write('  </qresource>\n')
file.write('</RCC>\n')
file.close()
