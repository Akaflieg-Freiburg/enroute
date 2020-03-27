# Generated Sources

The subdirectories of this directory contains source files that have been
generated from other sources, for instance icons in PNG format that have been
generated from scalable vector graphics. We include these generated sources here
because the tools required to build them are not universally available on all
platforms.

* The script **generateSprite.py** takes a number of PNG images. It sorts them into two groups (file names containing @2x and others) and then generates two sprite panes and two JSON files suitable for inclusion into MapBox style sheets. The output files are called 'spritePane.json', 'spritePane@2x.json', 'spritePane.png' and 'spritePane@2x.png'.

There exists a special CMake target, **generatedSources** that re-builds the
source files in this directory.
