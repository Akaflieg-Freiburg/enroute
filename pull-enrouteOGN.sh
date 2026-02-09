#!/bin/bash

cd 3rdParty/enrouteOGN
git fetch origin
git checkout main
git reset --hard origin/main
