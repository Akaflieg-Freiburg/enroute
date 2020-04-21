#!/bin/bash

cd 3rdParty/enrouteText

git pull

lupdate-qt5 ../../src -ts \
	    assets/enroute_cz.ts \
	    assets/enroute_de.ts

git commit -a -m "Update translations"
git push

