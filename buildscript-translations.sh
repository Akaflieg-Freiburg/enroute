#!/bin/bash
set -e

# Locate a Qt6 lupdate. Linux distros package it as "lupdate-qt6"; the Qt
# online-installer kits (Linux "gcc_64", macOS "macos") ship it as bin/lupdate.
if command -v lupdate-qt6 >/dev/null 2>&1; then
    LUPDATE=lupdate-qt6
else
    QTDIR=$(find ~/Software/buildsystems/Qt -maxdepth 2 -type d \( -name gcc_64 -o -name macos \) 2>/dev/null | sort -V | tail -1)
    if [ -n "$QTDIR" ] && [ -x "$QTDIR/bin/lupdate" ]; then
        LUPDATE="$QTDIR/bin/lupdate"
    elif command -v lupdate >/dev/null 2>&1; then
        LUPDATE=lupdate
    else
        echo "Error: cannot find lupdate (Qt 6)." >&2
        exit 1
    fi
fi


cd 3rdParty/enrouteText
git pull origin master
cd ../..

# We call the lupdate directly instead of using the cmake target because this
# way, source files that are not compiled (such as iOS files on a linux build
# host) will still be considered.

"$LUPDATE" src -ts 3rdParty/enrouteText/assets/enroute_cs.ts
"$LUPDATE" src -ts 3rdParty/enrouteText/assets/enroute_de.ts
"$LUPDATE" src -ts 3rdParty/enrouteText/assets/enroute_es.ts
"$LUPDATE" src -ts 3rdParty/enrouteText/assets/enroute_fr.ts
"$LUPDATE" src -ts 3rdParty/enrouteText/assets/enroute_it.ts
"$LUPDATE" src -ts 3rdParty/enrouteText/assets/enroute_pl.ts

cd 3rdParty/enrouteText
git commit -a -m "Update translations"
git push git@github.com:Akaflieg-Freiburg/enrouteText.git HEAD:master
cd ../..
