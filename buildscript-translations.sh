#!/bin/bash
set -e


cd 3rdParty/enrouteText
git pull origin master
cd ../..

# We call the lupdate directly instead of using the cmake target because this
# way, source files that are not compiled (such as iOS files on a linux build
# host) will still be considered.

lupdate-qt6 src -ts 3rdParty/enrouteText/assets/enroute_cs.ts
lupdate-qt6 src -ts 3rdParty/enrouteText/assets/enroute_de.ts
lupdate-qt6 src -ts 3rdParty/enrouteText/assets/enroute_es.ts
lupdate-qt6 src -ts 3rdParty/enrouteText/assets/enroute_fr.ts
lupdate-qt6 src -ts 3rdParty/enrouteText/assets/enroute_it.ts
lupdate-qt6 src -ts 3rdParty/enrouteText/assets/enroute_pl.ts

cd 3rdParty/enrouteText
git commit -a -m "Update translations"
git push origin HEAD:master
cd ../..
