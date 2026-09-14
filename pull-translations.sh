#!/bin/bash
set -euo pipefail

# Update the bundled translation checkout. Resolve the directory relative to
# this script, so that the git commands can never run in the wrong repository.
cd "$(dirname "$0")/3rdParty/enrouteText"
git pull origin master
