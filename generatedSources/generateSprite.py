#!/bin/python3

import json
import math
import sys
from PIL import Image, ImageDraw


def generateSpritePane(imageFileNames, factor):
    numImages = len(imageFileNames)
    numCols = math.floor(math.sqrt(numImages))
    numRows = math.ceil(numImages/numCols)
    print(str(numImages)+' Images, '+str(numCols)+' Colums, '+str(numRows)+' Rows')
    
    # Read images
    images = []
    for imageFileName in imageFileNames:
        images.append( Image.open(imageFileName))
        
    # Estimate size for spritePane
    rowHeight = [0]*numRows
    rowWidth  = [0]*numRows
    for i in range(0, numImages):
        rowIndex = math.floor(i/numCols)
        rowHeight[rowIndex] = max(rowHeight[rowIndex], images[i].height)
        rowWidth[rowIndex] = rowWidth[rowIndex]+images[i].width
        spritePaneWidth  = 0
        spritePaneHeight = 0
    for i in range(0, numRows):
        spritePaneWidth  = max(spritePaneWidth, rowWidth[i])
        spritePaneHeight = spritePaneHeight+rowHeight[i]
    print('spritePaneWidth  '+str(spritePaneWidth))
    print('spritePaneHeight '+str(spritePaneHeight))
    spritePane = Image.new('RGBA', (spritePaneWidth,spritePaneHeight), color = 'red')

    # Copy images into spritePane
    x = 0
    y = 0
    imageDescriptions = {}
    for i in range(0, numImages):
        imageDescription = {}
        imageDescription['x'] = x
        imageDescription['y'] = y
        imageDescription['height'] = images[i].height
        imageDescription['width']  = images[i].width
        imageDescription['pixelRatio']  = factor
    
        rowIndex = math.floor(i/numCols)
        spritePane.paste(images[i], (x,y))
        x = x+images[i].width
    
        imageDescriptions[imageFileNames[i].split('/')[-1].split('.')[0]] = imageDescription
    
        if i%numCols == (numCols-1):
            y = y+ rowHeight[rowIndex]
            x = 0
    if factor == 1:
        spritePanePNGName = 'spritePane.png'
        spritePaneJSONName = 'spritePane.json'
    else:
        spritePanePNGName = 'spritePane@'+str(factor)+'x.png'
        spritePaneJSONName = 'spritePane@'+str(factor)+'x.json'

    spritePane.save(spritePanePNGName, optimize=True)
    spriteJSON = json.dumps(imageDescriptions, sort_keys=True, indent=4)
    file = open(spritePaneJSONName, 'w')
    file.write(spriteJSON)


# Main Program starts here

imageFileNames_1x = [fileName for fileName in sys.argv[1:] if '@' not in fileName]
imageFileNames_2x = [fileName for fileName in sys.argv[1:] if '@2x' in fileName]

generateSpritePane(imageFileNames_1x, 1)
generateSpritePane(imageFileNames_2x, 2)
