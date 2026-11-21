#!/bin/sh
#
# Start the QML language server from the newest Qt installation under
# ~/Software/buildsystems/Qt (Linux kit "gcc_64" or macOS kit "macos"), so that
# it matches the Qt the project is built with. Referenced from
# qt-qml.qmlls.customExePath in .vscode/settings.json.
#

QTDIR=$(find "$HOME/Software/buildsystems/Qt" -maxdepth 2 -type d \( -name gcc_64 -o -name macos \) 2>/dev/null | sort -V | tail -1)
if [ -z "$QTDIR" ] || [ ! -x "$QTDIR/bin/qmlls" ]; then
    echo "qmlls.sh: no Qt installation with qmlls found under ~/Software/buildsystems/Qt" >&2
    exit 1
fi
exec "$QTDIR/bin/qmlls" "$@"
