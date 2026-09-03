#!/bin/bash
set -euo pipefail

# Update the bundled enrouteOGN checkout. Resolve the directory relative to
# this script, so that a failed cd can never leave the git commands running
# in the enroute repository itself.
cd "$(dirname "$0")/3rdParty/enrouteOGN"
git fetch origin
git checkout main
git reset --hard origin/main
