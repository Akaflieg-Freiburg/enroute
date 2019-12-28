#!/bin/python3

import glob

file = open('flightMap-fonts.qrc', 'w')
file.write('<!DOCTYPE RCC><RCC version="1.0">\n')
file.write('  <qresource>\n')
file.write('  <!-- MapBox font files -->\n')

for fileName in glob.glob('**/*.pbf', recursive=True):
    fontName = fileName.split('/')[-2]+'/'+fileName.split('/')[-1]
    file.write('    <file alias="flightMap/fonts/{}">{}</file>\n'.format(fontName, fileName))
    if fileName.split('/')[-2] == 'Roboto Regular':
        file.write('    <file alias="flightMap/fonts/Open Sans Regular,Arial Unicode MS Regular/{}">{}</file>\n'.format(fileName.split('/')[-1], fileName))
    

file.write('  </qresource>\n')
file.write('</RCC>\n')
file.close()
