#!/bin/bash

cd ../enrouteText
git pull
lupdate-qt5 ../enroute/src -ts \
	    assets/enroute_cz.ts \
	    assets/enroute_de.ts \
	    assets/enroute_fr.ts
git commit -a -m "Update translations"
git push


cd ../enroute/3rdParty/enrouteText
git pull
