#!/bin/python3

import numpy
import math
import os
import skimage
from PIL import Image, ImageDraw

path = '/home/kebekus/Software/projects/enroute/generatedSources/flightMap/sprites'
imageFileNames = os.listdir(path)

numImages = len(imageFileNames)
numCols = math.floor(math.sqrt(numImages))
numRows = math.ceil(numImages/numCols)
print(str(numImages)+' Images, '+str(numCols)+' Colums, '+str(numRows)+' Rows')

# Read images
images = []
for imageFileName in imageFileNames:
    images.append( Image.open(path+'/'+imageFileName))

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
spritePane = Image.new('RGB', (spritePaneWidth,spritePaneHeight), color = 'red')

# Copy images into spritePane
x = 0
y = 0
for i in range(0, numImages):
    rowIndex = math.floor(i/numCols)
    spritePane.paste(images[i], (x,0))
    x = x+images[i].width

spritePane.save('spritePane.png')
