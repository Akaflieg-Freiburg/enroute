#!/bin/bash

cd 3rdParty/enrouteText
git pull origin master
cd ../../build-linux-qt6-debug
ninja update_translations
cd ../3rdParty/enrouteText
git commit -a -m "Update translations"
git push origin HEAD:master
