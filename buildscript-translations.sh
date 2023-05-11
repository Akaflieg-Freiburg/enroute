#!/bin/bash
set -e


cd 3rdParty/enrouteText
git pull origin master
#lupdate-qt5 ../../src -ts assets/enroute_es.ts

cd ../../build-linux-debug
ninja update_translations
cd ../3rdParty/enrouteText
git commit -a -m "Update translations"
git push origin HEAD:master
