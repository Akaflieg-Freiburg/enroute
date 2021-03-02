#!/bin/bash

cd 3rdParty/enrouteText
git pull origin master
lupdate-qt5 ../../src -ts \
	    assets/enroute_de.ts \
	    assets/enroute_fr.ts \
	    assets/enroute_it.ts \
	    assets/enroute_pl.ts
git commit -a -m "Update translations"
git push origin HEAD:master
